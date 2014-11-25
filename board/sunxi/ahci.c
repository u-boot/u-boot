#include <common.h>
#include <ahci.h>
#include <scsi.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>

#define AHCI_PHYCS0R 0x00c0
#define AHCI_PHYCS1R 0x00c4
#define AHCI_PHYCS2R 0x00c8
#define AHCI_RWCR    0x00fc

/* This magic PHY initialisation was taken from the Allwinner releases
 * and Linux driver, but is completely undocumented.
 */
static int sunxi_ahci_phy_init(u32 base)
{
	u8 *reg_base = (u8 *)base;
	u32 reg_val;
	int timeout;

	writel(0, reg_base + AHCI_RWCR);
	mdelay(5);

	setbits_le32(reg_base + AHCI_PHYCS1R, 0x1 << 19);
	clrsetbits_le32(reg_base + AHCI_PHYCS0R,
			(0x7 << 24),
			(0x5 << 24) | (0x1 << 23) | (0x1 << 18));
	clrsetbits_le32(reg_base + AHCI_PHYCS1R,
			(0x3 << 16) | (0x1f << 8) | (0x3 << 6),
			(0x2 << 16) | (0x6 << 8) | (0x2 << 6));
	setbits_le32(reg_base + AHCI_PHYCS1R, (0x1 << 28) | (0x1 << 15));
	clrbits_le32(reg_base + AHCI_PHYCS1R, (0x1 << 19));
	clrsetbits_le32(reg_base + AHCI_PHYCS0R, (0x7 << 20), (0x3 << 20));
	clrsetbits_le32(reg_base + AHCI_PHYCS2R, (0x1f << 5), (0x19 << 5));
	mdelay(5);

	setbits_le32(reg_base + AHCI_PHYCS0R, (0x1 << 19));

	timeout = 250; /* Power up takes approx 50 us */
	for (;;) {
		reg_val = readl(reg_base + AHCI_PHYCS0R) & (0x7 << 28);
		if (reg_val == (0x2 << 28))
			break;
		if (--timeout == 0) {
			printf("AHCI PHY power up failed.\n");
			return -EIO;
		}
		udelay(1);
	};

	setbits_le32(reg_base + AHCI_PHYCS2R, (0x1 << 24));

	timeout = 100; /* Calibration takes approx 10 us */
	for (;;) {
		reg_val = readl(reg_base + AHCI_PHYCS2R) & (0x1 << 24);
		if (reg_val == 0x0)
			break;
		if (--timeout == 0) {
			printf("AHCI PHY calibration failed.\n");
			return -EIO;
		}
		udelay(1);
	}

	mdelay(15);

	writel(0x7, reg_base + AHCI_RWCR);

	return 0;
}

void scsi_init(void)
{
	printf("SUNXI SCSI INIT\n");
#ifdef CONFIG_SATAPWR
	gpio_request(CONFIG_SATAPWR, "satapwr");
	gpio_direction_output(CONFIG_SATAPWR, 1);
	/* Give attached sata device time to power-up to avoid link timeouts */
	mdelay(500);
#endif

	if (sunxi_ahci_phy_init(SUNXI_SATA_BASE) < 0)
		return;

	ahci_init(SUNXI_SATA_BASE);
}
