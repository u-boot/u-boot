/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <ahci.h>
#include <scsi.h>

/* port register default value */
#define AHCI_PORT_PHY_1_CFG	0xa003fffe
#define AHCI_PORT_PHY_2_CFG	0x28183414
#define AHCI_PORT_PHY_3_CFG	0x0e080e06
#define AHCI_PORT_PHY_4_CFG	0x064a080b
#define AHCI_PORT_PHY_5_CFG	0x2aa86470
#define AHCI_PORT_TRANS_CFG	0x08000029

#define SATA_ECC_REG_ADDR	0x20220520
#define SATA_ECC_DISABLE	0x00020000

int ls1021a_sata_init(void)
{
	struct ccsr_ahci __iomem *ccsr_ahci = (void *)AHCI_BASE_ADDR;

#ifdef CONFIG_SYS_FSL_ERRATUM_A008407
	out_le32((void *)SATA_ECC_REG_ADDR, SATA_ECC_DISABLE);
#endif

	out_le32(&ccsr_ahci->ppcfg, AHCI_PORT_PHY_1_CFG);
	out_le32(&ccsr_ahci->pp2c, AHCI_PORT_PHY_2_CFG);
	out_le32(&ccsr_ahci->pp3c, AHCI_PORT_PHY_3_CFG);
	out_le32(&ccsr_ahci->pp4c, AHCI_PORT_PHY_4_CFG);
	out_le32(&ccsr_ahci->pp5c, AHCI_PORT_PHY_5_CFG);
	out_le32(&ccsr_ahci->ptc, AHCI_PORT_TRANS_CFG);

	ahci_init((void __iomem *)AHCI_BASE_ADDR);
	scsi_scan(false);

	return 0;
}
