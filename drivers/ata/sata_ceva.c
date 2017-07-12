/*
 * (C) Copyright 2015 - 2016 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <asm/arch/hardware.h>

#include <asm/io.h>

/* Vendor Specific Register Offsets */
#define AHCI_VEND_PCFG  0xA4
#define AHCI_VEND_PPCFG 0xA8
#define AHCI_VEND_PP2C  0xAC
#define AHCI_VEND_PP3C  0xB0
#define AHCI_VEND_PP4C  0xB4
#define AHCI_VEND_PP5C  0xB8
#define AHCI_VEND_PAXIC 0xC0
#define AHCI_VEND_PTC   0xC8

/* Vendor Specific Register bit definitions */
#define PAXIC_ADBW_BW64 0x1
#define PAXIC_MAWIDD	(1 << 8)
#define PAXIC_MARIDD	(1 << 16)
#define PAXIC_OTL	(0x4 << 20)

#define PCFG_TPSS_VAL	(0x32 << 16)
#define PCFG_TPRS_VAL	(0x2 << 12)
#define PCFG_PAD_VAL	0x2

#define PPCFG_TTA	0x1FFFE
#define PPCFG_PSSO_EN	(1 << 28)
#define PPCFG_PSS_EN	(1 << 29)
#define PPCFG_ESDF_EN	(1 << 31)

#define PP2C_CIBGMN	0x0F
#define PP2C_CIBGMX	(0x25 << 8)
#define PP2C_CIBGN	(0x18 << 16)
#define PP2C_CINMP	(0x29 << 24)

#define PP3C_CWBGMN	0x04
#define PP3C_CWBGMX	(0x0B << 8)
#define PP3C_CWBGN	(0x08 << 16)
#define PP3C_CWNMP	(0x0F << 24)

#define PP4C_BMX	0x0a
#define PP4C_BNM	(0x08 << 8)
#define PP4C_SFD	(0x4a << 16)
#define PP4C_PTST	(0x06 << 24)

#define PP5C_RIT	0x60216
#define PP5C_RCT	(0x7f0 << 20)

#define PTC_RX_WM_VAL	0x40
#define PTC_RSVD	(1 << 27)

#define PORT0_BASE	0x100
#define PORT1_BASE	0x180

/* Port Control Register Bit Definitions */
#define PORT_SCTL_SPD_GEN3	(0x3 << 4)
#define PORT_SCTL_SPD_GEN2	(0x2 << 4)
#define PORT_SCTL_SPD_GEN1	(0x1 << 4)
#define PORT_SCTL_IPM		(0x3 << 8)

#define PORT_BASE	0x100
#define PORT_OFFSET	0x80
#define NR_PORTS	2
#define DRV_NAME	"ahci-ceva"
#define CEVA_FLAG_BROKEN_GEN2	1

static int ceva_init_sata(ulong mmio)
{
	ulong tmp;
	int i;

	/*
	 * AXI Data bus width to 64
	 * Set Mem Addr Read, Write ID for data transfers
	 * Transfer limit to 72 DWord
	 */
	tmp = PAXIC_ADBW_BW64 | PAXIC_MAWIDD | PAXIC_MARIDD | PAXIC_OTL;
	writel(tmp, mmio + AHCI_VEND_PAXIC);

	/* Set AHCI Enable */
	tmp = readl(mmio + HOST_CTL);
	tmp |= HOST_AHCI_EN;
	writel(tmp, mmio + HOST_CTL);

	for (i = 0; i < NR_PORTS; i++) {
		/* TPSS TPRS scalars, CISE and Port Addr */
		tmp = PCFG_TPSS_VAL | PCFG_TPRS_VAL | (PCFG_PAD_VAL + i);
		writel(tmp, mmio + AHCI_VEND_PCFG);

		/* Port Phy Cfg register enables */
		tmp = PPCFG_TTA | PPCFG_PSS_EN | PPCFG_ESDF_EN;
		writel(tmp, mmio + AHCI_VEND_PPCFG);

		/* Rx Watermark setting  */
		tmp = PTC_RX_WM_VAL | PTC_RSVD;
		writel(tmp, mmio + AHCI_VEND_PTC);

		/* Default to Gen 2 Speed and Gen 1 if Gen2 is broken */
		tmp = PORT_SCTL_SPD_GEN3 | PORT_SCTL_IPM;
		writel(tmp, mmio + PORT_SCR_CTL + PORT_BASE + PORT_OFFSET * i);
	}
	return 0;
}

static int sata_ceva_probe(struct udevice *dev)
{
	struct scsi_platdata *plat = dev_get_uclass_platdata(dev);

	ceva_init_sata(plat->base);

	return achi_init_one_dm(dev);
}

static const struct udevice_id sata_ceva_ids[] = {
	{ .compatible = "ceva,ahci-1v84" },
	{ }
};

static int sata_ceva_ofdata_to_platdata(struct udevice *dev)
{
	struct scsi_platdata *plat = dev_get_uclass_platdata(dev);

	plat->base = devfdt_get_addr(dev);
	if (plat->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Hardcode number for ceva sata controller */
	plat->max_lun = 1; /* Actually two but untested */
	plat->max_id = 2;

	return 0;
}

U_BOOT_DRIVER(ceva_host_blk) = {
	.name = "ceva_sata",
	.id = UCLASS_SCSI,
	.of_match = sata_ceva_ids,
	.ops = &scsi_ops,
	.probe = sata_ceva_probe,
	.ofdata_to_platdata = sata_ceva_ofdata_to_platdata,
};
