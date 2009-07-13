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
	nand->options	  = NAND_USE_FLASH_BBT;
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
