/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ATMEL_MPDDRC_H__
#define __ATMEL_MPDDRC_H__

/*
 * Only define the needed register in mpddr
 * If other register needed, will add them later
 */
struct atmel_mpddr {
	u32 mr;
	u32 rtr;
	u32 cr;
	u32 tpr0;
	u32 tpr1;
	u32 tpr2;
	u32 reserved[2];
	u32 md;
};


int ddr2_init(const unsigned int base,
	      const unsigned int ram_address,
	      const struct atmel_mpddr *mpddr);

/* Bit field in mode register */
#define ATMEL_MPDDRC_MR_MODE_NORMAL_CMD		0x0
#define ATMEL_MPDDRC_MR_MODE_NOP_CMD		0x1
#define ATMEL_MPDDRC_MR_MODE_PRCGALL_CMD	0x2
#define ATMEL_MPDDRC_MR_MODE_LMR_CMD		0x3
#define ATMEL_MPDDRC_MR_MODE_RFSH_CMD		0x4
#define ATMEL_MPDDRC_MR_MODE_EXT_LMR_CMD	0x5
#define ATMEL_MPDDRC_MR_MODE_DEEP_CMD		0x6
#define ATMEL_MPDDRC_MR_MODE_LPDDR2_CMD		0x7

/* Bit field in configuration register */
#define ATMEL_MPDDRC_CR_NC_MASK			0x3
#define ATMEL_MPDDRC_CR_NC_COL_9		0x0
#define ATMEL_MPDDRC_CR_NC_COL_10		0x1
#define ATMEL_MPDDRC_CR_NC_COL_11		0x2
#define ATMEL_MPDDRC_CR_NC_COL_12		0x3
#define ATMEL_MPDDRC_CR_NR_MASK			(0x3 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_11		(0x0 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_12		(0x1 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_13		(0x2 << 2)
#define ATMEL_MPDDRC_CR_NR_ROW_14		(0x3 << 2)
#define ATMEL_MPDDRC_CR_CAS_MASK		(0x7 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS2		(0x2 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS3		(0x3 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS4		(0x4 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS5		(0x5 << 4)
#define ATMEL_MPDDRC_CR_CAS_DDR_CAS6		(0x6 << 4)
#define ATMEL_MPDDRC_CR_DLL_RESET_ENABLED	(0x1 << 7)
#define ATMEL_MPDDRC_CR_DIC_DS			(0x1 << 8)
#define ATMEL_MPDDRC_CR_DIS_DLL			(0x1 << 9)
#define ATMEL_MPDDRC_CR_OCD_DEFAULT		(0x7 << 12)
#define ATMEL_MPDDRC_CR_DQMS_SHARED		(0x1 << 16)
#define ATMEL_MPDDRC_CR_ENRDM_ON		(0x1 << 17)
#define ATMEL_MPDDRC_CR_NB_8BANKS		(0x1 << 20)
#define ATMEL_MPDDRC_CR_NDQS_DISABLED		(0x1 << 21)
#define ATMEL_MPDDRC_CR_DECOD_INTERLEAVED	(0x1 << 22)
#define ATMEL_MPDDRC_CR_UNAL_SUPPORTED		(0x1 << 23)

/* Bit field in timing parameter 0 register */
#define ATMEL_MPDDRC_TPR0_TRAS_OFFSET		0
#define ATMEL_MPDDRC_TPR0_TRAS_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRCD_OFFSET		4
#define ATMEL_MPDDRC_TPR0_TRCD_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TWR_OFFSET		8
#define ATMEL_MPDDRC_TPR0_TWR_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRC_OFFSET		12
#define ATMEL_MPDDRC_TPR0_TRC_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRP_OFFSET		16
#define ATMEL_MPDDRC_TPR0_TRP_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TRRD_OFFSET		20
#define ATMEL_MPDDRC_TPR0_TRRD_MASK		0xf
#define ATMEL_MPDDRC_TPR0_TWTR_OFFSET		24
#define ATMEL_MPDDRC_TPR0_TWTR_MASK		0x7
#define ATMEL_MPDDRC_TPR0_RDC_WRRD_OFFSET	27
#define ATMEL_MPDDRC_TPR0_RDC_WRRD_MASK		0x1
#define ATMEL_MPDDRC_TPR0_TMRD_OFFSET		28
#define ATMEL_MPDDRC_TPR0_TMRD_MASK		0xf

/* Bit field in timing parameter 1 register */
#define ATMEL_MPDDRC_TPR1_TRFC_OFFSET		0
#define ATMEL_MPDDRC_TPR1_TRFC_MASK		0x7f
#define ATMEL_MPDDRC_TPR1_TXSNR_OFFSET		8
#define ATMEL_MPDDRC_TPR1_TXSNR_MASK		0xff
#define ATMEL_MPDDRC_TPR1_TXSRD_OFFSET		16
#define ATMEL_MPDDRC_TPR1_TXSRD_MASK		0xff
#define ATMEL_MPDDRC_TPR1_TXP_OFFSET		24
#define ATMEL_MPDDRC_TPR1_TXP_MASK		0xf

/* Bit field in timing parameter 2 register */
#define ATMEL_MPDDRC_TPR2_TXARD_OFFSET		0
#define ATMEL_MPDDRC_TPR2_TXARD_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TXARDS_OFFSET		4
#define ATMEL_MPDDRC_TPR2_TXARDS_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TRPA_OFFSET		8
#define ATMEL_MPDDRC_TPR2_TRPA_MASK		0xf
#define ATMEL_MPDDRC_TPR2_TRTP_OFFSET		12
#define ATMEL_MPDDRC_TPR2_TRTP_MASK		0x7
#define ATMEL_MPDDRC_TPR2_TFAW_OFFSET		16
#define ATMEL_MPDDRC_TPR2_TFAW_MASK		0xf

/* Bit field in Memory Device Register */
#define ATMEL_MPDDRC_MD_LPDDR_SDRAM	0x3
#define ATMEL_MPDDRC_MD_DDR2_SDRAM	0x6
#define ATMEL_MPDDRC_MD_DBW_MASK	(0x1 << 4)
#define ATMEL_MPDDRC_MD_DBW_32_BITS	(0x0 << 4)
#define ATMEL_MPDDRC_MD_DBW_16_BITS	(0x1 << 4)

#endif
