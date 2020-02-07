
#include "cpu.h"
#include "device-ata.h"

const phy_addr_t ata_mmio_size = 0x1000;
const char *firmware_revision = ""; /* 8 ASCII chars */
const char *model_number = "ncpu32k-ATA"; /* 40 ASCII chars */

/*
 * Definitions of ATA Commands
 */
#define CMD_IdentifyDrive 0xec
#define CMD_InitializeDriveParameters 0x91
#define CMD_ReadSector_WRetry  0x20
#define CMD_WriteSector_WRetry  0x30
#define CMD_ReadMultiple 0xc4
#define CMD_WriteMultiple 0xc5

#define ATA_REG_SHIFT 2

/*
 * Definitions of Command Block Regfile
 * There are two different addressing bank:
 *  1 READ(DIOR-)     ReadReg_*
 *  2 WRITE(DIOW-)    WriteReg_*
 */
#define ReadReg_Data     (0x00 << ATA_REG_SHIFT)
#define WriteReg_Data    ReadReg_Data

#define ReadReg_Error     (0x01 << ATA_REG_SHIFT)
#define WriteReg_Feature ReadReg_Error

#define ReadReg_SectorCount   (0x02 << ATA_REG_SHIFT)
#define WriteReg_SectorCount ReadReg_SectorCount

#define ReadReg_SectorNumber  (0x03 << ATA_REG_SHIFT)
#define WriteReg_SectorNumber ReadReg_SectorNumber

#define ReadReg_CylinderLow    (0x04 << ATA_REG_SHIFT)
#define WriteReg_CylinderLow  ReadReg_CylinderLow

#define ReadReg_CylinderHigh    (0x05 << ATA_REG_SHIFT)
#define WriteReg_CylinderHigh  ReadReg_CylinderHigh

#define ReadReg_DriverHead  (0x06 << ATA_REG_SHIFT)
#define WriteReg_DriverHead ReadReg_DriverHead

#define ReadReg_Status  (0x07 << ATA_REG_SHIFT)
#define WriteReg_Command   ReadReg_Status

/* Bitmap of Device Control register */
#define DeviceControl_RST 0x04
#define DeviceControl_IEN 0x02

/* Bitmap of Alternate Status register */
#define Status_IEN  0x80
#define Status_DRDY 0x40
#define Status_DF   0x20
#define Status_DSC  0x10
#define Status_DRQ  0x08
#define Status_COR  0x04
#define Status_IDX  0x02
#define Status_ERR  0x01

/* Bitmap of Driver/Head register */
#define DriverHead_DRV 0x10
#define DriverHead_L 0x40

/* Intrnal functions */
static void dev_ata_writereg8(phy_addr_t addr, uint8_t val, void *opaque);
static void dev_ata_writereg32(phy_addr_t addr, uint32_t val, void *opaque);
static uint8_t dev_ata_readreg8(phy_addr_t addr, void *opaque);
static uint16_t dev_ata_readreg16(phy_addr_t addr, void *opaque);
static uint32_t dev_ata_readreg32(phy_addr_t addr, void *opaque);

typedef uint32_t ata_lba_t;

#define ATA_CHUNK_MAXSIZE (64*1024) /* 64KiB */

struct device_ata {
  /* ATA control block regfile */
  uint16_t RegDeviceControl;
  
  /* ATA command block regfile */
  uint16_t RegError, RegFeature;
  uint16_t RegSectorCount;
  uint16_t RegSectorNumber;
  uint16_t RegCylinderLow;
  uint16_t RegCylinderHigh;
  uint16_t RegDriverHead;
  uint16_t RegStatus, RegCommand;
  
  uint16_t id16[256];
  
  char DRV; /* driver select: 0 = driver 0, 1 = driver 1 */
  uint16_t *buf_base;
  int buf_pos;
  int buf_len;
  int irq;
  
  uint16_t disk_chunk[ATA_CHUNK_MAXSIZE/2];
};


int dev_ata_init(struct device_ata **dev_out, phy_addr_t mmio_base, int irq)
{
  const int heads = 16;
  const int sectors = 64;
  const int cylinders = ATA_CHUNK_MAXSIZE/(heads*sectors*512);
  const int sects_total = heads*sectors*cylinders;
  
  struct device_ata *dev = (struct device_ata *)malloc(sizeof(struct device_ata));
  dev->irq = irq;
  
  /*
   * Setup "Identify Drive Information"
   */
  memset(dev->id16, 0, sizeof dev->id16);
  /* General configuration */
  dev->id16[0] = 0x0040; /* fixed drive */
  /* Number of cylinders */
  dev->id16[1] = cylinders;
  /* # of heads */
  dev->id16[3] = heads;
  /* # of unformatted bytes per track */
  dev->id16[4] = 512*sectors;
  /* # of unformatted bytes per sector */
  dev->id16[5] = 512;
  /* # of sectors per track */
  dev->id16[6] = sectors;
  /* buffer type */
  dev->id16[20] = 0x0003;
  /* buffer size in 512 bytes increment */
  dev->id16[21] = 512;
  /* # of ECC bytes avail on Read/Write Long cmds */
  dev->id16[22] = 4;
  /* Firmware revision (8 ASCII chars) */
  memset(&dev->id16[23], ' ', 4 * sizeof(uint16_t));
  memcpy(&dev->id16[27], firmware_revision, strlen(firmware_revision));
  /* Model number (40 ASCII chars) */
  memset(&dev->id16[27], ' ', 20 * sizeof(uint16_t));
  memcpy(&dev->id16[27], model_number, strlen(model_number));
  
  /* bit[15..8] Vendor Unique, bit[7..0] Read/Write Multiple: max num of sects */
  dev->id16[47] = 0x8000 | 128;
  /* 0000h = cannot perform doubleword I/O */
  dev->id16[48] = 0x0000;
  /* Capabilities */
  dev->id16[49] = (1<<9); /* LBA supported */
  /* bit[15..8]: PIO data transfer cycle timing mode */
  dev->id16[51] = 0x200;
  /* bit[15..8]: DMA data transfer cycle timing mode */
  dev->id16[52] = 0x200;
  /* # of current cylinders */
  dev->id16[54] = cylinders;
  /* # of current heads */
  dev->id16[55] = heads;
  /* # of current sectors */
  dev->id16[56] = sectors;
  /* Current capacity in sectors */
  dev->id16[57] = (sects_total >> 0)&0xFFFF;
  dev->id16[58] = (sects_total >>16)&0xFFFF;
  /* bit[8] multiple sector settings is valid
   * bit[7..0] current setting for number of sects per R/W Multiple Interrupt */
  dev->id16[59] = 0x0000; /* not valid */
  /* Total number of User Addressable Sectors (LBA mode Only) */
  dev->id16[60] = (sects_total >> 0)&0xFFFF;
  dev->id16[61] = (sects_total >>16)&0xFFFF;
  
  /* version, support ATA-1 and ATA-2 */
  dev->id16[80] = (1<<1)|(1<<2);
  /* Command sets supported. (NOP supported) */
  dev->id16[82] = (1<<14);
  dev->id16[83] = (1<<14); /* hardcoded to 1 */
  dev->id16[84] = (1<<14); /* hardcoded to 1 */
  /* Command set/feature enabled (NOP) */
  dev->id16[85] = (1<<14);
  /* Command set/feature enabled */
  dev->id16[86] = 0;
  dev->id16[87] = (1<<14); /* hardcoded to 1 */
  
  /*
   * Register it on MMIO
   */
  mmio_register_writem8(mmio_base, mmio_base + ata_mmio_size-1, dev_ata_writereg8, dev);
  mmio_register_writem32(mmio_base, mmio_base + ata_mmio_size-1, dev_ata_writereg32, dev);

  mmio_register_readm8(mmio_base, mmio_base + ata_mmio_size-1, dev_ata_readreg8, dev);
  mmio_register_readm16(mmio_base, mmio_base + ata_mmio_size-1, dev_ata_readreg16, dev);
  mmio_register_readm32(mmio_base, mmio_base + ata_mmio_size-1, dev_ata_readreg32, dev);
  
  dev->irq = irq;
  
  dev_ata_reset(dev);
  
  *dev_out = dev;
  return 0;
}

void dev_ata_reset(struct device_ata *dev)
{
  /* Reset ATA regfile... */
  dev->RegDeviceControl = (1<<3); /* bit3: hardcoded to 1 */
  dev->RegError = 0x1;
  dev->RegFeature = 0x0;
  dev->RegSectorCount = 0x1;
  dev->RegSectorNumber = 0x1;
  dev->RegCylinderLow = 0x0;
  dev->RegCylinderHigh = 0x0;
  dev->RegDriverHead = (1<<7)|(1<<5); /* bit7,5: hardcoded to 1 */
  dev->RegStatus = Status_DRDY;
  dev->RegCommand = 0x0;
  
  /* Reset internal status... */
  dev->DRV = 1; /* select Driver 1 */
  dev->buf_base = dev->id16;
  dev->buf_pos = 0;
  dev->buf_len = 256;
}

static inline ata_lba_t get_sector(struct device_ata *dev)
{
  if (!(dev->RegDriverHead & DriverHead_L))
    {
      fprintf(stderr, "%s() CHS mode unimplemented\n", __func__);
      panic(1);
    }
  return ((dev->RegDriverHead&0x0F) << 24) | (dev->RegCylinderHigh << 16) | (dev->RegCylinderLow << 8) | dev->RegSectorNumber;
}
static inline void set_sector(struct device_ata *dev, ata_lba_t sector)
{
  if (!(dev->RegDriverHead & DriverHead_L))
    {
      fprintf(stderr, "%s() CHS mode unimplemented\n", __func__);
      panic(1);
    }
  dev->RegSectorNumber = sector & 0xFF;
  dev->RegCylinderLow = (sector >> 8) & 0xFF;
  dev->RegCylinderHigh = (sector >> 16) & 0xFF;
  dev->RegDriverHead = (dev->RegDriverHead & 0xF0) | ((sector >> 24) & 0x0F);
}

static void dev_ata_writereg8(phy_addr_t addr, uint8_t val, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  addr &= (ata_mmio_size-1);

  if (addr == WriteReg_DriverHead)
    {
      dev->RegDriverHead = val;
      dev->DRV = (val & DriverHead_DRV) ? 0 : 1;
      return;
    }

  if (addr == 0x100) //device control register
    {
      if (!(val&DeviceControl_RST) && (dev->RegDeviceControl&DeviceControl_RST)) /* reset done */
        {
          dev->RegDriverHead &= 0xF0; /* reset head */
          dev->RegStatus = Status_DRDY | Status_DSC;
          dev->RegSectorCount = 0x1;
          dev->RegSectorNumber = 0x1;
          dev->RegCylinderLow = 0x0;
          dev->RegCylinderHigh = 0x0;
          dev->RegError = 0x1;
          dev->RegCommand = 0x0;
        }
      else if ((val&DeviceControl_RST) && !(dev->RegDeviceControl&DeviceControl_RST)) /* reset */
        {
          dev->RegError = 0x1; /* set diagnostics message */
          dev->RegStatus = Status_IEN | Status_DSC;
        }

      dev->RegDeviceControl = val;
      return;
    }

  if (!dev->DRV)
    {
      /* DRIVER 0 is not presented. */
      return;
    }
    
  switch(addr)
    {
      case WriteReg_Feature:
        dev->RegFeature = val;
        break;

      case WriteReg_SectorCount:
        dev->RegSectorCount = val;
        break;

      case WriteReg_SectorNumber:
        dev->RegSectorNumber = val;
        break;

      case WriteReg_CylinderLow:
        dev->RegCylinderLow = val;
        break;

      case WriteReg_CylinderHigh:
        dev->RegCylinderHigh = val;
        break;

      case WriteReg_Command:
        dev->RegCommand = val;
        switch(dev->RegCommand)
          {
            case CMD_IdentifyDrive:
              dev->buf_base = dev->id16;
              dev->buf_pos = 0;
              dev->buf_len = 256;
              dev->RegStatus = Status_DRDY | Status_DSC | Status_DRQ;
              if (!(dev->RegDeviceControl & DeviceControl_IEN))
                {
                  irqc_set_interrupt(dev->irq, 1);
                }
              break;

            case CMD_InitializeDriveParameters:
              dev->RegStatus = Status_DRDY | Status_DSC;
              dev->RegError = 0x0;
              if (!(dev->RegDeviceControl & DeviceControl_IEN))
                {
                  irqc_set_interrupt(dev->irq, 1);
                }
              break;

            default:
              fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
              panic(1);
              break;
          } /* switch(RegCommand) */
        break;

      default:  
        fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
        panic(1);
        break;
    }
}

static void dev_ata_writereg32(phy_addr_t addr, uint32_t val, void *opaque)
{
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
}

static uint8_t dev_ata_readreg8(phy_addr_t addr, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  if (!dev->DRV)
    {
      /* DRIVER 0 is not presented. */
      return 0xff;
    }
  addr &= (ata_mmio_size-1);
  switch(addr)
    {
      case ReadReg_Error:
        return dev->RegError;

      case ReadReg_SectorCount:
        return dev->RegSectorCount;

      case ReadReg_SectorNumber:
        return dev->RegSectorNumber;

      case ReadReg_CylinderLow:
        return dev->RegCylinderLow;

      case ReadReg_CylinderHigh:
        return dev->RegCylinderHigh;

      case ReadReg_DriverHead:
        return dev->RegDriverHead;

      case ReadReg_Status:
        irqc_set_interrupt(dev->irq, 0);
        return dev->RegStatus;

      case 0x100: // device control register, but read as status register
        return dev->RegStatus;

      default:
        fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
        panic(1);
        break;
    }
  return 0;
}

static uint16_t dev_ata_readreg16(phy_addr_t addr, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  addr &= (ata_mmio_size-1);
  
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
  return 0;
}

static uint32_t dev_ata_readreg32(phy_addr_t addr, void *opaque)
{
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
  return 0;
}
