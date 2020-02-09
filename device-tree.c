
#include "cpu.h"
#include "device-ata.h"
#include "device-tree.h"

const phy_addr_t mmio_base_ata_command = MMIO_PHY_BASE + 0x01000000;
const phy_addr_t mmio_base_ata_control = MMIO_PHY_BASE + 0x01000100;
const int irq_ata = 3;

static struct device_ata *dev_ata;

/**
 * @brief Init Device Tree.
 * @return status code
 */
int devicetree_init(void)
{
  int rc;
  
  if((rc = dev_ata_init(&dev_ata, mmio_base_ata_command, mmio_base_ata_control, irq_ata)))
    {
      return rc;
    }
  return 0;
}
