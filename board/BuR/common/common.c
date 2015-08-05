/*
 * common.c
 *
 * common board functions for B&R boards
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */
#include <version.h>
#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include <lcd.h>
#include <fs.h>
#ifdef CONFIG_USE_FDT
  #include <fdt_support.h>
#endif
#include "bur_common.h"
#include "../../../drivers/video/am335x-fb.h"
#include <nand.h>
#include <fdt_simplefb.h>

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USE_FDT
  #define FDTPROP(b, c) fdt_getprop_u32_default(gd->fdt_blob, b, c, ~0UL)
  #define PATHTIM "/panel/display-timings/default"
  #define PATHINF "/panel/panel-info"
#endif
/* --------------------------------------------------------------------------*/
#if defined(CONFIG_LCD) && defined(CONFIG_AM335X_LCD) && \
	!defined(CONFIG_SPL_BUILD)
void lcdbacklight(int on)
{
#ifdef CONFIG_USE_FDT
	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return;
	}
	unsigned int driver = FDTPROP(PATHINF, "brightdrv");
	unsigned int bright = FDTPROP(PATHINF, "brightdef");
	unsigned int pwmfrq = FDTPROP(PATHINF, "brightfdim");
#else
	unsigned int driver = getenv_ulong("ds1_bright_drv", 16, 0UL);
	unsigned int bright = getenv_ulong("ds1_bright_def", 10, 50);
	unsigned int pwmfrq = getenv_ulong("ds1_pwmfreq", 10, ~0UL);
#endif
	unsigned int tmp;
	struct gptimer *timerhw;

	if (on)
		bright = bright != ~0UL ? bright : 50;
	else
		bright = 0;

	switch (driver) {
	case 2:
		timerhw = (struct gptimer *)DM_TIMER5_BASE;
		break;
	default:
		timerhw = (struct gptimer *)DM_TIMER6_BASE;
	}

	switch (driver) {
	case 0:	/* PMIC LED-Driver */
		/* brightness level */
		tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				   TPS65217_WLEDCTRL2, bright, 0xFF);
		/* current sink */
		tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				   TPS65217_WLEDCTRL1,
				   bright != 0 ? 0x0A : 0x02,
				   0xFF);
		break;
	case 1:
	case 2: /* PWM using timer */
		if (pwmfrq != ~0UL) {
			timerhw->tiocp_cfg = TCFG_RESET;
			udelay(10);
			while (timerhw->tiocp_cfg & TCFG_RESET)
				;
			tmp = ~0UL-(V_OSCK/pwmfrq);	/* bottom value */
			timerhw->tldr = tmp;
			timerhw->tcrr = tmp;
			tmp = tmp + ((V_OSCK/pwmfrq)/100) * bright;
			timerhw->tmar = tmp;
			timerhw->tclr = (TCLR_PT | (2 << TCLR_TRG_SHIFT) |
					TCLR_CE | TCLR_AR | TCLR_ST);
		} else {
			puts("invalid pwmfrq in env/dtb! skip PWM-setup.\n");
		}
		break;
	default:
		puts("no suitable backlightdriver in env/dtb!\n");
		break;
	}
}

int load_lcdtiming(struct am335x_lcdpanel *panel)
{
	struct am335x_lcdpanel pnltmp;
#ifdef CONFIG_USE_FDT
	u32 dtbprop;
	char buf[32];
	const char *nodep = 0;
	int nodeoff;

	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return -1;
	}
	memcpy(&pnltmp, (void *)panel, sizeof(struct am335x_lcdpanel));

	pnltmp.hactive = FDTPROP(PATHTIM, "hactive");
	pnltmp.vactive = FDTPROP(PATHTIM, "vactive");
	pnltmp.bpp = FDTPROP(PATHINF, "bpp");
	pnltmp.hfp = FDTPROP(PATHTIM, "hfront-porch");
	pnltmp.hbp = FDTPROP(PATHTIM, "hback-porch");
	pnltmp.hsw = FDTPROP(PATHTIM, "hsync-len");
	pnltmp.vfp = FDTPROP(PATHTIM, "vfront-porch");
	pnltmp.vbp = FDTPROP(PATHTIM, "vback-porch");
	pnltmp.vsw = FDTPROP(PATHTIM, "vsync-len");
	pnltmp.pup_delay = FDTPROP(PATHTIM, "pupdelay");
	pnltmp.pon_delay = FDTPROP(PATHTIM, "pondelay");

	/* calc. proper clk-divisor */
	dtbprop = FDTPROP(PATHTIM, "clock-frequency");
	if (dtbprop != ~0UL)
		pnltmp.pxl_clk_div = 192000000 / dtbprop;
	else
		pnltmp.pxl_clk_div = ~0UL;

	/* check polarity of control-signals */
	dtbprop = FDTPROP(PATHTIM, "hsync-active");
	if (dtbprop == 0)
		pnltmp.pol |= HSYNC_INVERT;
	dtbprop = FDTPROP(PATHTIM, "vsync-active");
	if (dtbprop == 0)
		pnltmp.pol |= VSYNC_INVERT;
	dtbprop = FDTPROP(PATHINF, "sync-ctrl");
	if (dtbprop == 1)
		pnltmp.pol |= HSVS_CONTROL;
	dtbprop = FDTPROP(PATHINF, "sync-edge");
	if (dtbprop == 1)
		pnltmp.pol |= HSVS_RISEFALL;
	dtbprop = FDTPROP(PATHTIM, "pixelclk-active");
	if (dtbprop == 0)
		pnltmp.pol |= PXCLK_INVERT;
	dtbprop = FDTPROP(PATHTIM, "de-active");
	if (dtbprop == 0)
		pnltmp.pol |= DE_INVERT;

	nodeoff = fdt_path_offset(gd->fdt_blob, "/factory-settings");
	if (nodeoff >= 0) {
		nodep = fdt_getprop(gd->fdt_blob, nodeoff, "rotation", NULL);
		if (nodep != 0) {
			if (strcmp(nodep, "cw") == 0)
				panel_info.vl_rot = 1;
			else if (strcmp(nodep, "ud") == 0)
				panel_info.vl_rot = 2;
			else if (strcmp(nodep, "ccw") == 0)
				panel_info.vl_rot = 3;
			else
				panel_info.vl_rot = 0;
		}
	} else {
		puts("no 'factory-settings / rotation' in dtb!\n");
	}
	snprintf(buf, sizeof(buf), "fbcon=rotate:%d", panel_info.vl_rot);
	setenv("optargs_rot", buf);
#else
	pnltmp.hactive = getenv_ulong("ds1_hactive", 10, ~0UL);
	pnltmp.vactive = getenv_ulong("ds1_vactive", 10, ~0UL);
	pnltmp.bpp = getenv_ulong("ds1_bpp", 10, ~0UL);
	pnltmp.hfp = getenv_ulong("ds1_hfp", 10, ~0UL);
	pnltmp.hbp = getenv_ulong("ds1_hbp", 10, ~0UL);
	pnltmp.hsw = getenv_ulong("ds1_hsw", 10, ~0UL);
	pnltmp.vfp = getenv_ulong("ds1_vfp", 10, ~0UL);
	pnltmp.vbp = getenv_ulong("ds1_vbp", 10, ~0UL);
	pnltmp.vsw = getenv_ulong("ds1_vsw", 10, ~0UL);
	pnltmp.pxl_clk_div = getenv_ulong("ds1_pxlclkdiv", 10, ~0UL);
	pnltmp.pol = getenv_ulong("ds1_pol", 16, ~0UL);
	pnltmp.pup_delay = getenv_ulong("ds1_pupdelay", 10, ~0UL);
	pnltmp.pon_delay = getenv_ulong("ds1_tondelay", 10, ~0UL);
	panel_info.vl_rot = getenv_ulong("ds1_rotation", 10, 0);
#endif
	if (
	   ~0UL == (pnltmp.hactive) ||
	   ~0UL == (pnltmp.vactive) ||
	   ~0UL == (pnltmp.bpp) ||
	   ~0UL == (pnltmp.hfp) ||
	   ~0UL == (pnltmp.hbp) ||
	   ~0UL == (pnltmp.hsw) ||
	   ~0UL == (pnltmp.vfp) ||
	   ~0UL == (pnltmp.vbp) ||
	   ~0UL == (pnltmp.vsw) ||
	   ~0UL == (pnltmp.pxl_clk_div) ||
	   ~0UL == (pnltmp.pol) ||
	   ~0UL == (pnltmp.pup_delay) ||
	   ~0UL == (pnltmp.pon_delay)
	   ) {
		puts("lcd-settings in env/dtb incomplete!\n");
		printf("display-timings:\n"
			"================\n"
			"hactive: %d\n"
			"vactive: %d\n"
			"bpp    : %d\n"
			"hfp    : %d\n"
			"hbp    : %d\n"
			"hsw    : %d\n"
			"vfp    : %d\n"
			"vbp    : %d\n"
			"vsw    : %d\n"
			"pxlclk : %d\n"
			"pol    : 0x%08x\n"
			"pondly : %d\n",
			pnltmp.hactive, pnltmp.vactive, pnltmp.bpp,
			pnltmp.hfp, pnltmp.hbp, pnltmp.hsw,
			pnltmp.vfp, pnltmp.vbp, pnltmp.vsw,
			pnltmp.pxl_clk_div, pnltmp.pol, pnltmp.pon_delay);

		return -1;
	}
	debug("lcd-settings in env complete, taking over.\n");
	memcpy((void *)panel,
	       (void *)&pnltmp,
	       sizeof(struct am335x_lcdpanel));

	return 0;
}

#ifdef CONFIG_USE_FDT
static int load_devicetree(void)
{
	int rc;
	loff_t dtbsize;
	u32 dtbaddr = getenv_ulong("dtbaddr", 16, 0UL);

	if (dtbaddr == 0) {
		printf("%s: don't have a valid <dtbaddr> in env!\n", __func__);
		return -1;
	}
#ifdef CONFIG_NAND
	dtbsize = 0x20000;
	rc = nand_read_skip_bad(&nand_info[0], 0x40000, (size_t *)&dtbsize,
				NULL, 0x20000, (u_char *)dtbaddr);
#else
	char *dtbname = getenv("dtb");
	char *dtbdev = getenv("dtbdev");
	char *dtppart = getenv("dtbpart");
	if (!dtbdev || !dtbdev || !dtbname) {
		printf("%s: <dtbdev>/<dtbpart>/<dtb> missing.\n", __func__);
		return -1;
	}

	if (fs_set_blk_dev(dtbdev, dtppart, FS_TYPE_EXT)) {
		puts("load_devicetree: set_blk_dev failed.\n");
		return -1;
	}
	rc = fs_read(dtbname, (u32)dtbaddr, 0, 0, &dtbsize);
#endif
	if (rc == 0) {
		gd->fdt_blob = (void *)dtbaddr;
		gd->fdt_size = dtbsize;
		debug("loaded %d bytes of dtb onto 0x%08x\n",
		      (u32)dtbsize, (u32)gd->fdt_blob);
		return dtbsize;
	}

	printf("%s: load dtb failed!\n", __func__);
	return -1;
}

static const char *dtbmacaddr(u32 ifno)
{
	int node, len;
	char enet[16];
	const char *mac;
	const char *path;

	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return NULL;
	}

	node = fdt_path_offset(gd->fdt_blob, "/aliases");
	if (node < 0)
		return NULL;

	sprintf(enet, "ethernet%d", ifno);
	path = fdt_getprop(gd->fdt_blob, node, enet, NULL);
	if (!path) {
		printf("no alias for %s\n", enet);
		return NULL;
	}

	node = fdt_path_offset(gd->fdt_blob, path);
	mac = fdt_getprop(gd->fdt_blob, node, "mac-address", &len);
	if (mac && is_valid_ethaddr((u8 *)mac))
		return mac;

	return NULL;
}

static void br_summaryscreen_printdtb(char *prefix,
				       char *name,
				       char *suffix)
{
	char buf[32] = { 0 };
	const char *nodep = buf;
	char *mac = 0;
	int nodeoffset;
	int len;

	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return;
	}

	if (strcmp(name, "brmac1") == 0) {
		mac = (char *)dtbmacaddr(0);
		if (mac)
			sprintf(buf, "%pM", mac);
	} else if (strcmp(name, "brmac2") == 0) {
		mac =  (char *)dtbmacaddr(1);
		if (mac)
			sprintf(buf, "%pM", mac);
	} else {
		nodeoffset = fdt_path_offset(gd->fdt_blob,
					     "/factory-settings");
		if (nodeoffset < 0) {
			puts("no 'factory-settings' in dtb!\n");
			return;
		}
		nodep = fdt_getprop(gd->fdt_blob, nodeoffset, name, &len);
	}
	if (nodep && strlen(nodep) > 1)
		lcd_printf("%s %s %s", prefix, nodep, suffix);
	else
		lcd_printf("\n");
}
int ft_board_setup(void *blob, bd_t *bd)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/factory-settings");
	if (nodeoffset < 0) {
		puts("set bootloader version 'factory-settings' not in dtb!\n");
		return -1;
	}
	if (fdt_setprop(blob, nodeoffset, "bl-version",
			PLAIN_VERSION, strlen(PLAIN_VERSION)) != 0) {
		puts("set bootloader version 'bl-version' prop. not in dtb!\n");
		return -1;
	}
	/*
	 * if no simplefb is requested through environment, we don't set up
	 * one, instead we turn off backlight.
	 */
	if (getenv_ulong("simplefb", 10, 0) == 0) {
		lcdbacklight(0);
		return 0;
	}
	/* Setup simplefb devicetree node, also adapt memory-node,
	 * upper limit for kernel e.g. linux is memtop-framebuffer alligned
	 * to a full megabyte.
	 */
	u64 start = gd->bd->bi_dram[0].start;
	u64 size = (gd->fb_base - start) & ~0xFFFFF;
	int rc = fdt_fixup_memory_banks(blob, &start, &size, 1);

	if (rc) {
		puts("cannot setup simplefb: Error reserving memory!\n");
		return rc;
	}
	rc = lcd_dt_simplefb_enable_existing_node(blob);
	if (rc) {
		puts("cannot setup simplefb: error enabling simplefb node!\n");
		return rc;
	}

	return 0;
}
#else

static void br_summaryscreen_printenv(char *prefix,
				       char *name, char *altname,
				       char *suffix)
{
	char *envval = getenv(name);
	if (0 != envval) {
		lcd_printf("%s %s %s", prefix, envval, suffix);
	} else if (0 != altname) {
		envval = getenv(altname);
		if (0 != envval)
			lcd_printf("%s %s %s", prefix, envval, suffix);
	} else {
		lcd_printf("\n");
	}
}
#endif
void br_summaryscreen(void)
{
#ifdef CONFIG_USE_FDT
	br_summaryscreen_printdtb(" - B&R -", "order-no", "-\n");
	br_summaryscreen_printdtb(" Serial/Rev :", "serial-no", " /");
	br_summaryscreen_printdtb(" ", "hw-revision", "\n");
	br_summaryscreen_printdtb(" MAC (IF1)  :", "brmac1", "\n");
	br_summaryscreen_printdtb(" MAC (IF2)  :", "brmac2", "\n");
	lcd_puts(" Bootloader : " PLAIN_VERSION "\n");
	lcd_puts("\n");
#else
	br_summaryscreen_printenv(" - B&R -", "br_orderno", 0, "-\n");
	br_summaryscreen_printenv(" Serial/Rev :", "br_serial", 0, "\n");
	br_summaryscreen_printenv(" MAC (IF1)  :", "br_mac1", "ethaddr", "\n");
	br_summaryscreen_printenv(" MAC (IF2)  :", "br_mac2", 0, "\n");
	lcd_puts(" Bootloader : " PLAIN_VERSION "\n");
	lcd_puts("\n");
#endif
}

void lcdpower(int on)
{
	u32 pin, swval, i;
#ifdef CONFIG_USE_FDT
	if (gd->fdt_blob == NULL) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return;
	}
	pin = FDTPROP(PATHINF, "pwrpin");
#else
	pin = getenv_ulong("ds1_pwr", 16, ~0UL);
#endif
	if (pin == ~0UL) {
		puts("no pwrpin in dtb/env, cannot powerup display!\n");
		return;
	}

	for (i = 0; i < 3; i++) {
		if (pin != 0) {
			swval = pin & 0x80 ? 0 : 1;
			if (on)
				gpio_direction_output(pin & 0x7F, swval);
			else
				gpio_direction_output(pin & 0x7F, !swval);

			debug("switched pin %d to %d\n", pin & 0x7F, swval);
		}
		pin >>= 8;
	}
}

vidinfo_t	panel_info = {
		.vl_col = 1366,	/*
				 * give full resolution for allocating enough
				 * memory
				 */
		.vl_row = 768,
		.vl_bpix = 5,
		.priv = 0
};

void lcd_ctrl_init(void *lcdbase)
{
	struct am335x_lcdpanel lcd_panel;
#ifdef CONFIG_USE_FDT
	/* TODO: is there a better place to load the dtb ? */
	load_devicetree();
#endif
	memset(&lcd_panel, 0, sizeof(struct am335x_lcdpanel));
	if (load_lcdtiming(&lcd_panel) != 0)
		return;

	lcd_panel.panel_power_ctrl = &lcdpower;

	if (0 != am335xfb_init(&lcd_panel))
		printf("ERROR: failed to initialize video!");
	/*
	 * modifiy panel info to 'real' resolution, to operate correct with
	 * lcd-framework.
	 */
	panel_info.vl_col = lcd_panel.hactive;
	panel_info.vl_row = lcd_panel.vactive;

	lcd_set_flush_dcache(1);
}

void lcd_enable(void)
{
	br_summaryscreen();
	lcdbacklight(1);
}
#elif CONFIG_SPL_BUILD
#else
#error "LCD-support with a suitable FB-Driver is mandatory !"
#endif /* CONFIG_LCD */

#ifdef CONFIG_SPL_BUILD
void pmicsetup(u32 mpupll)
{
	int mpu_vdd;
	int usb_cur_lim;

	if (i2c_probe(TPS65217_CHIP_PM)) {
		puts("PMIC (0x24) not found! skip further initalization.\n");
		return;
	}

	/* Get the frequency which is defined by device fuses */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	printf("detected max. frequency: %d - ", dpll_mpu_opp100.m);

	if (0 != mpupll) {
		dpll_mpu_opp100.m = MPUPLL_M_1000;
		printf("retuning MPU-PLL to: %d MHz.\n", dpll_mpu_opp100.m);
	} else {
		puts("ok.\n");
	}
	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
	} else {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
	}

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_POWER_PATH,
			       usb_cur_lim, TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set LDO3 to 1.8V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");
	/* Set LDO4 to 3.3V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
	/* Set PWR_EN bit in Status Register */
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			   TPS65217_STATUS, TPS65217_PWR_OFF, TPS65217_PWR_OFF);
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

#endif /* CONFIG_SPL_BUILD */

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
static void cpsw_control(int enabled)
{
	/* VTP can be added here */
	return;
}

/* describing port offsets of TI's CPSW block */
static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 1,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 2,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 1,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};
#endif /* CONFIG_DRIVER_TI_CPSW, ... */

#if defined(CONFIG_DRIVER_TI_CPSW)

int board_eth_init(bd_t *bis)
{
	int rv = 0;
	char mac_addr[6];
	const char *mac = 0;
	uint32_t mac_hi, mac_lo;
	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

#if (defined(CONFIG_DRIVER_TI_CPSW) && !defined(CONFIG_SPL_BUILD)) || \
	(defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD))
	if (!getenv("ethaddr")) {
		#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_USE_FDT)
		printf("<ethaddr> not set. trying DTB ... ");
		mac = dtbmacaddr(0);
		#endif
		if (!mac) {
			printf("<ethaddr> not set. validating E-fuse MAC ... ");
			if (is_valid_ethaddr((const u8 *)mac_addr))
				mac = (const char *)mac_addr;
		}

		if (mac) {
			printf("using: %pM on ", mac);
			eth_setenv_enetaddr("ethaddr", (const u8 *)mac);
		}
	}
	writel(MII_MODE_ENABLE, &cdev->miisel);
	cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_MII;
	cpsw_slaves[1].phy_if =	PHY_INTERFACE_MODE_MII;

	rv = cpsw_register(&cpsw_data);
	if (rv < 0) {
		printf("Error %d registering CPSW switch\n", rv);
		return 0;
	}
#endif /* CONFIG_DRIVER_TI_CPSW, ... */
	return rv;
}
#endif /* CONFIG_DRIVER_TI_CPSW */
#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif
int overwrite_console(void)
{
	return 1;
}
