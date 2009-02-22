/*
 * (C) Copyright 2007 STMicroelectronics, <www.st.com>
 * (C) Copyright 2009 Alessandro Rubini <rubini@unipv.it>
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

#include <common.h>
#include <nand.h>
#include <asm/io.h>

static inline int parity(int b) /* b is really a byte; returns 0 or ~0 */
{
	__asm__ __volatile__(
		"eor   %0, %0, %0, lsr #4\n\t"
		"eor   %0, %0, %0, lsr #2\n\t"
		"eor   %0, %0, %0, lsr #1\n\t"
		"ands  %0, %0, #1\n\t"
		"subne %0, %0, #2\t"
		: "=r" (b) : "0" (b));
	return b;
}

/*
 * This is the ECC routine used in hardware, according to the manual.
 * HW claims to make the calculation but not the correction; so we must
 * recalculate the bytes for a comparison.
 */
static int ecc512(const unsigned char *data, unsigned char *ecc)
{
	int gpar = 0;
	int i, val, par;
	int pbits = 0;		/* P8, P16, ... P2048 */
	int pprime = 0;		/* P8', P16', ... P2048' */
	int lowbits;		/* P1, P2, P4 and primes */

	for (i = 0; i < 512; i++) {
		par = parity((val = data[i]));
		gpar ^= val;
		pbits ^= (i & par);
	}
	/*
	 * Ok, now gpar is global parity (xor of all bytes)
	 * pbits are all the parity bits (non-prime ones)
	 */
	par = parity(gpar);
	pprime = pbits ^ par;
	/* Put low bits in the right position for ecc[2] (bits 7..2) */
	lowbits = 0
		| (parity(gpar & 0xf0) & 0x80)	/* P4  */
		| (parity(gpar & 0x0f) & 0x40)	/* P4' */
		| (parity(gpar & 0xcc) & 0x20)	/* P2  */
		| (parity(gpar & 0x33) & 0x10)	/* P2' */
		| (parity(gpar & 0xaa) & 0x08)	/* P1  */
		| (parity(gpar & 0x55) & 0x04);	/* P1' */

	ecc[2] = ~(lowbits | ((pbits & 0x100) >> 7) | ((pprime & 0x100) >> 8));
	/* now intermix bits for ecc[1] (P1024..P128') and ecc[0] (P64..P8') */
	ecc[1] = ~(    (pbits & 0x80) >> 0  | ((pprime & 0x80) >> 1)
		    | ((pbits & 0x40) >> 1) | ((pprime & 0x40) >> 2)
		    | ((pbits & 0x20) >> 2) | ((pprime & 0x20) >> 3)
		    | ((pbits & 0x10) >> 3) | ((pprime & 0x10) >> 4));

	ecc[0] = ~(    (pbits & 0x8) << 4  | ((pprime & 0x8) << 3)
		    | ((pbits & 0x4) << 3) | ((pprime & 0x4) << 2)
		    | ((pbits & 0x2) << 2) | ((pprime & 0x2) << 1)
		    | ((pbits & 0x1) << 1) | ((pprime & 0x1) << 0));
	return 0;
}

/* This is the method in the chip->ecc field */
static int nomadik_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat,
				 uint8_t *ecc_code)
{
	return ecc512(dat, ecc_code);
}

static int nomadik_ecc_correct(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *r_ecc, uint8_t *c_ecc)
{
	struct nand_chip *chip = mtd->priv;
	uint32_t r, c, d, diff; /*read, calculated, xor of them */

	if (!memcmp(r_ecc, c_ecc, chip->ecc.bytes))
		return 0;

	/* Reorder the bytes into ascending-order 24 bits -- see manual */
	r = r_ecc[2] << 22 | r_ecc[1] << 14 | r_ecc[0] << 6 | r_ecc[2] >> 2;
	c = c_ecc[2] << 22 | c_ecc[1] << 14 | c_ecc[0] << 6 | c_ecc[2] >> 2;
	diff = (r ^ c) & ((1<<24)-1); /* use 24 bits only */

	/* If 12 bits are different, one per pair, it's correctable */
	if (((diff | (diff>>1)) & 0x555555) == 0x555555) {
		int bit = ((diff & 2) >> 1)
			| ((diff & 0x8) >> 2) | ((diff & 0x20) >> 3);
		int byte;

		d = diff >> 6; /* remove bit-order info */
		byte =  ((d & 2) >> 1)
			| ((d & 0x8) >> 2) | ((d & 0x20) >> 3)
			| ((d & 0x80) >> 4) | ((d & 0x200) >> 5)
			| ((d & 0x800) >> 6) | ((d & 0x2000) >> 7)
			| ((d & 0x8000) >> 8) | ((d & 0x20000) >> 9);
		/* correct the single bit */
		dat[byte] ^= 1<<bit;
		return 0;
	}
	/* If 1 bit only differs, it's one bit error in ECC, ignore */
	if ((diff ^ (1 << (ffs(diff) - 1))) == 0)
		return 0;
	/* Otherwise, uncorrectable */
	return -1;
}

static void nomadik_ecc_hwctl(struct mtd_info *mtd, int mode)
{ /* mandatory in the structure but not used here */ }


/* This is the layout used by older installations, we keep compatible */
struct nand_ecclayout nomadik_ecc_layout = {
	.eccbytes = 3 * 4,
	.eccpos = { /* each subpage has 16 bytes: pos 2,3,4 hosts ECC */
		0x02, 0x03, 0x04,
		0x12, 0x13, 0x14,
		0x22, 0x23, 0x24,
		0x32, 0x33, 0x34},
	.oobfree = { {0x08, 0x08}, {0x18, 0x08}, {0x28, 0x08}, {0x38, 0x08} },
};

#define MASK_ALE	(1 << 24)	/* our ALE is AD21 */
#define MASK_CLE	(1 << 23)	/* our CLE is AD22 */

/* This is copied from the AT91SAM9 devices (Stelian Pop, Lead Tech Design) */
static void nomadik_nand_hwcontrol(struct mtd_info *mtd,
				   int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	u32 pcr0 = readl(REG_FSMC_PCR0);

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
		IO_ADDR_W &= ~(MASK_ALE | MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= MASK_ALE;

		if (ctrl & NAND_NCE)
			writel(pcr0 | 0x4, REG_FSMC_PCR0);
		else
			writel(pcr0 & ~0x4, REG_FSMC_PCR0);

		this->IO_ADDR_W = (void *) IO_ADDR_W;
		this->IO_ADDR_R = (void *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

/* Returns 1 when ready; upper layers timeout at 20ms with timer routines */
static int nomadik_nand_ready(struct mtd_info *mtd)
{
	return 1; /* The ready bit is handled in hardware */
}

/* Copy a buffer 32bits at a time: faster than defualt method which is 8bit */
static void nomadik_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;
	u32 *p = (u32 *) buf;

	len >>= 2;
	writel(0, REG_FSMC_ECCR0);
	for (i = 0; i < len; i++)
		p[i] = readl(chip->IO_ADDR_R);
}

int board_nand_init(struct nand_chip *chip)
{
	/* Set up the FSMC_PCR0 for nand access*/
	writel(0x0000004a, REG_FSMC_PCR0);
	/* Set up FSMC_PMEM0, FSMC_PATT0 with timing data for access */
	writel(0x00020401, REG_FSMC_PMEM0);
	writel(0x00020404, REG_FSMC_PATT0);

	chip->options = NAND_COPYBACK |	NAND_CACHEPRG | NAND_NO_PADDING;
	chip->cmd_ctrl = nomadik_nand_hwcontrol;
	chip->dev_ready = nomadik_nand_ready;
	/* The chip allows 32bit reads, so avoid the default 8bit copy */
	chip->read_buf = nomadik_nand_read_buf;

	/* ECC: follow the hardware-defined rulse, but do it in sw */
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.bytes = 3;
	chip->ecc.size = 512;
	chip->ecc.layout = &nomadik_ecc_layout;
	chip->ecc.calculate = nomadik_ecc_calculate;
	chip->ecc.hwctl = nomadik_ecc_hwctl;
	chip->ecc.correct = nomadik_ecc_correct;

	return 0;
}
