/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2006 ATMEL Rousset, Lacressonniere Nicolas
 *
 * Add Programmable Multibit ECC support for various AT91 SoC
 *     (C) Copyright 2012 ATMEL, Hong Xu
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_pio.h>

#include <nand.h>
#include <watchdog.h>

#ifdef CONFIG_ATMEL_NAND_HWECC

/* Register access macros */
#define ecc_readl(add, reg)				\
	readl(AT91_BASE_SYS + add + ATMEL_ECC_##reg)
#define ecc_writel(add, reg, value)			\
	writel((value), AT91_BASE_SYS + add + ATMEL_ECC_##reg)

#include "atmel_nand_ecc.h"	/* Hardware ECC registers */

#ifdef CONFIG_ATMEL_NAND_HW_PMECC

struct atmel_nand_host {
	struct pmecc_regs __iomem *pmecc;
	struct pmecc_errloc_regs __iomem *pmerrloc;
	void __iomem		*pmecc_rom_base;

	u8		pmecc_corr_cap;
	u16		pmecc_sector_size;
	u32		pmecc_index_table_offset;

	int		pmecc_bytes_per_sector;
	int		pmecc_sector_number;
	int		pmecc_degree;	/* Degree of remainders */
	int		pmecc_cw_len;	/* Length of codeword */

	/* lookup table for alpha_to and index_of */
	void __iomem	*pmecc_alpha_to;
	void __iomem	*pmecc_index_of;

	/* data for pmecc computation */
	int16_t	pmecc_smu[(CONFIG_PMECC_CAP + 2) * (2 * CONFIG_PMECC_CAP + 1)];
	int16_t	pmecc_partial_syn[2 * CONFIG_PMECC_CAP + 1];
	int16_t	pmecc_si[2 * CONFIG_PMECC_CAP + 1];
	int16_t	pmecc_lmu[CONFIG_PMECC_CAP + 1]; /* polynomal order */
	int	pmecc_mu[CONFIG_PMECC_CAP + 1];
	int	pmecc_dmu[CONFIG_PMECC_CAP + 1];
	int	pmecc_delta[CONFIG_PMECC_CAP + 1];
};

static struct atmel_nand_host pmecc_host;
static struct nand_ecclayout atmel_pmecc_oobinfo;

/*
 * Return number of ecc bytes per sector according to sector size and
 * correction capability
 *
 * Following table shows what at91 PMECC supported:
 * Correction Capability	Sector_512_bytes	Sector_1024_bytes
 * =====================	================	=================
 *                2-bits                 4-bytes                  4-bytes
 *                4-bits                 7-bytes                  7-bytes
 *                8-bits                13-bytes                 14-bytes
 *               12-bits                20-bytes                 21-bytes
 *               24-bits                39-bytes                 42-bytes
 */
static int pmecc_get_ecc_bytes(int cap, int sector_size)
{
	int m = 12 + sector_size / 512;
	return (m * cap + 7) / 8;
}

static void pmecc_config_ecc_layout(struct nand_ecclayout *layout,
	int oobsize, int ecc_len)
{
	int i;

	layout->eccbytes = ecc_len;

	/* ECC will occupy the last ecc_len bytes continuously */
	for (i = 0; i < ecc_len; i++)
		layout->eccpos[i] = oobsize - ecc_len + i;

	layout->oobfree[0].offset = 2;
	layout->oobfree[0].length =
		oobsize - ecc_len - layout->oobfree[0].offset;
}

static void __iomem *pmecc_get_alpha_to(struct atmel_nand_host *host)
{
	int table_size;

	table_size = host->pmecc_sector_size == 512 ?
		PMECC_INDEX_TABLE_SIZE_512 : PMECC_INDEX_TABLE_SIZE_1024;

	/* the ALPHA lookup table is right behind the INDEX lookup table. */
	return host->pmecc_rom_base + host->pmecc_index_table_offset +
			table_size * sizeof(int16_t);
}

static void pmecc_gen_syndrome(struct mtd_info *mtd, int sector)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int i;
	uint32_t value;

	/* Fill odd syndromes */
	for (i = 0; i < host->pmecc_corr_cap; i++) {
		value = readl(&host->pmecc->rem_port[sector].rem[i / 2]);
		if (i & 1)
			value >>= 16;
		value &= 0xffff;
		host->pmecc_partial_syn[(2 * i) + 1] = (int16_t)value;
	}
}

static void pmecc_substitute(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int16_t __iomem *alpha_to = host->pmecc_alpha_to;
	int16_t __iomem *index_of = host->pmecc_index_of;
	int16_t *partial_syn = host->pmecc_partial_syn;
	const int cap = host->pmecc_corr_cap;
	int16_t *si;
	int i, j;

	/* si[] is a table that holds the current syndrome value,
	 * an element of that table belongs to the field
	 */
	si = host->pmecc_si;

	memset(&si[1], 0, sizeof(int16_t) * (2 * cap - 1));

	/* Computation 2t syndromes based on S(x) */
	/* Odd syndromes */
	for (i = 1; i < 2 * cap; i += 2) {
		for (j = 0; j < host->pmecc_degree; j++) {
			if (partial_syn[i] & (0x1 << j))
				si[i] = readw(alpha_to + i * j) ^ si[i];
		}
	}
	/* Even syndrome = (Odd syndrome) ** 2 */
	for (i = 2, j = 1; j <= cap; i = ++j << 1) {
		if (si[j] == 0) {
			si[i] = 0;
		} else {
			int16_t tmp;

			tmp = readw(index_of + si[j]);
			tmp = (tmp * 2) % host->pmecc_cw_len;
			si[i] = readw(alpha_to + tmp);
		}
	}
}

/*
 * This function defines a Berlekamp iterative procedure for
 * finding the value of the error location polynomial.
 * The input is si[], initialize by pmecc_substitute().
 * The output is smu[][].
 *
 * This function is written according to chip datasheet Chapter:
 * Find the Error Location Polynomial Sigma(x) of Section:
 * Programmable Multibit ECC Control (PMECC).
 */
static void pmecc_get_sigma(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;

	int16_t *lmu = host->pmecc_lmu;
	int16_t *si = host->pmecc_si;
	int *mu = host->pmecc_mu;
	int *dmu = host->pmecc_dmu;	/* Discrepancy */
	int *delta = host->pmecc_delta; /* Delta order */
	int cw_len = host->pmecc_cw_len;
	const int16_t cap = host->pmecc_corr_cap;
	const int num = 2 * cap + 1;
	int16_t __iomem	*index_of = host->pmecc_index_of;
	int16_t __iomem	*alpha_to = host->pmecc_alpha_to;
	int i, j, k;
	uint32_t dmu_0_count, tmp;
	int16_t *smu = host->pmecc_smu;

	/* index of largest delta */
	int ro;
	int largest;
	int diff;

	/* Init the Sigma(x) */
	memset(smu, 0, sizeof(int16_t) * ARRAY_SIZE(smu));

	dmu_0_count = 0;

	/* First Row */

	/* Mu */
	mu[0] = -1;

	smu[0] = 1;

	/* discrepancy set to 1 */
	dmu[0] = 1;
	/* polynom order set to 0 */
	lmu[0] = 0;
	/* delta[0] = (mu[0] * 2 - lmu[0]) >> 1; */
	delta[0] = -1;

	/* Second Row */

	/* Mu */
	mu[1] = 0;
	/* Sigma(x) set to 1 */
	smu[num] = 1;

	/* discrepancy set to S1 */
	dmu[1] = si[1];

	/* polynom order set to 0 */
	lmu[1] = 0;

	/* delta[1] = (mu[1] * 2 - lmu[1]) >> 1; */
	delta[1] = 0;

	for (i = 1; i <= cap; i++) {
		mu[i + 1] = i << 1;
		/* Begin Computing Sigma (Mu+1) and L(mu) */
		/* check if discrepancy is set to 0 */
		if (dmu[i] == 0) {
			dmu_0_count++;

			tmp = ((cap - (lmu[i] >> 1) - 1) / 2);
			if ((cap - (lmu[i] >> 1) - 1) & 0x1)
				tmp += 2;
			else
				tmp += 1;

			if (dmu_0_count == tmp) {
				for (j = 0; j <= (lmu[i] >> 1) + 1; j++)
					smu[(cap + 1) * num + j] =
							smu[i * num + j];

				lmu[cap + 1] = lmu[i];
				return;
			}

			/* copy polynom */
			for (j = 0; j <= lmu[i] >> 1; j++)
				smu[(i + 1) * num + j] = smu[i * num + j];

			/* copy previous polynom order to the next */
			lmu[i + 1] = lmu[i];
		} else {
			ro = 0;
			largest = -1;
			/* find largest delta with dmu != 0 */
			for (j = 0; j < i; j++) {
				if ((dmu[j]) && (delta[j] > largest)) {
					largest = delta[j];
					ro = j;
				}
			}

			/* compute difference */
			diff = (mu[i] - mu[ro]);

			/* Compute degree of the new smu polynomial */
			if ((lmu[i] >> 1) > ((lmu[ro] >> 1) + diff))
				lmu[i + 1] = lmu[i];
			else
				lmu[i + 1] = ((lmu[ro] >> 1) + diff) * 2;

			/* Init smu[i+1] with 0 */
			for (k = 0; k < num; k++)
				smu[(i + 1) * num + k] = 0;

			/* Compute smu[i+1] */
			for (k = 0; k <= lmu[ro] >> 1; k++) {
				int16_t a, b, c;

				if (!(smu[ro * num + k] && dmu[i]))
					continue;
				a = readw(index_of + dmu[i]);
				b = readw(index_of + dmu[ro]);
				c = readw(index_of + smu[ro * num + k]);
				tmp = a + (cw_len - b) + c;
				a = readw(alpha_to + tmp % cw_len);
				smu[(i + 1) * num + (k + diff)] = a;
			}

			for (k = 0; k <= lmu[i] >> 1; k++)
				smu[(i + 1) * num + k] ^= smu[i * num + k];
		}

		/* End Computing Sigma (Mu+1) and L(mu) */
		/* In either case compute delta */
		delta[i + 1] = (mu[i + 1] * 2 - lmu[i + 1]) >> 1;

		/* Do not compute discrepancy for the last iteration */
		if (i >= cap)
			continue;

		for (k = 0; k <= (lmu[i + 1] >> 1); k++) {
			tmp = 2 * (i - 1);
			if (k == 0) {
				dmu[i + 1] = si[tmp + 3];
			} else if (smu[(i + 1) * num + k] && si[tmp + 3 - k]) {
				int16_t a, b, c;
				a = readw(index_of +
						smu[(i + 1) * num + k]);
				b = si[2 * (i - 1) + 3 - k];
				c = readw(index_of + b);
				tmp = a + c;
				tmp %= cw_len;
				dmu[i + 1] = readw(alpha_to + tmp) ^
					dmu[i + 1];
			}
		}
	}
}

static int pmecc_err_location(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	const int cap = host->pmecc_corr_cap;
	const int num = 2 * cap + 1;
	int sector_size = host->pmecc_sector_size;
	int err_nbr = 0;	/* number of error */
	int roots_nbr;		/* number of roots */
	int i;
	uint32_t val;
	int16_t *smu = host->pmecc_smu;
	int timeout = PMECC_MAX_TIMEOUT_US;

	writel(PMERRLOC_DISABLE, &host->pmerrloc->eldis);

	for (i = 0; i <= host->pmecc_lmu[cap + 1] >> 1; i++) {
		writel(smu[(cap + 1) * num + i], &host->pmerrloc->sigma[i]);
		err_nbr++;
	}

	val = PMERRLOC_ELCFG_NUM_ERRORS(err_nbr - 1);
	if (sector_size == 1024)
		val |= PMERRLOC_ELCFG_SECTOR_1024;

	writel(val, &host->pmerrloc->elcfg);
	writel(sector_size * 8 + host->pmecc_degree * cap,
			&host->pmerrloc->elen);

	while (--timeout) {
		if (readl(&host->pmerrloc->elisr) & PMERRLOC_CALC_DONE)
			break;
		WATCHDOG_RESET();
		udelay(1);
	}

	if (!timeout) {
		printk(KERN_ERR "atmel_nand : Timeout to calculate PMECC error location\n");
		return -1;
	}

	roots_nbr = (readl(&host->pmerrloc->elisr) & PMERRLOC_ERR_NUM_MASK)
			>> 8;
	/* Number of roots == degree of smu hence <= cap */
	if (roots_nbr == host->pmecc_lmu[cap + 1] >> 1)
		return err_nbr - 1;

	/* Number of roots does not match the degree of smu
	 * unable to correct error */
	return -1;
}

static void pmecc_correct_data(struct mtd_info *mtd, uint8_t *buf, uint8_t *ecc,
		int sector_num, int extra_bytes, int err_nbr)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int i = 0;
	int byte_pos, bit_pos, sector_size, pos;
	uint32_t tmp;
	uint8_t err_byte;

	sector_size = host->pmecc_sector_size;

	while (err_nbr) {
		tmp = readl(&host->pmerrloc->el[i]) - 1;
		byte_pos = tmp / 8;
		bit_pos  = tmp % 8;

		if (byte_pos >= (sector_size + extra_bytes))
			BUG();	/* should never happen */

		if (byte_pos < sector_size) {
			err_byte = *(buf + byte_pos);
			*(buf + byte_pos) ^= (1 << bit_pos);

			pos = sector_num * host->pmecc_sector_size + byte_pos;
			printk(KERN_INFO "Bit flip in data area, byte_pos: %d, bit_pos: %d, 0x%02x -> 0x%02x\n",
				pos, bit_pos, err_byte, *(buf + byte_pos));
		} else {
			/* Bit flip in OOB area */
			tmp = sector_num * host->pmecc_bytes_per_sector
					+ (byte_pos - sector_size);
			err_byte = ecc[tmp];
			ecc[tmp] ^= (1 << bit_pos);

			pos = tmp + nand_chip->ecc.layout->eccpos[0];
			printk(KERN_INFO "Bit flip in OOB, oob_byte_pos: %d, bit_pos: %d, 0x%02x -> 0x%02x\n",
				pos, bit_pos, err_byte, ecc[tmp]);
		}

		i++;
		err_nbr--;
	}

	return;
}

static int pmecc_correction(struct mtd_info *mtd, u32 pmecc_stat, uint8_t *buf,
	u8 *ecc)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int i, err_nbr, eccbytes;
	uint8_t *buf_pos;

	eccbytes = nand_chip->ecc.bytes;
	for (i = 0; i < eccbytes; i++)
		if (ecc[i] != 0xff)
			goto normal_check;
	/* Erased page, return OK */
	return 0;

normal_check:
	for (i = 0; i < host->pmecc_sector_number; i++) {
		err_nbr = 0;
		if (pmecc_stat & 0x1) {
			buf_pos = buf + i * host->pmecc_sector_size;

			pmecc_gen_syndrome(mtd, i);
			pmecc_substitute(mtd);
			pmecc_get_sigma(mtd);

			err_nbr = pmecc_err_location(mtd);
			if (err_nbr == -1) {
				printk(KERN_ERR "PMECC: Too many errors\n");
				mtd->ecc_stats.failed++;
				return -EIO;
			} else {
				pmecc_correct_data(mtd, buf_pos, ecc, i,
					host->pmecc_bytes_per_sector, err_nbr);
				mtd->ecc_stats.corrected += err_nbr;
			}
		}
		pmecc_stat >>= 1;
	}

	return 0;
}

static int atmel_nand_pmecc_read_page(struct mtd_info *mtd,
	struct nand_chip *chip, uint8_t *buf, int page)
{
	struct atmel_nand_host *host = chip->priv;
	int eccsize = chip->ecc.size;
	uint8_t *oob = chip->oob_poi;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint32_t stat;
	int timeout = PMECC_MAX_TIMEOUT_US;

	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_RST);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_DISABLE);
	pmecc_writel(host->pmecc, cfg, ((pmecc_readl(host->pmecc, cfg))
		& ~PMECC_CFG_WRITE_OP) | PMECC_CFG_AUTO_ENABLE);

	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_ENABLE);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_DATA);

	chip->read_buf(mtd, buf, eccsize);
	chip->read_buf(mtd, oob, mtd->oobsize);

	while (--timeout) {
		if (!(pmecc_readl(host->pmecc, sr) & PMECC_SR_BUSY))
			break;
		WATCHDOG_RESET();
		udelay(1);
	}

	if (!timeout) {
		printk(KERN_ERR "atmel_nand : Timeout to read PMECC page\n");
		return -1;
	}

	stat = pmecc_readl(host->pmecc, isr);
	if (stat != 0)
		if (pmecc_correction(mtd, stat, buf, &oob[eccpos[0]]) != 0)
			return -EIO;

	return 0;
}

static void atmel_nand_pmecc_write_page(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf)
{
	struct atmel_nand_host *host = chip->priv;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	int i, j;
	int timeout = PMECC_MAX_TIMEOUT_US;

	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_RST);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_DISABLE);

	pmecc_writel(host->pmecc, cfg, (pmecc_readl(host->pmecc, cfg) |
		PMECC_CFG_WRITE_OP) & ~PMECC_CFG_AUTO_ENABLE);

	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_ENABLE);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_DATA);

	chip->write_buf(mtd, (u8 *)buf, mtd->writesize);

	while (--timeout) {
		if (!(pmecc_readl(host->pmecc, sr) & PMECC_SR_BUSY))
			break;
		WATCHDOG_RESET();
		udelay(1);
	}

	if (!timeout) {
		printk(KERN_ERR "atmel_nand : Timeout to read PMECC status, fail to write PMECC in oob\n");
		return;
	}

	for (i = 0; i < host->pmecc_sector_number; i++) {
		for (j = 0; j < host->pmecc_bytes_per_sector; j++) {
			int pos;

			pos = i * host->pmecc_bytes_per_sector + j;
			chip->oob_poi[eccpos[pos]] =
				readb(&host->pmecc->ecc_port[i].ecc[j]);
		}
	}
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

static void atmel_pmecc_core_init(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	uint32_t val = 0;
	struct nand_ecclayout *ecc_layout;

	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_RST);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_DISABLE);

	switch (host->pmecc_corr_cap) {
	case 2:
		val = PMECC_CFG_BCH_ERR2;
		break;
	case 4:
		val = PMECC_CFG_BCH_ERR4;
		break;
	case 8:
		val = PMECC_CFG_BCH_ERR8;
		break;
	case 12:
		val = PMECC_CFG_BCH_ERR12;
		break;
	case 24:
		val = PMECC_CFG_BCH_ERR24;
		break;
	}

	if (host->pmecc_sector_size == 512)
		val |= PMECC_CFG_SECTOR512;
	else if (host->pmecc_sector_size == 1024)
		val |= PMECC_CFG_SECTOR1024;

	switch (host->pmecc_sector_number) {
	case 1:
		val |= PMECC_CFG_PAGE_1SECTOR;
		break;
	case 2:
		val |= PMECC_CFG_PAGE_2SECTORS;
		break;
	case 4:
		val |= PMECC_CFG_PAGE_4SECTORS;
		break;
	case 8:
		val |= PMECC_CFG_PAGE_8SECTORS;
		break;
	}

	val |= (PMECC_CFG_READ_OP | PMECC_CFG_SPARE_DISABLE
		| PMECC_CFG_AUTO_DISABLE);
	pmecc_writel(host->pmecc, cfg, val);

	ecc_layout = nand_chip->ecc.layout;
	pmecc_writel(host->pmecc, sarea, mtd->oobsize - 1);
	pmecc_writel(host->pmecc, saddr, ecc_layout->eccpos[0]);
	pmecc_writel(host->pmecc, eaddr,
			ecc_layout->eccpos[ecc_layout->eccbytes - 1]);
	/* See datasheet about PMECC Clock Control Register */
	pmecc_writel(host->pmecc, clk, PMECC_CLK_133MHZ);
	pmecc_writel(host->pmecc, idr, 0xff);
	pmecc_writel(host->pmecc, ctrl, PMECC_CTRL_ENABLE);
}

static int atmel_pmecc_nand_init_params(struct nand_chip *nand,
		struct mtd_info *mtd)
{
	struct atmel_nand_host *host;
	int cap, sector_size;

	host = nand->priv = &pmecc_host;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.calculate = NULL;
	nand->ecc.correct = NULL;
	nand->ecc.hwctl = NULL;

	cap = host->pmecc_corr_cap = CONFIG_PMECC_CAP;
	sector_size = host->pmecc_sector_size = CONFIG_PMECC_SECTOR_SIZE;
	host->pmecc_index_table_offset = CONFIG_PMECC_INDEX_TABLE_OFFSET;

	MTDDEBUG(MTD_DEBUG_LEVEL1,
		"Initialize PMECC params, cap: %d, sector: %d\n",
		cap, sector_size);

	host->pmecc = (struct pmecc_regs __iomem *) ATMEL_BASE_PMECC;
	host->pmerrloc = (struct pmecc_errloc_regs __iomem *)
			ATMEL_BASE_PMERRLOC;
	host->pmecc_rom_base = (void __iomem *) ATMEL_BASE_ROM;

	/* ECC is calculated for the whole page (1 step) */
	nand->ecc.size = mtd->writesize;

	/* set ECC page size and oob layout */
	switch (mtd->writesize) {
	case 2048:
	case 4096:
		host->pmecc_degree = PMECC_GF_DIMENSION_13;
		host->pmecc_cw_len = (1 << host->pmecc_degree) - 1;
		host->pmecc_sector_number = mtd->writesize / sector_size;
		host->pmecc_bytes_per_sector = pmecc_get_ecc_bytes(
			cap, sector_size);
		host->pmecc_alpha_to = pmecc_get_alpha_to(host);
		host->pmecc_index_of = host->pmecc_rom_base +
			host->pmecc_index_table_offset;

		nand->ecc.steps = 1;
		nand->ecc.bytes = host->pmecc_bytes_per_sector *
				       host->pmecc_sector_number;
		if (nand->ecc.bytes > mtd->oobsize - 2) {
			printk(KERN_ERR "No room for ECC bytes\n");
			return -EINVAL;
		}
		pmecc_config_ecc_layout(&atmel_pmecc_oobinfo,
					mtd->oobsize,
					nand->ecc.bytes);
		nand->ecc.layout = &atmel_pmecc_oobinfo;
		break;
	case 512:
	case 1024:
		/* TODO */
		printk(KERN_ERR "Unsupported page size for PMECC, use Software ECC\n");
	default:
		/* page size not handled by HW ECC */
		/* switching back to soft ECC */
		nand->ecc.mode = NAND_ECC_SOFT;
		nand->ecc.read_page = NULL;
		nand->ecc.postpad = 0;
		nand->ecc.prepad = 0;
		nand->ecc.bytes = 0;
		return 0;
	}

	nand->ecc.read_page = atmel_nand_pmecc_read_page;
	nand->ecc.write_page = atmel_nand_pmecc_write_page;

	atmel_pmecc_core_init(mtd);

	return 0;
}

#else

/* oob layout for large page size
 * bad block info is on bytes 0 and 1
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_large = {
	.eccbytes = 4,
	.eccpos = {60, 61, 62, 63},
	.oobfree = {
		{2, 58}
	},
};

/* oob layout for small page size
 * bad block info is on bytes 4 and 5
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_small = {
	.eccbytes = 4,
	.eccpos = {0, 1, 2, 3},
	.oobfree = {
		{6, 10}
	},
};

/*
 * Calculate HW ECC
 *
 * function called after a write
 *
 * mtd:        MTD block structure
 * dat:        raw data (unused)
 * ecc_code:   buffer for ECC
 */
static int atmel_nand_calculate(struct mtd_info *mtd,
		const u_char *dat, unsigned char *ecc_code)
{
	unsigned int ecc_value;

	/* get the first 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR);

	ecc_code[0] = ecc_value & 0xFF;
	ecc_code[1] = (ecc_value >> 8) & 0xFF;

	/* get the last 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, NPR) & ATMEL_ECC_NPARITY;

	ecc_code[2] = ecc_value & 0xFF;
	ecc_code[3] = (ecc_value >> 8) & 0xFF;

	return 0;
}

/*
 * HW ECC read page function
 *
 * mtd:        mtd info structure
 * chip:       nand chip info structure
 * buf:        buffer to store read data
 */
static int atmel_nand_read_page(struct mtd_info *mtd,
		struct nand_chip *chip, uint8_t *buf, int page)
{
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;
	uint8_t *ecc_pos;
	int stat;

	/* read the page */
	chip->read_buf(mtd, p, eccsize);

	/* move to ECC position if needed */
	if (eccpos[0] != 0) {
		/* This only works on large pages
		 * because the ECC controller waits for
		 * NAND_CMD_RNDOUTSTART after the
		 * NAND_CMD_RNDOUT.
		 * anyway, for small pages, the eccpos[0] == 0
		 */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT,
				mtd->writesize + eccpos[0], -1);
	}

	/* the ECC controller needs to read the ECC just after the data */
	ecc_pos = oob + eccpos[0];
	chip->read_buf(mtd, ecc_pos, eccbytes);

	/* check if there's an error */
	stat = chip->ecc.correct(mtd, p, oob, NULL);

	if (stat < 0)
		mtd->ecc_stats.failed++;
	else
		mtd->ecc_stats.corrected += stat;

	/* get back to oob start (end of page) */
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);

	/* read the oob */
	chip->read_buf(mtd, oob, mtd->oobsize);

	return 0;
}

/*
 * HW ECC Correction
 *
 * function called after a read
 *
 * mtd:        MTD block structure
 * dat:        raw data read from the chip
 * read_ecc:   ECC from the chip (unused)
 * isnull:     unused
 *
 * Detect and correct a 1 bit error for a page
 */
static int atmel_nand_correct(struct mtd_info *mtd, u_char *dat,
		u_char *read_ecc, u_char *isnull)
{
	struct nand_chip *nand_chip = mtd->priv;
	unsigned int ecc_status;
	unsigned int ecc_word, ecc_bit;

	/* get the status from the Status Register */
	ecc_status = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, SR);

	/* if there's no error */
	if (likely(!(ecc_status & ATMEL_ECC_RECERR)))
		return 0;

	/* get error bit offset (4 bits) */
	ecc_bit = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_BITADDR;
	/* get word address (12 bits) */
	ecc_word = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_WORDADDR;
	ecc_word >>= 4;

	/* if there are multiple errors */
	if (ecc_status & ATMEL_ECC_MULERR) {
		/* check if it is a freshly erased block
		 * (filled with 0xff) */
		if ((ecc_bit == ATMEL_ECC_BITADDR)
				&& (ecc_word == (ATMEL_ECC_WORDADDR >> 4))) {
			/* the block has just been erased, return OK */
			return 0;
		}
		/* it doesn't seems to be a freshly
		 * erased block.
		 * We can't correct so many errors */
		printk(KERN_WARNING "atmel_nand : multiple errors detected."
				" Unable to correct.\n");
		return -EIO;
	}

	/* if there's a single bit error : we can correct it */
	if (ecc_status & ATMEL_ECC_ECCERR) {
		/* there's nothing much to do here.
		 * the bit error is on the ECC itself.
		 */
		printk(KERN_WARNING "atmel_nand : one bit error on ECC code."
				" Nothing to correct\n");
		return 0;
	}

	printk(KERN_WARNING "atmel_nand : one bit error on data."
			" (word offset in the page :"
			" 0x%x bit offset : 0x%x)\n",
			ecc_word, ecc_bit);
	/* correct the error */
	if (nand_chip->options & NAND_BUSWIDTH_16) {
		/* 16 bits words */
		((unsigned short *) dat)[ecc_word] ^= (1 << ecc_bit);
	} else {
		/* 8 bits words */
		dat[ecc_word] ^= (1 << ecc_bit);
	}
	printk(KERN_WARNING "atmel_nand : error corrected\n");
	return 1;
}

/*
 * Enable HW ECC : unused on most chips
 */
static void atmel_nand_hwctl(struct mtd_info *mtd, int mode)
{
}

int atmel_hwecc_nand_init_param(struct nand_chip *nand, struct mtd_info *mtd)
{
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.calculate = atmel_nand_calculate;
	nand->ecc.correct = atmel_nand_correct;
	nand->ecc.hwctl = atmel_nand_hwctl;
	nand->ecc.read_page = atmel_nand_read_page;
	nand->ecc.bytes = 4;

	if (nand->ecc.mode == NAND_ECC_HW) {
		/* ECC is calculated for the whole page (1 step) */
		nand->ecc.size = mtd->writesize;

		/* set ECC page size and oob layout */
		switch (mtd->writesize) {
		case 512:
			nand->ecc.layout = &atmel_oobinfo_small;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR,
					ATMEL_ECC_PAGESIZE_528);
			break;
		case 1024:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR,
					ATMEL_ECC_PAGESIZE_1056);
			break;
		case 2048:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR,
					ATMEL_ECC_PAGESIZE_2112);
			break;
		case 4096:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR,
					ATMEL_ECC_PAGESIZE_4224);
			break;
		default:
			/* page size not handled by HW ECC */
			/* switching back to soft ECC */
			nand->ecc.mode = NAND_ECC_SOFT;
			nand->ecc.calculate = NULL;
			nand->ecc.correct = NULL;
			nand->ecc.hwctl = NULL;
			nand->ecc.read_page = NULL;
			nand->ecc.postpad = 0;
			nand->ecc.prepad = 0;
			nand->ecc.bytes = 0;
			break;
		}
	}

	return 0;
}

#endif /* CONFIG_ATMEL_NAND_HW_PMECC */

#endif /* CONFIG_ATMEL_NAND_HWECC */

static void at91_nand_hwcontrol(struct mtd_info *mtd,
					 int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
		IO_ADDR_W &= ~(CONFIG_SYS_NAND_MASK_ALE
			     | CONFIG_SYS_NAND_MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_ALE;

#ifdef CONFIG_SYS_NAND_ENABLE_PIN
		at91_set_gpio_value(CONFIG_SYS_NAND_ENABLE_PIN,
				    !(ctrl & NAND_NCE));
#endif
		this->IO_ADDR_W = (void *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SYS_NAND_READY_PIN
static int at91_nand_ready(struct mtd_info *mtd)
{
	return at91_get_gpio_value(CONFIG_SYS_NAND_READY_PIN);
}
#endif

#ifndef CONFIG_SYS_NAND_BASE_LIST
#define CONFIG_SYS_NAND_BASE_LIST { CONFIG_SYS_NAND_BASE }
#endif
static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];
static ulong base_addr[CONFIG_SYS_MAX_NAND_DEVICE] = CONFIG_SYS_NAND_BASE_LIST;

int atmel_nand_chip_init(int devnum, ulong base_addr)
{
	int ret;
	struct mtd_info *mtd = &nand_info[devnum];
	struct nand_chip *nand = &nand_chip[devnum];

	mtd->priv = nand;
	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)base_addr;

	nand->ecc.mode = NAND_ECC_SOFT;
#ifdef CONFIG_SYS_NAND_DBW_16
	nand->options = NAND_BUSWIDTH_16;
#endif
	nand->cmd_ctrl = at91_nand_hwcontrol;
#ifdef CONFIG_SYS_NAND_READY_PIN
	nand->dev_ready = at91_nand_ready;
#endif
	nand->chip_delay = 20;

	ret = nand_scan_ident(mtd, CONFIG_SYS_NAND_MAX_CHIPS, NULL);
	if (ret)
		return ret;

#ifdef CONFIG_ATMEL_NAND_HWECC
#ifdef CONFIG_ATMEL_NAND_HW_PMECC
	ret = atmel_pmecc_nand_init_params(nand, mtd);
#else
	ret = atmel_hwecc_nand_init_param(nand, mtd);
#endif
	if (ret)
		return ret;
#endif

	ret = nand_scan_tail(mtd);
	if (!ret)
		nand_register(devnum);

	return ret;
}

void board_nand_init(void)
{
	int i;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		if (atmel_nand_chip_init(i, base_addr[i]))
			printk(KERN_ERR "atmel_nand: Fail to initialize #%d chip",
				i);
}
