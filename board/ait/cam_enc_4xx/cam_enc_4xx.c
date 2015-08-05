/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cli.h>
#include <errno.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/davinci_nand.h>
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

static int timer_running(void)
{
	return readl(&timer->tcr) &
		(DV_TIMER_TCR_ENAMODE_MASK << DV_TIMER_TCR_ENAMODE34_SHIFT);
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
static int cam_enc_4xx_check_network(void)
{
	char *s;

	s = getenv("ethaddr");
	if (!s)
		return -EINVAL;

	if (!is_valid_ethaddr((const u8 *)s))
		return -EINVAL;

	s = getenv("ipaddr");
	if (!s)
		return -EINVAL;

	s = getenv("netmask");
	if (!s)
		return -EINVAL;

	s = getenv("serverip");
	if (!s)
		return -EINVAL;

	s = getenv("gatewayip");
	if (!s)
		return -EINVAL;

	return 0;
}
int board_eth_init(bd_t *bis)
{
	int ret;

	ret = cam_enc_4xx_check_network();
	if (ret)
		return ret;

	davinci_emac_initialize();

	return 0;
}
#endif

#ifdef CONFIG_NAND_DAVINCI
static int
davinci_std_read_page_syndrome(struct mtd_info *mtd, struct nand_chip *chip,
				   uint8_t *buf, int oob_required, int page)
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

static int davinci_std_write_page_syndrome(struct mtd_info *mtd,
				    struct nand_chip *chip, const uint8_t *buf,
				    int oob_required)
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
	return 0;
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
	struct nand_chip *chip, int page)
{
	struct nand_chip *this = mtd->priv;
	uint8_t *buf = chip->oob_poi;
	uint8_t *bufpoi = buf;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0x0, page & this->pagemask);

	chip->read_buf(mtd, bufpoi, mtd->oobsize);

	return 0;
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
		if (notsaved == 0) {
			printf("switching to uboot hw functions.\n");
			memcpy(&nand->ecc, &org_ecc,
				sizeof(struct nand_ecc_ctrl));
		}
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
	while ((get_timer_val() < CONFIG_AIT_TIMER_TIMEOUT) &&
		timer_running())
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
#if defined(CONFIG_MENU)
#include "menu.h"

#define MENU_EXIT		-1
#define MENU_EXIT_BOOTCMD	-2
#define MENU_STAY		0
#define MENU_MAIN		1
#define MENU_UPDATE		2
#define MENU_NETWORK		3
#define MENU_LOAD		4

static int menu_start;

#define FIT_SUBTYPE_UNKNOWN		0
#define FIT_SUBTYPE_UBL_HEADER		1
#define FIT_SUBTYPE_SPL_IMAGE		2
#define FIT_SUBTYPE_UBOOT_IMAGE		3
#define FIT_SUBTYPE_DF_ENV_IMAGE	4
#define FIT_SUBTYPE_RAMDISK_IMAGE	5

struct fit_images_info {
	u_int8_t type;
	int subtype;
	char desc[200];
	const void *data;
	size_t size;
};

static struct fit_images_info imgs[10];

struct menu_display {
	char	title[50];
	int	timeout; /* in sec */
	int	id; /* MENU_* */
	char	**menulist;
	int (*menu_evaluate)(char *choice);
};

char *menu_main[] = {
	"(1) Boot",
	"(2) Update Software",
	"(3) Reset to default setting and boot",
	"(4) Enter U-Boot console",
	NULL
};

char *menu_update[] = {
	"(1) Network settings",
	"(2) load image",
	"(3) back to main",
	NULL
};

char *menu_load[] = {
	"(1) install image",
	"(2) cancel",
	NULL
};

char *menu_network[] = {
	"(1) ipaddr   ",
	"(2) netmask  ",
	"(3) serverip ",
	"(4) gatewayip",
	"(5) tftp image name",
	"(6) back to update software",
	NULL
};

static void ait_menu_print(void *data)
{
	printf("%s\n", (char *)data);
	return;
}

static char *menu_handle(struct menu_display *display)
{
	struct menu *m;
	int i;
	void *choice = NULL;
	char key[2];
	int ret;
	char *s;
	char temp[6][200];

	m = menu_create(display->title, display->timeout, 1, ait_menu_print,
			NULL, NULL);

	for (i = 0; display->menulist[i]; i++) {
		sprintf(key, "%d", i + 1);
		if (display->id == MENU_NETWORK) {
			switch (i) {
			case 0:
				s = getenv("ipaddr");
				break;
			case 1:
				s = getenv("netmask");
				break;
			case 2:
				s = getenv("serverip");
				break;
			case 3:
				s = getenv("gatewayip");
				break;
			case 4:
				s = getenv("img_file");
				break;
			default:
				s = NULL;
				break;
			}
			if (s) {
				sprintf(temp[i], "%s: %s",
					display->menulist[i], s);
				ret = menu_item_add(m, key, temp[i]);
			} else {
				ret = menu_item_add(m, key,
					display->menulist[i]);
			}
		} else {
			ret = menu_item_add(m, key, display->menulist[i]);
		}

		if (ret != 1) {
			printf("failed to add item!");
			menu_destroy(m);
			return NULL;
		}
	}
	sprintf(key, "%d", 1);
	menu_default_set(m, key);

	if (menu_get_choice(m, &choice) != 1)
		debug("Problem picking a choice!\n");

	menu_destroy(m);

	return choice;
}

static int ait_menu_show(struct menu_display *display, int bootdelay)
{
	int end = MENU_STAY;
	char *choice;

	if ((menu_start == 0) && (display->id == MENU_MAIN))
		display->timeout = bootdelay;
	else
		display->timeout = 0;

	while (end == MENU_STAY) {
		choice = menu_handle(display);
		if (choice)
			end = display->menu_evaluate(choice);

		if (end == display->id)
			end = MENU_STAY;
		if (display->id == MENU_MAIN) {
			if (menu_start == 0)
				end = MENU_EXIT_BOOTCMD;
			else
				display->timeout = 0;
		}
	}
	return end;
}

static int ait_writeublheader(void)
{
	char s[20];
	unsigned long i;
	int ret;

	for (i = CONFIG_SYS_NAND_BLOCK_SIZE;
		i < CONFIG_SYS_NAND_U_BOOT_OFFS;
		i += CONFIG_SYS_NAND_BLOCK_SIZE) {
		sprintf(s, "%lx", i);
		ret = setenv("header_addr", s);
		if (ret == 0)
			ret = run_command("run img_writeheader", 0);
		if (ret != 0)
			break;
	}
	return ret;
}

static int ait_menu_install_images(void)
{
	int ret = 0;
	int count = 0;
	char s[100];
	char *t;

	/*
	 * possible image types:
	 * FIT_SUBTYPE_UNKNOWN
	 * FIT_SUBTYPE_UBL_HEADER
	 * FIT_SUBTYPE_SPL_IMAGE
	 * FIT_SUBTYPE_UBOOT_IMAGE
	 * FIT_SUBTYPE_DF_ENV_IMAGE
	 * FIT_SUBTYPE_RAMDISK_IMAGE
	 *
	 * use Envvariables:
	 * img_addr_r: image start addr
	 * header_addr: addr where to write to UBL header
	 * img_writeheader: write ubl header to nand
	 * img_writespl: write spl to nand
	 * img_writeuboot: write uboot to nand
	 * img_writedfenv: write default environment to ubi volume
	 * img_volume: which ubi volume should be updated with img_writeramdisk
	 * filesize: size of data for updating ubi volume
	 * img_writeramdisk: write ramdisk to ubi volume
	 */

	while (imgs[count].type != IH_TYPE_INVALID) {
		printf("Installing %s\n",
			genimg_get_type_name(imgs[count].type));
		sprintf(s, "%p", imgs[count].data);
		setenv("img_addr_r", s);
		sprintf(s, "%lx", (unsigned long)imgs[count].size);
		setenv("filesize", s);
		switch (imgs[count].subtype) {
		case FIT_SUBTYPE_DF_ENV_IMAGE:
			ret = run_command("run img_writedfenv", 0);
			break;
		case FIT_SUBTYPE_RAMDISK_IMAGE:
			t = getenv("img_volume");
			if (!t) {
				ret = setenv("img_volume", "rootfs1");
			} else {
				/* switch to other volume */
				if (strncmp(t, "rootfs1", 7) == 0)
					ret = setenv("img_volume", "rootfs2");
				else
					ret = setenv("img_volume", "rootfs1");
			}
			if (ret != 0)
				break;

			ret = run_command("run img_writeramdisk", 0);
			break;
		case FIT_SUBTYPE_SPL_IMAGE:
			ret = run_command("run img_writespl", 0);
			break;
		case FIT_SUBTYPE_UBL_HEADER:
			ret = ait_writeublheader();
			break;
		case FIT_SUBTYPE_UBOOT_IMAGE:
			ret = run_command("run img_writeuboot", 0);
			break;
		default:
			/* not supported type */
			break;
		}
		count++;
	}
	/* now save dvn_* and img_volume env vars to new values */
	if (ret == 0) {
		t = getenv("x_dvn_boot_vers");
		if (t)
			setenv("dvn_boot_vers", t);

		t = getenv("x_dvn_app_vers");
		if (t)
			setenv("dvn_boot_vers", t);

		setenv("x_dvn_boot_vers", NULL);
		setenv("x_dvn_app_vers", NULL);
		ret = run_command("run savenewvers", 0);
	}

	return ret;
}

static int ait_menu_evaluate_load(char *choice)
{
	if (!choice)
		return -1;

	switch (choice[1]) {
	case '1':
		/* install image */
		ait_menu_install_images();
		break;
	case '2':
		/* cancel, back to main */
		setenv("x_dvn_boot_vers", NULL);
		setenv("x_dvn_app_vers", NULL);
		break;
	}

	return MENU_MAIN;
}

struct menu_display ait_load = {
	.title = "AIT load image",
	.timeout = 0,
	.id = MENU_LOAD,
	.menulist = menu_load,
	.menu_evaluate = ait_menu_evaluate_load,
};

static void ait_menu_read_env(char *name)
{
	char output[CONFIG_SYS_CBSIZE];
	char cbuf[CONFIG_SYS_CBSIZE];
	int readret;
	int ret;

	sprintf(output, "%s old: %s value: ", name, getenv(name));
	memset(cbuf, 0, CONFIG_SYS_CBSIZE);
	readret = cli_readline_into_buffer(output, cbuf, 0);

	if (readret >= 0) {
		ret = setenv(name, cbuf);
		if (ret) {
			printf("Error setting %s\n", name);
			return;
		}
	}
	return;
}

static int ait_menu_evaluate_network(char *choice)
{
	if (!choice)
		return MENU_MAIN;

	switch (choice[1]) {
	case '1':
		ait_menu_read_env("ipaddr");
		break;
	case '2':
		ait_menu_read_env("netmask");
		break;
	case '3':
		ait_menu_read_env("serverip");
		break;
	case '4':
		ait_menu_read_env("gatewayip");
		break;
	case '5':
		ait_menu_read_env("img_file");
		break;
	case '6':
		return MENU_UPDATE;
		break;
	}

	return MENU_STAY;
}

struct menu_display ait_network = {
	.title = "AIT network settings",
	.timeout = 0,
	.id = MENU_NETWORK,
	.menulist = menu_network,
	.menu_evaluate = ait_menu_evaluate_network,
};

static int fit_get_subtype(const void *fit, int noffset, char **subtype)
{
	int len;

	*subtype = (char *)fdt_getprop(fit, noffset, "subtype", &len);
	if (*subtype == NULL)
		return -1;

	return 0;
}

static int ait_subtype_nr(char *subtype)
{
	int ret = FIT_SUBTYPE_UNKNOWN;

	if (!strncmp("ublheader", subtype, strlen("ublheader")))
		return FIT_SUBTYPE_UBL_HEADER;
	if (!strncmp("splimage", subtype, strlen("splimage")))
		return FIT_SUBTYPE_SPL_IMAGE;
	if (!strncmp("ubootimage", subtype, strlen("ubootimage")))
		return FIT_SUBTYPE_UBOOT_IMAGE;
	if (!strncmp("dfenvimage", subtype, strlen("dfenvimage")))
		return FIT_SUBTYPE_DF_ENV_IMAGE;

	return ret;
}

static int ait_menu_check_image(void)
{
	char *s;
	unsigned long fit_addr;
	void *addr;
	int format;
	char *desc;
	char *subtype;
	int images_noffset;
	int noffset;
	int ndepth;
	int count = 0;
	int ret;
	int i;
	int found_uboot = -1;
	int found_ramdisk = -1;

	memset(imgs, 0, sizeof(imgs));
	s = getenv("fit_addr_r");
	fit_addr = s ? (unsigned long)simple_strtol(s, NULL, 16) : \
			CONFIG_BOARD_IMG_ADDR_R;

	addr = (void *)fit_addr;
	/* check if it is a FIT image */
	format = genimg_get_format(addr);
	if (format != IMAGE_FORMAT_FIT)
		return -EINVAL;

	if (!fit_check_format(addr))
		return -EINVAL;

	/* print the FIT description */
	ret = fit_get_desc(addr, 0, &desc);
	printf("FIT description: ");
	if (ret)
		printf("unavailable\n");
	else
		printf("%s\n", desc);

	/* find images */
	images_noffset = fdt_path_offset(addr, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
			FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return -EINVAL;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(addr, images_noffset, &ndepth);
		(noffset >= 0) && (ndepth > 0);
		noffset = fdt_next_node(addr, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			printf("Image %u (%s)\n", count,
					fit_get_name(addr, noffset, NULL));

			fit_image_print(addr, noffset, "");

			fit_image_get_type(addr, noffset,
				&imgs[count].type);
			/* Mandatory properties */
			ret = fit_get_desc(addr, noffset, &desc);
			printf("Description:  ");
			if (ret)
				printf("unavailable\n");
			else
				printf("%s\n", desc);

			ret = fit_get_subtype(addr, noffset, &subtype);
			printf("Subtype:  ");
			if (ret) {
				printf("unavailable\n");
			} else {
				imgs[count].subtype = ait_subtype_nr(subtype);
				printf("%s %d\n", subtype,
					imgs[count].subtype);
			}

			sprintf(imgs[count].desc, "%s", desc);

			ret = fit_image_get_data(addr, noffset,
				&imgs[count].data,
				&imgs[count].size);

			printf("Data Size:    ");
			if (ret)
				printf("unavailable\n");
			else
				genimg_print_size(imgs[count].size);
			printf("Data @ %p\n", imgs[count].data);
			count++;
		}
	}

	for (i = 0; i < count; i++) {
		if (imgs[i].subtype == FIT_SUBTYPE_UBOOT_IMAGE)
			found_uboot = i;
		if (imgs[i].type == IH_TYPE_RAMDISK) {
			found_ramdisk = i;
			imgs[i].subtype = FIT_SUBTYPE_RAMDISK_IMAGE;
		}
	}

	/* dvn_* env var update, if the FIT descriptors are different */
	if (found_uboot >= 0) {
		s = getenv("dvn_boot_vers");
		if (s) {
			ret = strcmp(s, imgs[found_uboot].desc);
			if (ret != 0) {
				setenv("x_dvn_boot_vers",
					imgs[found_uboot].desc);
			} else {
				found_uboot = -1;
				printf("no new uboot version\n");
			}
		} else {
			setenv("dvn_boot_vers", imgs[found_uboot].desc);
		}
	}
	if (found_ramdisk >= 0) {
		s = getenv("dvn_app_vers");
		if (s) {
			ret = strcmp(s, imgs[found_ramdisk].desc);
			if (ret != 0) {
				setenv("x_dvn_app_vers",
					imgs[found_ramdisk].desc);
			} else {
				found_ramdisk = -1;
				printf("no new ramdisk version\n");
			}
		} else {
			setenv("dvn_app_vers", imgs[found_ramdisk].desc);
		}
	}
	if ((found_uboot == -1) && (found_ramdisk == -1))
		return -EINVAL;

	return 0;
}

static int ait_menu_evaluate_update(char *choice)
{
	int ret;

	if (!choice)
		return MENU_MAIN;

	switch (choice[1]) {
	case '1':
		return ait_menu_show(&ait_network, 0);
		break;
	case '2':
		/* load image */
		ret = run_command("run load_img", 0);
		printf("ret: %d\n", ret);
		if (ret)
			return MENU_UPDATE;

		ret = ait_menu_check_image();
		if (ret)
			return MENU_UPDATE;

		return ait_menu_show(&ait_load, 0);
		break;
	case '3':
		return MENU_MAIN;
		break;

	}

	return MENU_MAIN;
}

struct menu_display ait_update = {
	.title = "AIT Update Software",
	.timeout = 0,
	.id = MENU_UPDATE,
	.menulist = menu_update,
	.menu_evaluate = ait_menu_evaluate_update,
};

static int ait_menu_evaluate_main(char *choice)
{
	if (!choice)
		return MENU_STAY;

	menu_start = 1;
	switch (choice[1]) {
	case '1':
		/* run bootcmd */
		return MENU_EXIT_BOOTCMD;
		break;
	case '2':
		return ait_menu_show(&ait_update, 0);
		break;
	case '3':
		/* reset to default settings */
		setenv("app_reset", "yes");
		return MENU_EXIT_BOOTCMD;
		break;
	case '4':
		/* u-boot shell */
		return MENU_EXIT;
		break;
	}

	return MENU_EXIT;
}

struct menu_display ait_main = {
	.title = "AIT Main",
	.timeout = CONFIG_BOOTDELAY,
	.id = MENU_MAIN,
	.menulist = menu_main,
	.menu_evaluate = ait_menu_evaluate_main,
};

int menu_show(int bootdelay)
{
	int ret;

	run_command("run saveparms", 0);
	ret = ait_menu_show(&ait_main, bootdelay);
	run_command("run restoreparms", 0);

	if (ret == MENU_EXIT_BOOTCMD)
		return 0;

	return MENU_EXIT;
}

void menu_display_statusline(struct menu *m)
{
	char *s1, *s2;

	s1 = getenv("x_dvn_boot_vers");
	if (!s1)
		s1 = getenv("dvn_boot_vers");

	s2 = getenv("x_dvn_app_vers");
	if (!s2)
		s2 = getenv("dvn_app_vers");

	printf("State: dvn_boot_vers: %s dvn_app_vers: %s\n", s1, s2);
	return;
}
#endif
