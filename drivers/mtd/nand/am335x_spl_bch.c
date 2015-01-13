/*
 * (C) Copyright 2012
 * Konstantin Kozhevnikov, Cogent Embedded
 *
 * based on nand_spl_simple code
 *
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <linux/mtd/nand_ecc.h>

static int nand_ecc_pos[] = CONFIG_SYS_NAND_ECCPOS;
nand_info_t nand_info[1];
static struct nand_chip nand_chip;

#define ECCSTEPS	(CONFIG_SYS_NAND_PAGE_SIZE / \
					CONFIG_SYS_NAND_ECCSIZE)
#define ECCTOTAL	(ECCSTEPS * CONFIG_SYS_NAND_ECCBYTES)


/*
 * NAND command for large page NAND devices (2k)
 */
static int nand_command(int block, int page, uint32_t offs,
	u8 cmd)
{
	struct nand_chip *this = nand_info[0].priv;
	int page_addr = page + block * CONFIG_SYS_NAND_PAGE_COUNT;
	void (*hwctrl)(struct mtd_info *mtd, int cmd,
			unsigned int ctrl) = this->cmd_ctrl;

	while (!this->dev_ready(&nand_info[0]))
		;

	/* Emulate NAND_CMD_READOOB */
	if (cmd == NAND_CMD_READOOB) {
		offs += CONFIG_SYS_NAND_PAGE_SIZE;
		cmd = NAND_CMD_READ0;
	}

	/* Begin command latch cycle */
	hwctrl(&nand_info[0], cmd, NAND_CTRL_CLE | NAND_CTRL_CHANGE);

	if (cmd == NAND_CMD_RESET) {
		hwctrl(&nand_info[0], NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		while (!this->dev_ready(&nand_info[0]))
			;
		return 0;
	}

	/* Shift the offset from byte addressing to word addressing. */
	if ((this->options & NAND_BUSWIDTH_16) && !nand_opcode_8bits(cmd))
		offs >>= 1;

	/* Set ALE and clear CLE to start address cycle */
	/* Column address */
	hwctrl(&nand_info[0], offs & 0xff,
		       NAND_CTRL_ALE | NAND_CTRL_CHANGE); /* A[7:0] */
	hwctrl(&nand_info[0], (offs >> 8) & 0xff, NAND_CTRL_ALE); /* A[11:9] */
	/* Row address */
	if (cmd != NAND_CMD_RNDOUT) {
		hwctrl(&nand_info[0], (page_addr & 0xff),
		       NAND_CTRL_ALE); /* A[19:12] */
		hwctrl(&nand_info[0], ((page_addr >> 8) & 0xff),
		       NAND_CTRL_ALE); /* A[27:20] */
#ifdef CONFIG_SYS_NAND_5_ADDR_CYCLE
		/* One more address cycle for devices > 128MiB */
		hwctrl(&nand_info[0], (page_addr >> 16) & 0x0f,
		       NAND_CTRL_ALE); /* A[31:28] */
#endif
	}

	hwctrl(&nand_info[0], NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	if (cmd == NAND_CMD_READ0) {
		/* Latch in address */
		hwctrl(&nand_info[0], NAND_CMD_READSTART,
			   NAND_CTRL_CLE | NAND_CTRL_CHANGE);
		hwctrl(&nand_info[0], NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

		/*
		 * Wait a while for the data to be ready
		 */
		while (!this->dev_ready(&nand_info[0]))
			;
	} else if (cmd == NAND_CMD_RNDOUT) {
		hwctrl(&nand_info[0], NAND_CMD_RNDOUTSTART, NAND_CTRL_CLE |
					NAND_CTRL_CHANGE);
		hwctrl(&nand_info[0], NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
	}

	return 0;
}

static int nand_is_bad_block(int block)
{
	struct nand_chip *this = nand_info[0].priv;

	nand_command(block, 0, CONFIG_SYS_NAND_BAD_BLOCK_POS,
		NAND_CMD_READOOB);

	/*
	 * Read one byte (or two if it's a 16 bit chip).
	 */
	if (this->options & NAND_BUSWIDTH_16) {
		if (readw(this->IO_ADDR_R) != 0xffff)
			return 1;
	} else {
		if (readb(this->IO_ADDR_R) != 0xff)
			return 1;
	}

	return 0;
}

static int nand_read_page(int block, int page, void *dst)
{
	struct nand_chip *this = nand_info[0].priv;
	u_char ecc_calc[ECCTOTAL];
	u_char ecc_code[ECCTOTAL];
	u_char oob_data[CONFIG_SYS_NAND_OOBSIZE];
	int i;
	int eccsize = CONFIG_SYS_NAND_ECCSIZE;
	int eccbytes = CONFIG_SYS_NAND_ECCBYTES;
	int eccsteps = ECCSTEPS;
	uint8_t *p = dst;
	uint32_t data_pos = 0;
	uint8_t *oob = &oob_data[0] + nand_ecc_pos[0];
	uint32_t oob_pos = eccsize * eccsteps + nand_ecc_pos[0];

	nand_command(block, page, 0, NAND_CMD_READ0);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		this->ecc.hwctl(&nand_info[0], NAND_ECC_READ);
		nand_command(block, page, data_pos, NAND_CMD_RNDOUT);

		this->read_buf(&nand_info[0], p, eccsize);

		nand_command(block, page, oob_pos, NAND_CMD_RNDOUT);

		this->read_buf(&nand_info[0], oob, eccbytes);
		this->ecc.calculate(&nand_info[0], p, &ecc_calc[i]);

		data_pos += eccsize;
		oob_pos += eccbytes;
		oob += eccbytes;
	}

	/* Pick the ECC bytes out of the oob data */
	for (i = 0; i < ECCTOTAL; i++)
		ecc_code[i] = oob_data[nand_ecc_pos[i]];

	eccsteps = ECCSTEPS;
	p = dst;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		/* No chance to do something with the possible error message
		 * from correct_data(). We just hope that all possible errors
		 * are corrected by this routine.
		 */
		this->ecc.correct(&nand_info[0], p, &ecc_code[i], &ecc_calc[i]);
	}

	return 0;
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	unsigned int block, lastblock;
	unsigned int page;

	/*
	 * offs has to be aligned to a page address!
	 */
	block = offs / CONFIG_SYS_NAND_BLOCK_SIZE;
	lastblock = (offs + size - 1) / CONFIG_SYS_NAND_BLOCK_SIZE;
	page = (offs % CONFIG_SYS_NAND_BLOCK_SIZE) / CONFIG_SYS_NAND_PAGE_SIZE;

	while (block <= lastblock) {
		if (!nand_is_bad_block(block)) {
			/*
			 * Skip bad blocks
			 */
			while (page < CONFIG_SYS_NAND_PAGE_COUNT) {
				nand_read_page(block, page, dst);
				dst += CONFIG_SYS_NAND_PAGE_SIZE;
				page++;
			}

			page = 0;
		} else {
			lastblock++;
		}

		block++;
	}

	return 0;
}

/* nand_init() - initialize data to make nand usable by SPL */
void nand_init(void)
{
	/*
	 * Init board specific nand support
	 */
	nand_info[0].priv = &nand_chip;
	nand_chip.IO_ADDR_R = nand_chip.IO_ADDR_W =
		(void  __iomem *)CONFIG_SYS_NAND_BASE;
	board_nand_init(&nand_chip);

	if (nand_chip.select_chip)
		nand_chip.select_chip(&nand_info[0], 0);

	/* NAND chip may require reset after power-on */
	nand_command(0, 0, 0, NAND_CMD_RESET);
}

/* Unselect after operation */
void nand_deselect(void)
{
	if (nand_chip.select_chip)
		nand_chip.select_chip(&nand_info[0], -1);
}
