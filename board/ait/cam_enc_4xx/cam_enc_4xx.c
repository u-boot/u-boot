/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/davinci_misc.h>
#ifdef CONFIG_DAVINCI_MMC
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SPL_BUILD
static struct davinci_timer *timer =
	(struct davinci_timer *)DAVINCI_TIMER3_BASE;

static unsigned long get_timer_val(void)
{
	unsigned long now = readl(&timer->tim34);

	return now;
}

static void stop_timer(void)
{
	writel(0x0, &timer->tcr);
	return;
}

int checkboard(void)
{
	printf("Board: AIT CAM ENC 4XX\n");
	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC
int board_eth_init(bd_t *bis)
{
	davinci_emac_initialize();

	return 0;
}
#endif

#ifdef CONFIG_NAND_DAVINCI
static int
davinci_std_read_page_syndrome(struct mtd_info *mtd, struct nand_chip *chip,
				   uint8_t *buf, int page)
{
	struct nand_chip *this = mtd->priv;
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0x0, page & this->pagemask);

	chip->read_buf(mtd, oob, mtd->oobsize);

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x0, page & this->pagemask);


	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		chip->ecc.hwctl(mtd, NAND_ECC_READSYN);

		if (chip->ecc.prepad)
			oob += chip->ecc.prepad;

		stat = chip->ecc.correct(mtd, p, oob, NULL);

		if (stat == -1)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;

		oob += eccbytes;

		if (chip->ecc.postpad)
			oob += chip->ecc.postpad;
	}

	/* Calculate remaining oob bytes */
	i = mtd->oobsize - (oob - chip->oob_poi);
	if (i)
		chip->read_buf(mtd, oob, i);

	return 0;
}

static void davinci_std_write_page_syndrome(struct mtd_info *mtd,
				    struct nand_chip *chip, const uint8_t *buf)
{
	unsigned char davinci_ecc_buf[NAND_MAX_OOBSIZE];
	struct nand_chip *this = mtd->priv;
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int chunk = chip->ecc.bytes + chip->ecc.prepad + chip->ecc.postpad;
	int offset = 0;
	const uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);

		/* Calculate ECC without prepad */
		chip->ecc.calculate(mtd, p, oob + chip->ecc.prepad);

		if (chip->ecc.prepad) {
			offset = (chip->ecc.steps - eccsteps) * chunk;
			memcpy(&davinci_ecc_buf[offset], oob, chip->ecc.prepad);
			oob += chip->ecc.prepad;
		}

		offset = ((chip->ecc.steps - eccsteps) * chunk) +
				chip->ecc.prepad;
		memcpy(&davinci_ecc_buf[offset], oob, eccbytes);
		oob += eccbytes;

		if (chip->ecc.postpad) {
			offset = ((chip->ecc.steps - eccsteps) * chunk) +
					chip->ecc.prepad + eccbytes;
			memcpy(&davinci_ecc_buf[offset], oob,
				chip->ecc.postpad);
			oob += chip->ecc.postpad;
		}
	}

	/*
	 * Write the sparebytes into the page once
	 * all eccsteps have been covered
	 */
	for (i = 0; i < mtd->oobsize; i++)
		writeb(davinci_ecc_buf[i], this->IO_ADDR_W);

	/* Calculate remaining oob bytes */
	i = mtd->oobsize - (oob - chip->oob_poi);
	if (i)
		chip->write_buf(mtd, oob, i);
}

static int davinci_std_write_oob_syndrome(struct mtd_info *mtd,
				   struct nand_chip *chip, int page)
{
	int pos, status = 0;
	const uint8_t *bufpoi = chip->oob_poi;

	pos = mtd->writesize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, pos, page);

	chip->write_buf(mtd, bufpoi, mtd->oobsize);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -1 : 0;
}

static int davinci_std_read_oob_syndrome(struct mtd_info *mtd,
	struct nand_chip *chip, int page, int sndcmd)
{
	struct nand_chip *this = mtd->priv;
	uint8_t *buf = chip->oob_poi;
	uint8_t *bufpoi = buf;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0x0, page & this->pagemask);

	chip->read_buf(mtd, bufpoi, mtd->oobsize);

	return 1;
}

static void nand_dm365evm_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip	*this = mtd->priv;
	unsigned long		wbase = (unsigned long) this->IO_ADDR_W;
	unsigned long		rbase = (unsigned long) this->IO_ADDR_R;

	if (chip == 1) {
		__set_bit(14, &wbase);
		__set_bit(14, &rbase);
	} else {
		__clear_bit(14, &wbase);
		__clear_bit(14, &rbase);
	}
	this->IO_ADDR_W = (void *)wbase;
	this->IO_ADDR_R = (void *)rbase;
}

int board_nand_init(struct nand_chip *nand)
{
	davinci_nand_init(nand);
	nand->select_chip = nand_dm365evm_select_chip;

	return 0;
}

struct nand_ecc_ctrl org_ecc;
static int notsaved = 1;

static int nand_switch_hw_func(int mode)
{
	struct nand_chip *nand;
	struct mtd_info *mtd;

	if (nand_curr_device < 0 ||
	    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		printf("Error: Can't switch hw functions," \
			" no devices available\n");
		return -1;
	}

	mtd = &nand_info[nand_curr_device];
	nand = mtd->priv;

	if (mode == 0) {
		printf("switching to uboot hw functions.\n");
		memcpy(&nand->ecc, &org_ecc, sizeof(struct nand_ecc_ctrl));
	} else {
		/* RBL */
		printf("switching to RBL hw functions.\n");
		if (notsaved == 1) {
			memcpy(&org_ecc, &nand->ecc,
				sizeof(struct nand_ecc_ctrl));
			notsaved = 0;
		}
		nand->ecc.mode = NAND_ECC_HW_SYNDROME;
		nand->ecc.prepad = 6;
		nand->ecc.read_page = davinci_std_read_page_syndrome;
		nand->ecc.write_page = davinci_std_write_page_syndrome;
		nand->ecc.read_oob = davinci_std_read_oob_syndrome;
		nand->ecc.write_oob = davinci_std_write_oob_syndrome;
	}
	return mode;
}

static int hwmode;

static int do_switch_ecc(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	if (argc != 2)
		goto usage;
	if (strncmp(argv[1], "rbl", 2) == 0)
		hwmode = nand_switch_hw_func(1);
	else if (strncmp(argv[1], "uboot", 2) == 0)
		hwmode = nand_switch_hw_func(0);
	else
		goto usage;

	return 0;

usage:
	printf("Usage: nandrbl %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandrbl, 2, 1,	do_switch_ecc,
	"switch between rbl/uboot NAND ECC calculation algorithm",
	"[rbl/uboot] - Switch between rbl/uboot NAND ECC algorithm"
);


#endif /* #ifdef CONFIG_NAND_DAVINCI */

#ifdef CONFIG_DAVINCI_MMC
static struct davinci_mmc mmc_sd0 = {
	.reg_base	= (struct davinci_mmc_regs *)DAVINCI_MMC_SD0_BASE,
	.input_clk	= 121500000,
	.host_caps	= MMC_MODE_4BIT,
	.voltages	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.version	= MMC_CTLR_VERSION_2,
};

int board_mmc_init(bd_t *bis)
{
	int err;

	/* Add slot-0 to mmc subsystem */
	err = davinci_mmc_init(bis, &mmc_sd0);

	return err;
}
#endif

int board_late_init(void)
{
	struct davinci_gpio *gpio = davinci_gpio_bank45;

	/* 24MHz InputClock / 15 prediv -> 1.6 MHz timer running */
	while (get_timer_val() < 0x186a00)
		;

	/* 1 sec reached -> stop timer, clear all LED */
	stop_timer();
	clrbits_le32(&gpio->out_data, CONFIG_CAM_ENC_LED_MASK);
	return 0;
}

void reset_phy(void)
{
	char *name = "GENERIC @ 0x00";

	/* reset the phy */
	miiphy_reset(name, 0x0);
}

#else /* #ifndef CONFIG_SPL_BUILD */
static void cam_enc_4xx_set_all_led(void)
{
	struct davinci_gpio *gpio = davinci_gpio_bank45;

	setbits_le32(&gpio->out_data, CONFIG_CAM_ENC_LED_MASK);
}

/*
 * TIMER 0 is used for tick
 */
static struct davinci_timer *timer =
	(struct davinci_timer *)DAVINCI_TIMER3_BASE;

#define TIMER_LOAD_VAL	0xffffffff
#define TIM_CLK_DIV	16

static int cam_enc_4xx_timer_init(void)
{
	/* We are using timer34 in unchained 32-bit mode, full speed */
	writel(0x0, &timer->tcr);
	writel(0x0, &timer->tgcr);
	writel(0x06 | ((TIM_CLK_DIV - 1) << 8), &timer->tgcr);
	writel(0x0, &timer->tim34);
	writel(TIMER_LOAD_VAL, &timer->prd34);
	writel(2 << 22, &timer->tcr);
	return 0;
}

void board_gpio_init(void)
{
	struct davinci_gpio *gpio;

	cam_enc_4xx_set_all_led();
	cam_enc_4xx_timer_init();
	gpio = davinci_gpio_bank01;
	clrbits_le32(&gpio->dir, ~0xfdfffffe);
	/* clear LED D14 = GPIO25 */
	clrbits_le32(&gpio->out_data, 0x02000000);
	gpio = davinci_gpio_bank23;
	clrbits_le32(&gpio->dir, ~0x5ff0afef);
	/* set GPIO61 to 1 -> intern UART0 as Console */
	setbits_le32(&gpio->out_data, 0x20000000);
	/*
	 * PHY out of reset GIO 50 = 1
	 * NAND WP off GIO 51 = 1
	 */
	setbits_le32(&gpio->out_data, 0x000c0004);
	gpio = davinci_gpio_bank45;
	clrbits_le32(&gpio->dir, ~(0xdb2fffff) | CONFIG_CAM_ENC_LED_MASK);
	/*
	 * clear LED:
	 * D17 = GPIO86
	 * D11 = GPIO87
	 * GPIO88
	 * GPIO89
	 * D13 = GPIO90
	 * GPIO91
	 */
	clrbits_le32(&gpio->out_data, CONFIG_CAM_ENC_LED_MASK);
	gpio = davinci_gpio_bank67;
	clrbits_le32(&gpio->dir, ~0x000007ff);
}

/*
 * functions for the post memory test.
 */
int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	*vstart = CONFIG_SYS_SDRAM_BASE;
	*size = PHYS_SDRAM_1_SIZE;
	*phys_offset = 0;
	return 0;
}

void arch_memory_failure_handle(void)
{
	cam_enc_4xx_set_all_led();
	puts("mem failure\n");
	while (1)
		;
}
#endif
