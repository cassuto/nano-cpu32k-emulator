
#include "cpu.h"
#include "device-ata.h"

const phy_addr_t ata_mmio_size = 0x1000;

/* Intrnal functions */
static void dev_ata_writereg8(phy_addr_t addr, uint8_t val, void *opaque);
static void dev_ata_writereg32(phy_addr_t addr, uint32_t val, void *opaque);
static uint8_t dev_ata_readreg8(phy_addr_t addr, void *opaque);
static uint16_t dev_ata_readreg16(phy_addr_t addr, void *opaque);
static uint32_t dev_ata_readreg32(phy_addr_t addr, void *opaque);

struct device_ata {
  int irq;
};

int dev_ata_init(struct device_ata **dev_out, phy_addr_t mmio_base, int irq)
{
  struct device_ata *dev = (struct device_ata *)malloc(sizeof(struct device_ata));
  dev->irq = irq;
  
  dev_ata_reset(dev);
  
  *dev_out = dev;
  return 0;
}

void dev_ata_reset(struct device_ata *dev)
{
  
}

static void dev_ata_writereg8(phy_addr_t addr, uint8_t val, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  addr &= ata_mmio_size;
  
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
}

static void dev_ata_writereg32(phy_addr_t addr, uint32_t val, void *opaque)
{
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
}

static uint8_t dev_ata_readreg8(phy_addr_t addr, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  addr &= ata_mmio_size;
  
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
  return 0;
}

static uint16_t dev_ata_readreg16(phy_addr_t addr, void *opaque)
{
  struct device_ata *dev = (struct device_ata *)opaque;
  addr &= ata_mmio_size;
  
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
}

static uint32_t dev_ata_readreg32(phy_addr_t addr, void *opaque)
{
  fprintf(stderr, "%s() invalid reg address %#x\n", __func__, addr);
  panic(1);
  return 0;
}
