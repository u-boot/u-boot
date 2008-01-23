/*
 * SSI Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SSI_H__
#define __SSI_H__

/*********************************************************************
* Synchronous Serial Interface (SSI)
*********************************************************************/

typedef struct ssi {
	u32 tx0;
	u32 tx1;
	u32 rx0;
	u32 rx1;
	u32 cr;
	u32 isr;
	u32 ier;
	u32 tcr;
	u32 rcr;
	u32 ccr;
	u8 resv0[0x4];
	u32 fcsr;
	u8 resv1[0x8];
	u32 acr;
	u32 acadd;
	u32 acdat;
	u32 atag;
	u32 tmask;
	u32 rmask;
} ssi_t;

/*********************************************************************
* Synchronous Serial Interface (SSI)
*********************************************************************/

/* Bit definitions and macros for SSI_CR */
#define SSI_CR_CIS			(0x00000200)
#define SSI_CR_TCH			(0x00000100)
#define SSI_CR_MCE			(0x00000080)
#define SSI_CR_I2S_SLAVE		(0x00000040)
#define SSI_CR_I2S_MASTER		(0x00000020)
#define SSI_CR_I2S_NORMAL		(0x00000000)
#define SSI_CR_SYN			(0x00000010)
#define SSI_CR_NET			(0x00000008)
#define SSI_CR_RE			(0x00000004)
#define SSI_CR_TE			(0x00000002)
#define SSI_CR_SSI_EN			(0x00000001)

/* Bit definitions and macros for SSI_ISR */
#define SSI_ISR_CMDAU			(0x00040000)
#define SSI_ISR_CMDDU			(0x00020000)
#define SSI_ISR_RXT			(0x00010000)
#define SSI_ISR_RDR1			(0x00008000)
#define SSI_ISR_RDR0			(0x00004000)
#define SSI_ISR_TDE1			(0x00002000)
#define SSI_ISR_TDE0			(0x00001000)
#define SSI_ISR_ROE1			(0x00000800)
#define SSI_ISR_ROE0			(0x00000400)
#define SSI_ISR_TUE1			(0x00000200)
#define SSI_ISR_TUE0			(0x00000100)
#define SSI_ISR_TFS			(0x00000080)
#define SSI_ISR_RFS			(0x00000040)
#define SSI_ISR_TLS			(0x00000020)
#define SSI_ISR_RLS			(0x00000010)
#define SSI_ISR_RFF1			(0x00000008)
#define SSI_ISR_RFF0			(0x00000004)
#define SSI_ISR_TFE1			(0x00000002)
#define SSI_ISR_TFE0			(0x00000001)

/* Bit definitions and macros for SSI_IER */
#define SSI_IER_RDMAE			(0x00400000)
#define SSI_IER_RIE			(0x00200000)
#define SSI_IER_TDMAE			(0x00100000)
#define SSI_IER_TIE			(0x00080000)
#define SSI_IER_CMDAU			(0x00040000)
#define SSI_IER_CMDU			(0x00020000)
#define SSI_IER_RXT			(0x00010000)
#define SSI_IER_RDR1			(0x00008000)
#define SSI_IER_RDR0			(0x00004000)
#define SSI_IER_TDE1			(0x00002000)
#define SSI_IER_TDE0			(0x00001000)
#define SSI_IER_ROE1			(0x00000800)
#define SSI_IER_ROE0			(0x00000400)
#define SSI_IER_TUE1			(0x00000200)
#define SSI_IER_TUE0			(0x00000100)
#define SSI_IER_TFS			(0x00000080)
#define SSI_IER_RFS			(0x00000040)
#define SSI_IER_TLS			(0x00000020)
#define SSI_IER_RLS			(0x00000010)
#define SSI_IER_RFF1			(0x00000008)
#define SSI_IER_RFF0			(0x00000004)
#define SSI_IER_TFE1			(0x00000002)
#define SSI_IER_TFE0			(0x00000001)

/* Bit definitions and macros for SSI_TCR */
#define SSI_TCR_TXBIT0			(0x00000200)
#define SSI_TCR_TFEN1			(0x00000100)
#define SSI_TCR_TFEN0			(0x00000080)
#define SSI_TCR_TFDIR			(0x00000040)
#define SSI_TCR_TXDIR			(0x00000020)
#define SSI_TCR_TSHFD			(0x00000010)
#define SSI_TCR_TSCKP			(0x00000008)
#define SSI_TCR_TFSI			(0x00000004)
#define SSI_TCR_TFSL			(0x00000002)
#define SSI_TCR_TEFS			(0x00000001)

/* Bit definitions and macros for SSI_RCR */
#define SSI_RCR_RXEXT			(0x00000400)
#define SSI_RCR_RXBIT0			(0x00000200)
#define SSI_RCR_RFEN1			(0x00000100)
#define SSI_RCR_RFEN0			(0x00000080)
#define SSI_RCR_RSHFD			(0x00000010)
#define SSI_RCR_RSCKP			(0x00000008)
#define SSI_RCR_RFSI			(0x00000004)
#define SSI_RCR_RFSL			(0x00000002)
#define SSI_RCR_REFS			(0x00000001)

/* Bit definitions and macros for SSI_CCR */
#define SSI_CCR_DIV2			(0x00040000)
#define SSI_CCR_PSR			(0x00020000)
#define SSI_CCR_WL(x)			(((x)&0x0000000F)<<13)
#define SSI_CCR_DC(x)			(((x)&0x0000001F)<<8)
#define SSI_CCR_PM(x)			((x)&0x000000FF)

/* Bit definitions and macros for SSI_FCSR */
#define SSI_FCSR_RFCNT1(x)		(((x)&0x0000000F)<<28)
#define SSI_FCSR_TFCNT1(x)		(((x)&0x0000000F)<<24)
#define SSI_FCSR_RFWM1(x)		(((x)&0x0000000F)<<20)
#define SSI_FCSR_TFWM1(x)		(((x)&0x0000000F)<<16)
#define SSI_FCSR_RFCNT0(x)		(((x)&0x0000000F)<<12)
#define SSI_FCSR_TFCNT0(x)		(((x)&0x0000000F)<<8)
#define SSI_FCSR_RFWM0(x)		(((x)&0x0000000F)<<4)
#define SSI_FCSR_TFWM0(x)		((x)&0x0000000F)

/* Bit definitions and macros for SSI_ACR */
#define SSI_ACR_FRDIV(x)		(((x)&0x0000003F)<<5)
#define SSI_ACR_WR			(0x00000010)
#define SSI_ACR_RD			(0x00000008)
#define SSI_ACR_TIF			(0x00000004)
#define SSI_ACR_FV			(0x00000002)
#define SSI_ACR_AC97EN			(0x00000001)

/* Bit definitions and macros for SSI_ACADD */
#define SSI_ACADD_SSI_ACADD(x)		((x)&0x0007FFFF)

/* Bit definitions and macros for SSI_ACDAT */
#define SSI_ACDAT_SSI_ACDAT(x)		((x)&0x0007FFFF)

/* Bit definitions and macros for SSI_ATAG */
#define SSI_ATAG_DDI_ATAG(x)		((x)&0x0000FFFF)

#endif					/* __SSI_H__ */
