
#include "cpu.h"
#include "device-ata.h"
#include "device-tree.h"

const phy_addr_t mmio_base_ata = MMIO_PHY_BASE + 0x01000000;

const int irq_ata = 1;

static struct device_ata *dev_ata;

int devicetree_init(void)
{
  int rc;
  
  if((rc = dev_ata_init(&dev_ata, mmio_base_ata, irq_ata)))
    {
      return rc;
    }
  return 0;
}
