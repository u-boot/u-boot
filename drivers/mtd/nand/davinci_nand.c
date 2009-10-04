/*
 * NAND driver for TI DaVinci based boards.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on Linux DaVinci NAND driver by TI. Original copyright follows:
 */

/*
 *
 * linux/drivers/mtd/nand/nand_davinci.c
 *
 * NAND Flash Driver
 *
 * Copyright (C) 2006 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 *
 *  Overview:
 *   This is a device driver for the NAND flash device found on the
 *   DaVinci board which utilizes the Samsung k9k2g08 part.
 *
 Modifications:
 ver. 1.0: Feb 2005, Vinod/Sudhakar
 -
 *
 */

#include <common.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/emif_defs.h>

/* Definitions for 4-bit hardware ECC */
#define NAND_TIMEOUT			10240
#define NAND_ECC_BUSY			0xC
#define NAND_4BITECC_MASK		0x03FF03FF
#define EMIF_NANDFSR_ECC_STATE_MASK  	0x00000F00
#define ECC_STATE_NO_ERR		0x0
#define ECC_STATE_TOO_MANY_ERRS		0x1
#define ECC_STATE_ERR_CORR_COMP_P	0x2
#define ECC_STATE_ERR_CORR_COMP_N	0x3

static emif_registers *const emif_regs = (void *) DAVINCI_ASYNC_EMIF_CNTRL_BASE;

static void nand_davinci_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct		nand_chip *this = mtd->priv;
	u_int32_t	IO_ADDR_W = (u_int32_t)this->IO_ADDR_W;

	IO_ADDR_W &= ~(MASK_ALE|MASK_CLE);

	if (ctrl & NAND_CTRL_CHANGE) {
		if ( ctrl & NAND_CLE )
			IO_ADDR_W |= MASK_CLE;
		if ( ctrl & NAND_ALE )
			IO_ADDR_W |= MASK_ALE;
		this->IO_ADDR_W = (void __iomem *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SYS_NAND_HW_ECC

static void nand_davinci_enable_hwecc(struct mtd_info *mtd, int mode)
{
	int		dummy;

	dummy = emif_regs->NANDF1ECC;

	/* FIXME:  only chipselect 0 is supported for now */
	emif_regs->NANDFCR |= 1 << 8;
}

static u_int32_t nand_davinci_readecc(struct mtd_info *mtd, u_int32_t region)
{
	u_int32_t	ecc = 0;

	if (region == 1)
		ecc = emif_regs->NANDF1ECC;
	else if (region == 2)
		ecc = emif_regs->NANDF2ECC;
	else if (region == 3)
		ecc = emif_regs->NANDF3ECC;
	else if (region == 4)
		ecc = emif_regs->NANDF4ECC;

	return(ecc);
}

static int nand_davinci_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	u_int32_t		tmp;
	const int region = 1;

	tmp = nand_davinci_readecc(mtd, region);

	/* Squeeze 4 bytes ECC into 3 bytes by removing RESERVED bits
	 * and shifting. RESERVED bits are 31 to 28 and 15 to 12. */
	tmp = (tmp & 0x00000fff) | ((tmp & 0x0fff0000) >> 4);

	/* Invert so that erased block ECC is correct */
	tmp = ~tmp;

	*ecc_code++ = tmp;
	*ecc_code++ = tmp >>  8;
	*ecc_code++ = tmp >> 16;

	/* NOTE:  the above code matches mainline Linux:
	 *	.PQR.stu ==> ~PQRstu
	 *
	 * MontaVista/TI kernels encode those bytes differently, use
	 * complicated (and allegedly sometimes-wrong) correction code,
	 * and usually shipped with U-Boot that uses software ECC:
	 *	.PQR.stu ==> PsQRtu
	 *
	 * If you need MV/TI compatible NAND I/O in U-Boot, it should
	 * be possible to (a) change the mangling above, (b) reverse
	 * that mangling in nand_davinci_correct_data() below.
	 */

	return 0;
}

static int nand_davinci_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	struct nand_chip *this = mtd->priv;
	u_int32_t ecc_nand = read_ecc[0] | (read_ecc[1] << 8) |
					  (read_ecc[2] << 16);
	u_int32_t ecc_calc = calc_ecc[0] | (calc_ecc[1] << 8) |
					  (calc_ecc[2] << 16);
	u_int32_t diff = ecc_calc ^ ecc_nand;

	if (diff) {
		if ((((diff >> 12) ^ diff) & 0xfff) == 0xfff) {
			/* Correctable error */
			if ((diff >> (12 + 3)) < this->ecc.size) {
				uint8_t find_bit = 1 << ((diff >> 12) & 7);
				uint32_t find_byte = diff >> (12 + 3);

				dat[find_byte] ^= find_bit;
				MTDDEBUG(MTD_DEBUG_LEVEL0, "Correcting single "
					 "bit ECC error at offset: %d, bit: "
					 "%d\n", find_byte, find_bit);
				return 1;
			} else {
				return -1;
			}
		} else if (!(diff & (diff - 1))) {
			/* Single bit ECC error in the ECC itself,
			   nothing to fix */
			MTDDEBUG(MTD_DEBUG_LEVEL0, "Single bit ECC error in "
				 "ECC.\n");
			return 1;
		} else {
			/* Uncorrectable error */
			MTDDEBUG(MTD_DEBUG_LEVEL0, "ECC UNCORRECTED_ERROR 1\n");
			return -1;
		}
	}
	return(0);
}
#endif /* CONFIG_SYS_NAND_HW_ECC */

#ifdef CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
static struct nand_ecclayout nand_davinci_4bit_layout_oobfirst = {
/*
 * TI uses a different layout for 4K page deviecs. Since the
 * eccpos filed can hold only a limited number of entries, adding
 * support for 4K page will result in compilation warnings
 * 4K Support will be added later
 */
#ifdef CONFIG_SYS_NAND_PAGE_2K
	.eccbytes = 40,
	.eccpos = {
		24, 25, 26, 27, 28,
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
		49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
		59, 60, 61, 62, 63,
		},
	.oobfree = {
		{.offset = 2, .length = 22, },
	},
#endif
};

static void nand_davinci_4bit_enable_hwecc(struct mtd_info *mtd, int mode)
{
	u32 val;

	switch (mode) {
	case NAND_ECC_WRITE:
	case NAND_ECC_READ:
		/*
		 * Start a new ECC calculation for reading or writing 512 bytes
		 * of data.
		 */
		val = (emif_regs->NANDFCR & ~(3 << 4)) | (1 << 12);
		emif_regs->NANDFCR = val;
		break;
	case NAND_ECC_READSYN:
		val = emif_regs->NAND4BITECC1;
		break;
	default:
		break;
	}
}

static u32 nand_davinci_4bit_readecc(struct mtd_info *mtd, unsigned int ecc[4])
{
	ecc[0] = emif_regs->NAND4BITECC1 & NAND_4BITECC_MASK;
	ecc[1] = emif_regs->NAND4BITECC2 & NAND_4BITECC_MASK;
	ecc[2] = emif_regs->NAND4BITECC3 & NAND_4BITECC_MASK;
	ecc[3] = emif_regs->NAND4BITECC4 & NAND_4BITECC_MASK;

	return 0;
}

static int nand_davinci_4bit_calculate_ecc(struct mtd_info *mtd,
					   const uint8_t *dat,
					   uint8_t *ecc_code)
{
	unsigned int hw_4ecc[4] = { 0, 0, 0, 0 };
	unsigned int const1 = 0, const2 = 0;
	unsigned char count1 = 0;

	nand_davinci_4bit_readecc(mtd, hw_4ecc);

	/*Convert 10 bit ecc value to 8 bit */
	for (count1 = 0; count1 < 2; count1++) {
		const2 = count1 * 5;
		const1 = count1 * 2;

		/* Take first 8 bits from val1 (count1=0) or val5 (count1=1) */
		ecc_code[const2] = hw_4ecc[const1] & 0xFF;

		/*
		 * Take 2 bits as LSB bits from val1 (count1=0) or val5
		 * (count1=1) and 6 bits from val2 (count1=0) or
		 * val5 (count1=1)
		 */
		ecc_code[const2 + 1] =
		    ((hw_4ecc[const1] >> 8) & 0x3) | ((hw_4ecc[const1] >> 14) &
						      0xFC);

		/*
		 * Take 4 bits from val2 (count1=0) or val5 (count1=1) and
		 * 4 bits from val3 (count1=0) or val6 (count1=1)
		 */
		ecc_code[const2 + 2] =
		    ((hw_4ecc[const1] >> 22) & 0xF) |
		    ((hw_4ecc[const1 + 1] << 4) & 0xF0);

		/*
		 * Take 6 bits from val3(count1=0) or val6 (count1=1) and
		 * 2 bits from val4 (count1=0) or  val7 (count1=1)
		 */
		ecc_code[const2 + 3] =
		    ((hw_4ecc[const1 + 1] >> 4) & 0x3F) |
		    ((hw_4ecc[const1 + 1] >> 10) & 0xC0);

		/* Take 8 bits from val4 (count1=0) or val7 (count1=1) */
		ecc_code[const2 + 4] = (hw_4ecc[const1 + 1] >> 18) & 0xFF;
	}
	return 0;
}


static int nand_davinci_4bit_correct_data(struct mtd_info *mtd, uint8_t *dat,
					  uint8_t *read_ecc, uint8_t *calc_ecc)
{
	unsigned short ecc_10bit[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int i;
	unsigned int hw_4ecc[4] = { 0, 0, 0, 0 }, iserror = 0;
	unsigned short *pspare = NULL, *pspare1 = NULL;
	unsigned int numerrors, erroraddress, errorvalue;
	u32 val;

	/*
	 * Check for an ECC where all bytes are 0xFF.  If this is the case, we
	 * will assume we are looking at an erased page and we should ignore
	 * the ECC.
	 */
	for (i = 0; i < 10; i++) {
		if (read_ecc[i] != 0xFF)
			break;
	}
	if (i == 10)
		return 0;

	/* Convert 8 bit in to 10 bit */
	pspare = (unsigned short *)&read_ecc[2];
	pspare1 = (unsigned short *)&read_ecc[0];

	/* Take 10 bits from 0th and 1st bytes */
	ecc_10bit[0] = (*pspare1) & 0x3FF;

	/* Take 6 bits from 1st byte and 4 bits from 2nd byte */
	ecc_10bit[1] = (((*pspare1) >> 10) & 0x3F)
	    | (((pspare[0]) << 6) & 0x3C0);

	/* Take 4 bits form 2nd bytes and 6 bits from 3rd bytes */
	ecc_10bit[2] = ((pspare[0]) >> 4) & 0x3FF;

	/*Take 2 bits from 3rd byte and 8 bits from 4th byte */
	ecc_10bit[3] = (((pspare[0]) >> 14) & 0x3)
	    | ((((pspare[1])) << 2) & 0x3FC);

	/* Take 8 bits from 5th byte and 2 bits from 6th byte */
	ecc_10bit[4] = ((pspare[1]) >> 8)
	    | ((((pspare[2])) << 8) & 0x300);

	/* Take 6 bits from 6th byte and 4 bits from 7th byte */
	ecc_10bit[5] = (pspare[2] >> 2) & 0x3FF;

	/* Take 4 bits from 7th byte and 6 bits from 8th byte */
	ecc_10bit[6] = (((pspare[2]) >> 12) & 0xF)
	    | ((((pspare[3])) << 4) & 0x3F0);

	/*Take 2 bits from 8th byte and 8 bits from 9th byte */
	ecc_10bit[7] = ((pspare[3]) >> 6) & 0x3FF;

	/*
	 * Write the parity values in the NAND Flash 4-bit ECC Load register.
	 * Write each parity value one at a time starting from 4bit_ecc_val8
	 * to 4bit_ecc_val1.
	 */
	for (i = 7; i >= 0; i--)
		emif_regs->NAND4BITECCLOAD = ecc_10bit[i];

	/*
	 * Perform a dummy read to the EMIF Revision Code and Status register.
	 * This is required to ensure time for syndrome calculation after
	 * writing the ECC values in previous step.
	 */

	val = emif_regs->NANDFSR;

	/*
	 * Read the syndrome from the NAND Flash 4-Bit ECC 1-4 registers.
	 * A syndrome value of 0 means no bit errors. If the syndrome is
	 * non-zero then go further otherwise return.
	 */
	nand_davinci_4bit_readecc(mtd, hw_4ecc);

	if (hw_4ecc[0] == ECC_STATE_NO_ERR && hw_4ecc[1] == ECC_STATE_NO_ERR &&
	    hw_4ecc[2] == ECC_STATE_NO_ERR && hw_4ecc[3] == ECC_STATE_NO_ERR)
		return 0;

	/*
	 * Clear any previous address calculation by doing a dummy read of an
	 * error address register.
	 */
	val = emif_regs->NANDERRADD1;

	/*
	 * Set the addr_calc_st bit(bit no 13) in the NAND Flash Control
	 * register to 1.
	 */
	emif_regs->NANDFCR |= 1 << 13;

	/*
	 * Wait for the corr_state field (bits 8 to 11)in the
	 * NAND Flash Status register to be equal to 0x0, 0x1, 0x2, or 0x3.
	 */
	i = NAND_TIMEOUT;
	do {
		val = emif_regs->NANDFSR;
		val &= 0xc00;
		i--;
	} while ((i > 0) && val);

	iserror = emif_regs->NANDFSR;
	iserror &= EMIF_NANDFSR_ECC_STATE_MASK;
	iserror = iserror >> 8;

	/*
	 * ECC_STATE_TOO_MANY_ERRS (0x1) means errors cannot be
	 * corrected (five or more errors).  The number of errors
	 * calculated (err_num field) differs from the number of errors
	 * searched.  ECC_STATE_ERR_CORR_COMP_P (0x2) means error
	 * correction complete (errors on bit 8 or 9).
	 * ECC_STATE_ERR_CORR_COMP_N (0x3) means error correction
	 * complete (error exists).
	 */

	if (iserror == ECC_STATE_NO_ERR) {
		val = emif_regs->NANDERRVAL1;
		return 0;
	} else if (iserror == ECC_STATE_TOO_MANY_ERRS) {
		val = emif_regs->NANDERRVAL1;
		return -1;
	}

	numerrors = ((emif_regs->NANDFSR >> 16) & 0x3) + 1;

	/* Read the error address, error value and correct */
	for (i = 0; i < numerrors; i++) {
		if (i > 1) {
			erroraddress =
			    ((emif_regs->NANDERRADD2 >>
			      (16 * (i & 1))) & 0x3FF);
			erroraddress = ((512 + 7) - erroraddress);
			errorvalue =
			    ((emif_regs->NANDERRVAL2 >>
			      (16 * (i & 1))) & 0xFF);
		} else {
			erroraddress =
			    ((emif_regs->NANDERRADD1 >>
			      (16 * (i & 1))) & 0x3FF);
			erroraddress = ((512 + 7) - erroraddress);
			errorvalue =
			    ((emif_regs->NANDERRVAL1 >>
			      (16 * (i & 1))) & 0xFF);
		}
		/* xor the corrupt data with error value */
		if (erroraddress < 512)
			dat[erroraddress] ^= errorvalue;
	}

	return numerrors;
}
#endif /* CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST */

static int nand_davinci_dev_ready(struct mtd_info *mtd)
{
	return emif_regs->NANDFSR & 0x1;
}

static void nand_flash_init(void)
{
	/* This is for DM6446 EVM and *very* similar.  DO NOT GROW THIS!
	 * Instead, have your board_init() set EMIF timings, based on its
	 * knowledge of the clocks and what devices are hooked up ... and
	 * don't even do that unless no UBL handled it.
	 */
#ifdef CONFIG_SOC_DM644X
	u_int32_t	acfg1 = 0x3ffffffc;

	/*------------------------------------------------------------------*
	 *  NAND FLASH CHIP TIMEOUT @ 459 MHz                               *
	 *                                                                  *
	 *  AEMIF.CLK freq   = PLL1/6 = 459/6 = 76.5 MHz                    *
	 *  AEMIF.CLK period = 1/76.5 MHz = 13.1 ns                         *
	 *                                                                  *
	 *------------------------------------------------------------------*/
	 acfg1 = 0
		| (0 << 31 )	/* selectStrobe */
		| (0 << 30 )	/* extWait */
		| (1 << 26 )	/* writeSetup	10 ns */
		| (3 << 20 )	/* writeStrobe	40 ns */
		| (1 << 17 )	/* writeHold	10 ns */
		| (1 << 13 )	/* readSetup	10 ns */
		| (5 << 7 )	/* readStrobe	60 ns */
		| (1 << 4 )	/* readHold	10 ns */
		| (3 << 2 )	/* turnAround	?? ns */
		| (0 << 0 )	/* asyncSize	8-bit bus */
		;

	emif_regs->AB1CR = acfg1; /* CS2 */

	emif_regs->NANDFCR = 0x00000101; /* NAND flash on CS2 */
#endif
}

void davinci_nand_init(struct nand_chip *nand)
{
	nand->chip_delay  = 0;
#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT
	nand->options	  |= NAND_USE_FLASH_BBT;
#endif
#ifdef CONFIG_SYS_NAND_HW_ECC
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.size = 512;
	nand->ecc.bytes = 3;
	nand->ecc.calculate = nand_davinci_calculate_ecc;
	nand->ecc.correct  = nand_davinci_correct_data;
	nand->ecc.hwctl  = nand_davinci_enable_hwecc;
#else
	nand->ecc.mode = NAND_ECC_SOFT;
#endif /* CONFIG_SYS_NAND_HW_ECC */
#ifdef CONFIG_SYS_NAND_4BIT_HW_ECC_OOBFIRST
	nand->ecc.mode = NAND_ECC_HW_OOB_FIRST;
	nand->ecc.size = 512;
	nand->ecc.bytes = 10;
	nand->ecc.calculate = nand_davinci_4bit_calculate_ecc;
	nand->ecc.correct = nand_davinci_4bit_correct_data;
	nand->ecc.hwctl = nand_davinci_4bit_enable_hwecc;
	nand->ecc.layout = &nand_davinci_4bit_layout_oobfirst;
#endif
	/* Set address of hardware control function */
	nand->cmd_ctrl = nand_davinci_hwcontrol;

	nand->dev_ready = nand_davinci_dev_ready;

	nand_flash_init();
}

int board_nand_init(struct nand_chip *chip) __attribute__((weak));

int board_nand_init(struct nand_chip *chip)
{
	davinci_nand_init(chip);
	return 0;
}
