/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/mem.h>
#include <asm/arch/cpu.h>
#include <asm/omap_gpmc.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/bch.h>
#include <linux/compiler.h>
#include <nand.h>
#ifdef CONFIG_AM33XX
#include <asm/arch/elm.h>
#endif

static uint8_t cs;
static __maybe_unused struct nand_ecclayout hw_nand_oob =
	GPMC_NAND_HW_ECC_LAYOUT;
static __maybe_unused struct nand_ecclayout hw_bch8_nand_oob =
	GPMC_NAND_HW_BCH8_ECC_LAYOUT;

/*
 * omap_nand_hwcontrol - Set the address pointers corretly for the
 *			following address/data/command operation
 */
static void omap_nand_hwcontrol(struct mtd_info *mtd, int32_t cmd,
				uint32_t ctrl)
{
	register struct nand_chip *this = mtd->priv;

	/*
	 * Point the IO_ADDR to DATA and ADDRESS registers instead
	 * of chip address
	 */
	switch (ctrl) {
	case NAND_CTRL_CHANGE | NAND_CTRL_CLE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;
		break;
	case NAND_CTRL_CHANGE | NAND_CTRL_ALE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_adr;
		break;
	case NAND_CTRL_CHANGE | NAND_NCE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
		break;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SPL_BUILD
/* Check wait pin as dev ready indicator */
int omap_spl_dev_ready(struct mtd_info *mtd)
{
	return gpmc_cfg->status & (1 << 8);
}
#endif

/*
 * omap_hwecc_init - Initialize the Hardware ECC for NAND flash in
 *                   GPMC controller
 * @mtd:        MTD device structure
 *
 */
static void __maybe_unused omap_hwecc_init(struct nand_chip *chip)
{
	/*
	 * Init ECC Control Register
	 * Clear all ECC | Enable Reg1
	 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
	writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL, &gpmc_cfg->ecc_size_config);
}

/*
 * gen_true_ecc - This function will generate true ECC value, which
 * can be used when correcting data read from NAND flash memory core
 *
 * @ecc_buf:	buffer to store ecc code
 *
 * @return:	re-formatted ECC value
 */
static uint32_t gen_true_ecc(uint8_t *ecc_buf)
{
	return ecc_buf[0] | (ecc_buf[1] << 16) | ((ecc_buf[2] & 0xF0) << 20) |
		((ecc_buf[2] & 0x0F) << 8);
}

/*
 * omap_correct_data - Compares the ecc read from nand spare area with ECC
 * registers values and corrects one bit error if it has occured
 * Further details can be had from OMAP TRM and the following selected links:
 * http://en.wikipedia.org/wiki/Hamming_code
 * http://www.cs.utexas.edu/users/plaxton/c/337/05f/slides/ErrorCorrection-4.pdf
 *
 * @mtd:		 MTD device structure
 * @dat:		 page data
 * @read_ecc:		 ecc read from nand flash
 * @calc_ecc:		 ecc read from ECC registers
 *
 * @return 0 if data is OK or corrected, else returns -1
 */
static int __maybe_unused omap_correct_data(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t orig_ecc, new_ecc, res, hm;
	uint16_t parity_bits, byte;
	uint8_t bit;

	/* Regenerate the orginal ECC */
	orig_ecc = gen_true_ecc(read_ecc);
	new_ecc = gen_true_ecc(calc_ecc);
	/* Get the XOR of real ecc */
	res = orig_ecc ^ new_ecc;
	if (res) {
		/* Get the hamming width */
		hm = hweight32(res);
		/* Single bit errors can be corrected! */
		if (hm == 12) {
			/* Correctable data! */
			parity_bits = res >> 16;
			bit = (parity_bits & 0x7);
			byte = (parity_bits >> 3) & 0x1FF;
			/* Flip the bit to correct */
			dat[byte] ^= (0x1 << bit);
		} else if (hm == 1) {
			printf("Error: Ecc is wrong\n");
			/* ECC itself is corrupted */
			return 2;
		} else {
			/*
			 * hm distance != parity pairs OR one, could mean 2 bit
			 * error OR potentially be on a blank page..
			 * orig_ecc: contains spare area data from nand flash.
			 * new_ecc: generated ecc while reading data area.
			 * Note: if the ecc = 0, all data bits from which it was
			 * generated are 0xFF.
			 * The 3 byte(24 bits) ecc is generated per 512byte
			 * chunk of a page. If orig_ecc(from spare area)
			 * is 0xFF && new_ecc(computed now from data area)=0x0,
			 * this means that data area is 0xFF and spare area is
			 * 0xFF. A sure sign of a erased page!
			 */
			if ((orig_ecc == 0x0FFF0FFF) && (new_ecc == 0x00000000))
				return 0;
			printf("Error: Bad compare! failed\n");
			/* detected 2 bit error */
			return -1;
		}
	}
	return 0;
}

/*
 *  omap_calculate_ecc - Generate non-inverted ECC bytes.
 *
 *  Using noninverted ECC can be considered ugly since writing a blank
 *  page ie. padding will clear the ECC bytes. This is no problem as
 *  long nobody is trying to write data on the seemingly unused page.
 *  Reading an erased page will produce an ECC mismatch between
 *  generated and read ECC bytes that has to be dealt with separately.
 *  E.g. if page is 0xFF (fresh erased), and if HW ECC engine within GPMC
 *  is used, the result of read will be 0x0 while the ECC offsets of the
 *  spare area will be 0xFF which will result in an ECC mismatch.
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
static int __maybe_unused omap_calculate_ecc(struct mtd_info *mtd,
		const uint8_t *dat, uint8_t *ecc_code)
{
	u_int32_t val;

	/* Start Reading from HW ECC1_Result = 0x200 */
	val = readl(&gpmc_cfg->ecc1_result);

	ecc_code[0] = val & 0xFF;
	ecc_code[1] = (val >> 16) & 0xFF;
	ecc_code[2] = ((val >> 8) & 0x0F) | ((val >> 20) & 0xF0);

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	writel(0x000, &gpmc_cfg->ecc_config);

	return 0;
}

/*
 * omap_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
static void __maybe_unused omap_enable_hwecc(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *chip = mtd->priv;
	uint32_t val, dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;

	switch (mode) {
	case NAND_ECC_READ:
	case NAND_ECC_WRITE:
		/* Clear the ecc result registers, select ecc reg as 1 */
		writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

		/*
		 * Size 0 = 0xFF, Size1 is 0xFF - both are 512 bytes
		 * tell all regs to generate size0 sized regs
		 * we just have a single ECC engine for all CS
		 */
		writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL,
			&gpmc_cfg->ecc_size_config);
		val = (dev_width << 7) | (cs << 1) | (0x1);
		writel(val, &gpmc_cfg->ecc_config);
		break;
	default:
		printf("Error: Unrecognized Mode[%d]!\n", mode);
		break;
	}
}

/*
 * Generic BCH interface
 */
struct nand_bch_priv {
	uint8_t mode;
	uint8_t type;
	uint8_t nibbles;
	struct bch_control *control;
};

/* bch types */
#define ECC_BCH4	0
#define ECC_BCH8	1
#define ECC_BCH16	2

/* GPMC ecc engine settings */
#define BCH_WRAPMODE_1		1	/* BCH wrap mode 1 */
#define BCH_WRAPMODE_6		6	/* BCH wrap mode 6 */

/* BCH nibbles for diff bch levels */
#define NAND_ECC_HW_BCH ((uint8_t)(NAND_ECC_HW_OOB_FIRST) + 1)
#define ECC_BCH4_NIBBLES	13
#define ECC_BCH8_NIBBLES	26
#define ECC_BCH16_NIBBLES	52

/*
 * This can be a single instance cause all current users have only one NAND
 * with nearly the same setup (BCH8, some with ELM and others with sw BCH
 * library).
 * When some users with other BCH strength will exists this have to change!
 */
static __maybe_unused struct nand_bch_priv bch_priv = {
	.mode = NAND_ECC_HW_BCH,
	.type = ECC_BCH8,
	.nibbles = ECC_BCH8_NIBBLES,
	.control = NULL
};

/*
 * omap_hwecc_init_bch - Initialize the BCH Hardware ECC for NAND flash in
 *				GPMC controller
 * @mtd:	MTD device structure
 * @mode:	Read/Write mode
 */
__maybe_unused
static void omap_hwecc_init_bch(struct nand_chip *chip, int32_t mode)
{
	uint32_t val;
	uint32_t dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;
#ifdef CONFIG_AM33XX
	uint32_t unused_length = 0;
#endif
	uint32_t wr_mode = BCH_WRAPMODE_6;
	struct nand_bch_priv *bch = chip->priv;

	/* Clear the ecc result registers, select ecc reg as 1 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

#ifdef CONFIG_AM33XX
	wr_mode = BCH_WRAPMODE_1;

	switch (bch->nibbles) {
	case ECC_BCH4_NIBBLES:
		unused_length = 3;
		break;
	case ECC_BCH8_NIBBLES:
		unused_length = 2;
		break;
	case ECC_BCH16_NIBBLES:
		unused_length = 0;
		break;
	}

	/*
	 * This is ecc_size_config for ELM mode.
	 * Here we are using different settings for read and write access and
	 * also depending on BCH strength.
	 */
	switch (mode) {
	case NAND_ECC_WRITE:
		/* write access only setup eccsize1 config */
		val = ((unused_length + bch->nibbles) << 22);
		break;

	case NAND_ECC_READ:
	default:
		/*
		 * by default eccsize0 selected for ecc1resultsize
		 * eccsize0 config.
		 */
		val  = (bch->nibbles << 12);
		/* eccsize1 config */
		val |= (unused_length << 22);
		break;
	}
#else
	/*
	 * This ecc_size_config setting is for BCH sw library.
	 *
	 * Note: we only support BCH8 currently with BCH sw library!
	 * Should be really easy to adobt to BCH4, however some omap3 have
	 * flaws with BCH4.
	 *
	 * Here we are using wrapping mode 6 both for reading and writing, with:
	 *  size0 = 0  (no additional protected byte in spare area)
	 *  size1 = 32 (skip 32 nibbles = 16 bytes per sector in spare area)
	 */
	val = (32 << 22) | (0 << 12);
#endif
	/* ecc size configuration */
	writel(val, &gpmc_cfg->ecc_size_config);

	/*
	 * Configure the ecc engine in gpmc
	 * We assume 512 Byte sector pages for access to NAND.
	 */
	val  = (1 << 16);		/* enable BCH mode */
	val |= (bch->type << 12);	/* setup BCH type */
	val |= (wr_mode << 8);		/* setup wrapping mode */
	val |= (dev_width << 7);	/* setup device width (16 or 8 bit) */
	val |= (cs << 1);		/* setup chip select to work on */
	debug("set ECC_CONFIG=0x%08x\n", val);
	writel(val, &gpmc_cfg->ecc_config);
}

/*
 * omap_enable_ecc_bch - This function enables the bch h/w ecc functionality
 * @mtd:	MTD device structure
 * @mode:	Read/Write mode
 */
__maybe_unused
static void omap_enable_ecc_bch(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *chip = mtd->priv;

	omap_hwecc_init_bch(chip, mode);
	/* enable ecc */
	writel((readl(&gpmc_cfg->ecc_config) | 0x1), &gpmc_cfg->ecc_config);
}

/*
 * omap_ecc_disable - Disable H/W ECC calculation
 *
 * @mtd:	MTD device structure
 */
static void __maybe_unused omap_ecc_disable(struct mtd_info *mtd)
{
	writel((readl(&gpmc_cfg->ecc_config) & ~0x1), &gpmc_cfg->ecc_config);
}

/*
 * BCH8 support (needs ELM and thus AM33xx-only)
 */
#ifdef CONFIG_AM33XX
/*
 * omap_read_bch8_result - Read BCH result for BCH8 level
 *
 * @mtd:	MTD device structure
 * @big_endian:	When set read register 3 first
 * @ecc_code:	Read syndrome from BCH result registers
 */
static void omap_read_bch8_result(struct mtd_info *mtd, uint8_t big_endian,
				uint8_t *ecc_code)
{
	uint32_t *ptr;
	int8_t i = 0, j;

	if (big_endian) {
		ptr = &gpmc_cfg->bch_result_0_3[0].bch_result_x[3];
		ecc_code[i++] = readl(ptr) & 0xFF;
		ptr--;
		for (j = 0; j < 3; j++) {
			ecc_code[i++] = (readl(ptr) >> 24) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 16) & 0xFF;
			ecc_code[i++] = (readl(ptr) >>  8) & 0xFF;
			ecc_code[i++] = readl(ptr) & 0xFF;
			ptr--;
		}
	} else {
		ptr = &gpmc_cfg->bch_result_0_3[0].bch_result_x[0];
		for (j = 0; j < 3; j++) {
			ecc_code[i++] = readl(ptr) & 0xFF;
			ecc_code[i++] = (readl(ptr) >>  8) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 16) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 24) & 0xFF;
			ptr++;
		}
		ecc_code[i++] = readl(ptr) & 0xFF;
		ecc_code[i++] = 0;	/* 14th byte is always zero */
	}
}

/*
 * omap_rotate_ecc_bch - Rotate the syndrome bytes
 *
 * @mtd:	MTD device structure
 * @calc_ecc:	ECC read from ECC registers
 * @syndrome:	Rotated syndrome will be retuned in this array
 *
 */
static void omap_rotate_ecc_bch(struct mtd_info *mtd, uint8_t *calc_ecc,
		uint8_t *syndrome)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t n_bytes = 0;
	int8_t i, j;

	switch (bch->type) {
	case ECC_BCH4:
		n_bytes = 8;
		break;

	case ECC_BCH16:
		n_bytes = 28;
		break;

	case ECC_BCH8:
	default:
		n_bytes = 13;
		break;
	}

	for (i = 0, j = (n_bytes-1); i < n_bytes; i++, j--)
		syndrome[i] =  calc_ecc[j];
}

/*
 *  omap_calculate_ecc_bch - Read BCH ECC result
 *
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
static int omap_calculate_ecc_bch(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc_code)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t big_endian = 1;
	int8_t ret = 0;

	if (bch->type == ECC_BCH8)
		omap_read_bch8_result(mtd, big_endian, ecc_code);
	else /* BCH4 and BCH16 currently not supported */
		ret = -1;

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	omap_ecc_disable(mtd);

	return ret;
}

/*
 * omap_fix_errors_bch - Correct bch error in the data
 *
 * @mtd:	MTD device structure
 * @data:	Data read from flash
 * @error_count:Number of errors in data
 * @error_loc:	Locations of errors in the data
 *
 */
static void omap_fix_errors_bch(struct mtd_info *mtd, uint8_t *data,
		uint32_t error_count, uint32_t *error_loc)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t count = 0;
	uint32_t error_byte_pos;
	uint32_t error_bit_mask;
	uint32_t last_bit = (bch->nibbles * 4) - 1;

	/* Flip all bits as specified by the error location array. */
	/* FOR( each found error location flip the bit ) */
	for (count = 0; count < error_count; count++) {
		if (error_loc[count] > last_bit) {
			/* Remove the ECC spare bits from correction. */
			error_loc[count] -= (last_bit + 1);
			/* Offset bit in data region */
			error_byte_pos = ((512 * 8) -
					(error_loc[count]) - 1) / 8;
			/* Error Bit mask */
			error_bit_mask = 0x1 << (error_loc[count] % 8);
			/* Toggle the error bit to make the correction. */
			data[error_byte_pos] ^= error_bit_mask;
		}
	}
}

/*
 * omap_correct_data_bch - Compares the ecc read from nand spare area
 * with ECC registers values and corrects one bit error if it has occured
 *
 * @mtd:	MTD device structure
 * @dat:	page data
 * @read_ecc:	ecc read from nand flash (ignored)
 * @calc_ecc:	ecc read from ECC registers
 *
 * @return 0 if data is OK or corrected, else returns -1
 */
static int omap_correct_data_bch(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t syndrome[28];
	uint32_t error_count = 0;
	uint32_t error_loc[8];
	uint32_t i, ecc_flag;

	ecc_flag = 0;
	for (i = 0; i < chip->ecc.bytes; i++)
		if (read_ecc[i] != 0xff)
			ecc_flag = 1;

	if (!ecc_flag)
		return 0;

	elm_reset();
	elm_config((enum bch_level)(bch->type));

	/*
	 * while reading ECC result we read it in big endian.
	 * Hence while loading to ELM we have rotate to get the right endian.
	 */
	omap_rotate_ecc_bch(mtd, calc_ecc, syndrome);

	/* use elm module to check for errors */
	if (elm_check_error(syndrome, bch->nibbles, &error_count,
				error_loc) != 0) {
		printf("ECC: uncorrectable.\n");
		return -1;
	}

	/* correct bch error */
	if (error_count > 0)
		omap_fix_errors_bch(mtd, dat, error_count, error_loc);

	return 0;
}

/**
 * omap_read_page_bch - hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @oob_required: caller expects OOB data read to chip->oob_poi
 * @page:	page number to read
 *
 */
static int omap_read_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *oob = chip->oob_poi;
	uint32_t data_pos;
	uint32_t oob_pos;

	data_pos = 0;
	/* oob area start */
	oob_pos = (eccsize * eccsteps) + chip->ecc.layout->eccpos[0];
	oob += chip->ecc.layout->eccpos[0];

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize,
				oob += eccbytes) {
		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		/* read data */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, data_pos, page);
		chip->read_buf(mtd, p, eccsize);

		/* read respective ecc from oob area */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, oob_pos, page);
		chip->read_buf(mtd, oob, eccbytes);
		/* read syndrome */
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		data_pos += eccsize;
		oob_pos += eccbytes;
	}

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}
#endif /* CONFIG_AM33XX */

/*
 * OMAP3 BCH8 support (with BCH library)
 */
#ifdef CONFIG_NAND_OMAP_BCH8
/*
 *  omap_calculate_ecc_bch - Read BCH ECC result
 *
 *  @mtd:	MTD device structure
 *  @dat:	The pointer to data on which ecc is computed (unused here)
 *  @ecc:	The ECC output buffer
 */
static int omap_calculate_ecc_bch(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc)
{
	int ret = 0;
	size_t i;
	unsigned long nsectors, val1, val2, val3, val4;

	nsectors = ((readl(&gpmc_cfg->ecc_config) >> 4) & 0x7) + 1;

	for (i = 0; i < nsectors; i++) {
		/* Read hw-computed remainder */
		val1 = readl(&gpmc_cfg->bch_result_0_3[i].bch_result_x[0]);
		val2 = readl(&gpmc_cfg->bch_result_0_3[i].bch_result_x[1]);
		val3 = readl(&gpmc_cfg->bch_result_0_3[i].bch_result_x[2]);
		val4 = readl(&gpmc_cfg->bch_result_0_3[i].bch_result_x[3]);

		/*
		 * Add constant polynomial to remainder, in order to get an ecc
		 * sequence of 0xFFs for a buffer filled with 0xFFs.
		 */
		*ecc++ = 0xef ^ (val4 & 0xFF);
		*ecc++ = 0x51 ^ ((val3 >> 24) & 0xFF);
		*ecc++ = 0x2e ^ ((val3 >> 16) & 0xFF);
		*ecc++ = 0x09 ^ ((val3 >> 8) & 0xFF);
		*ecc++ = 0xed ^ (val3 & 0xFF);
		*ecc++ = 0x93 ^ ((val2 >> 24) & 0xFF);
		*ecc++ = 0x9a ^ ((val2 >> 16) & 0xFF);
		*ecc++ = 0xc2 ^ ((val2 >> 8) & 0xFF);
		*ecc++ = 0x97 ^ (val2 & 0xFF);
		*ecc++ = 0x79 ^ ((val1 >> 24) & 0xFF);
		*ecc++ = 0xe5 ^ ((val1 >> 16) & 0xFF);
		*ecc++ = 0x24 ^ ((val1 >> 8) & 0xFF);
		*ecc++ = 0xb5 ^ (val1 & 0xFF);
	}

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	omap_ecc_disable(mtd);

	return ret;
}

/**
 * omap_correct_data_bch - Decode received data and correct errors
 * @mtd: MTD device structure
 * @data: page data
 * @read_ecc: ecc read from nand flash
 * @calc_ecc: ecc read from HW ECC registers
 */
static int omap_correct_data_bch(struct mtd_info *mtd, u_char *data,
				 u_char *read_ecc, u_char *calc_ecc)
{
	int i, count;
	/* cannot correct more than 8 errors */
	unsigned int errloc[8];
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *chip_priv = chip->priv;
	struct bch_control *bch = chip_priv->control;

	count = decode_bch(bch, NULL, 512, read_ecc, calc_ecc, NULL, errloc);
	if (count > 0) {
		/* correct errors */
		for (i = 0; i < count; i++) {
			/* correct data only, not ecc bytes */
			if (errloc[i] < 8*512)
				data[errloc[i]/8] ^= 1 << (errloc[i] & 7);
			printf("corrected bitflip %u\n", errloc[i]);
#ifdef DEBUG
			puts("read_ecc: ");
			/*
			 * BCH8 have 13 bytes of ECC; BCH4 needs adoption
			 * here!
			 */
			for (i = 0; i < 13; i++)
				printf("%02x ", read_ecc[i]);
			puts("\n");
			puts("calc_ecc: ");
			for (i = 0; i < 13; i++)
				printf("%02x ", calc_ecc[i]);
			puts("\n");
#endif
		}
	} else if (count < 0) {
		puts("ecc unrecoverable error\n");
	}
	return count;
}

/**
 * omap_free_bch - Release BCH ecc resources
 * @mtd: MTD device structure
 */
static void __maybe_unused omap_free_bch(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *chip_priv = chip->priv;
	struct bch_control *bch = NULL;

	if (chip_priv)
		bch = chip_priv->control;

	if (bch) {
		free_bch(bch);
		chip_priv->control = NULL;
	}
}
#endif /* CONFIG_NAND_OMAP_BCH8 */

#ifndef CONFIG_SPL_BUILD
/*
 * omap_nand_switch_ecc - switch the ECC operation between different engines
 * (h/w and s/w) and different algorithms (hamming and BCHx)
 *
 * @hardware		- true if one of the HW engines should be used
 * @eccstrength		- the number of bits that could be corrected
 *			  (1 - hamming, 4 - BCH4, 8 - BCH8, 16 - BCH16)
 */
void omap_nand_switch_ecc(uint32_t hardware, uint32_t eccstrength)
{
	struct nand_chip *nand;
	struct mtd_info *mtd;

	if (nand_curr_device < 0 ||
	    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		printf("Error: Can't switch ecc, no devices available\n");
		return;
	}

	mtd = &nand_info[nand_curr_device];
	nand = mtd->priv;

	nand->options |= NAND_OWN_BUFFERS;

	/* Reset ecc interface */
	nand->ecc.mode = NAND_ECC_NONE;
	nand->ecc.read_page = NULL;
	nand->ecc.write_page = NULL;
	nand->ecc.read_oob = NULL;
	nand->ecc.write_oob = NULL;
	nand->ecc.hwctl = NULL;
	nand->ecc.correct = NULL;
	nand->ecc.calculate = NULL;
	nand->ecc.strength = eccstrength;

	/* Setup the ecc configurations again */
	if (hardware) {
		if (eccstrength == 1) {
			nand->ecc.mode = NAND_ECC_HW;
			nand->ecc.layout = &hw_nand_oob;
			nand->ecc.size = 512;
			nand->ecc.bytes = 3;
			nand->ecc.hwctl = omap_enable_hwecc;
			nand->ecc.correct = omap_correct_data;
			nand->ecc.calculate = omap_calculate_ecc;
			omap_hwecc_init(nand);
			printf("1-bit hamming HW ECC selected\n");
		}
#if defined(CONFIG_AM33XX) || defined(CONFIG_NAND_OMAP_BCH8)
		else if (eccstrength == 8) {
			nand->ecc.mode = NAND_ECC_HW;
			nand->ecc.layout = &hw_bch8_nand_oob;
			nand->ecc.size = 512;
#ifdef CONFIG_AM33XX
			nand->ecc.bytes = 14;
			nand->ecc.read_page = omap_read_page_bch;
#else
			nand->ecc.bytes = 13;
#endif
			nand->ecc.hwctl = omap_enable_ecc_bch;
			nand->ecc.correct = omap_correct_data_bch;
			nand->ecc.calculate = omap_calculate_ecc_bch;
			omap_hwecc_init_bch(nand, NAND_ECC_READ);
			printf("8-bit BCH HW ECC selected\n");
		}
#endif
	} else {
		nand->ecc.mode = NAND_ECC_SOFT;
		/* Use mtd default settings */
		nand->ecc.layout = NULL;
		nand->ecc.size = 0;
		printf("SW ECC selected\n");
	}

	/* Update NAND handling after ECC mode switch */
	nand_scan_tail(mtd);

	nand->options &= ~NAND_OWN_BUFFERS;
}
#endif /* CONFIG_SPL_BUILD */

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific:
 * - IO_ADDR_R: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W: address to write the 8 I/O lines of the flash device
 * - cmd_ctrl: hardwarespecific function for accesing control-lines
 * - waitfunc: hardwarespecific function for accesing device ready/busy line
 * - ecc.hwctl: function to enable (reset) hardware ecc generator
 * - ecc.mode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 */
int board_nand_init(struct nand_chip *nand)
{
	int32_t gpmc_config = 0;
	cs = 0;

	/*
	 * xloader/Uboot's gpmc configuration would have configured GPMC for
	 * nand type of memory. The following logic scans and latches on to the
	 * first CS with NAND type memory.
	 * TBD: need to make this logic generic to handle multiple CS NAND
	 * devices.
	 */
	while (cs < GPMC_MAX_CS) {
		/* Check if NAND type is set */
		if ((readl(&gpmc_cfg->cs[cs].config1) & 0xC00) == 0x800) {
			/* Found it!! */
			break;
		}
		cs++;
	}
	if (cs >= GPMC_MAX_CS) {
		printf("NAND: Unable to find NAND settings in "
			"GPMC Configuration - quitting\n");
		return -ENODEV;
	}

	gpmc_config = readl(&gpmc_cfg->config);
	/* Disable Write protect */
	gpmc_config |= 0x10;
	writel(gpmc_config, &gpmc_cfg->config);

	nand->IO_ADDR_R = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
	nand->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;

	nand->cmd_ctrl = omap_nand_hwcontrol;
	nand->options = NAND_NO_PADDING | NAND_CACHEPRG;
	/* If we are 16 bit dev, our gpmc config tells us that */
	if ((readl(&gpmc_cfg->cs[cs].config1) & 0x3000) == 0x1000)
		nand->options |= NAND_BUSWIDTH_16;

	nand->chip_delay = 100;

#if defined(CONFIG_AM33XX) || defined(CONFIG_NAND_OMAP_BCH8)
#ifdef CONFIG_AM33XX
	/* AM33xx uses the ELM */
	/* required in case of BCH */
	elm_init();
#else
	/*
	 * Whereas other OMAP based SoC do not have the ELM, they use the BCH
	 * SW library.
	 */
	bch_priv.control = init_bch(13, 8, 0x201b /* hw polynominal */);
	if (!bch_priv.control) {
		puts("Could not init_bch()\n");
		return -ENODEV;
	}
#endif
	/* BCH info that will be correct for SPL or overridden otherwise. */
	nand->priv = &bch_priv;
#endif

	/* Default ECC mode */
#if defined(CONFIG_AM33XX) || defined(CONFIG_NAND_OMAP_BCH8)
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.layout = &hw_bch8_nand_oob;
	nand->ecc.size = CONFIG_SYS_NAND_ECCSIZE;
	nand->ecc.bytes = CONFIG_SYS_NAND_ECCBYTES;
	nand->ecc.strength = 8;
	nand->ecc.hwctl = omap_enable_ecc_bch;
	nand->ecc.correct = omap_correct_data_bch;
	nand->ecc.calculate = omap_calculate_ecc_bch;
#ifdef CONFIG_AM33XX
	nand->ecc.read_page = omap_read_page_bch;
#endif
	omap_hwecc_init_bch(nand, NAND_ECC_READ);
#else
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_NAND_SOFTECC)
	nand->ecc.mode = NAND_ECC_SOFT;
#else
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.layout = &hw_nand_oob;
	nand->ecc.size = CONFIG_SYS_NAND_ECCSIZE;
	nand->ecc.bytes = CONFIG_SYS_NAND_ECCBYTES;
	nand->ecc.hwctl = omap_enable_hwecc;
	nand->ecc.correct = omap_correct_data;
	nand->ecc.calculate = omap_calculate_ecc;
	nand->ecc.strength = 1;
	omap_hwecc_init(nand);
#endif
#endif

#ifdef CONFIG_SPL_BUILD
	if (nand->options & NAND_BUSWIDTH_16)
		nand->read_buf = nand_read_buf16;
	else
		nand->read_buf = nand_read_buf;
	nand->dev_ready = omap_spl_dev_ready;
#endif

	return 0;
}
