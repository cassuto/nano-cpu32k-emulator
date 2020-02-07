#ifndef DEVICE_ATA_H_
#define DEVICE_ATA_H_

struct device_ata;

extern int dev_ata_init(struct device_ata **dev, phy_addr_t mmio_base, int irq);
extern void dev_ata_reset(struct device_ata *dev);

#endif /* DEVICE_ATA_H_ */
