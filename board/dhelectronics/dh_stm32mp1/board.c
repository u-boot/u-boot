// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <adc.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <bootm.h>
#include <clk.h>
#include <config.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <env.h>
#include <env_internal.h>
#include <g_dnl.h>
#include <generic-phy.h>
#include <hang.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <init.h>
#include <led.h>
#include <memalign.h>
#include <misc.h>
#include <mtd.h>
#include <mtd_node.h>
#include <netdev.h>
#include <phy.h>
#include <power/regulator.h>
#include <remoteproc.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <watchdog.h>

/* SYSCFG registers */
#define SYSCFG_BOOTR		0x00
#define SYSCFG_PMCSETR		0x04
#define SYSCFG_IOCTRLSETR	0x18
#define SYSCFG_ICNR		0x1C
#define SYSCFG_CMPCR		0x20
#define SYSCFG_CMPENSETR	0x24
#define SYSCFG_PMCCLRR		0x44

#define SYSCFG_BOOTR_BOOT_MASK		GENMASK(2, 0)
#define SYSCFG_BOOTR_BOOTPD_SHIFT	4

#define SYSCFG_IOCTRLSETR_HSLVEN_TRACE		BIT(0)
#define SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI	BIT(1)
#define SYSCFG_IOCTRLSETR_HSLVEN_ETH		BIT(2)
#define SYSCFG_IOCTRLSETR_HSLVEN_SDMMC		BIT(3)
#define SYSCFG_IOCTRLSETR_HSLVEN_SPI		BIT(4)

#define SYSCFG_CMPCR_SW_CTRL		BIT(1)
#define SYSCFG_CMPCR_READY		BIT(8)

#define SYSCFG_CMPENSETR_MPU_EN		BIT(0)

#define SYSCFG_PMCSETR_ETH_CLK_SEL	BIT(16)
#define SYSCFG_PMCSETR_ETH_REF_CLK_SEL	BIT(17)

#define SYSCFG_PMCSETR_ETH_SELMII	BIT(20)

#define SYSCFG_PMCSETR_ETH_SEL_MASK	GENMASK(23, 21)
#define SYSCFG_PMCSETR_ETH_SEL_GMII_MII	0
#define SYSCFG_PMCSETR_ETH_SEL_RGMII	BIT(21)
#define SYSCFG_PMCSETR_ETH_SEL_RMII	BIT(23)

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

int setup_mac_address(void)
{
	unsigned char enetaddr[6];
	struct udevice *dev;
	int off, ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		return 0;

	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("Cannot find EEPROM!\n");
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0xfa, enetaddr, 0x6);
	if (ret) {
		printf("Error reading configuration EEPROM!\n");
		return ret;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("ethaddr", enetaddr);

	return 0;
}

int checkboard(void)
{
	char *mode;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_STM32MP1_OPTEE))
		mode = "trusted with OP-TEE";
	else if (IS_ENABLED(CONFIG_STM32MP1_TRUSTED))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	return 0;
}

static void board_key_check(void)
{
#if defined(CONFIG_FASTBOOT) || defined(CONFIG_CMD_STM32PROG)
	ofnode node;
	struct gpio_desc gpio;
	enum forced_boot_mode boot_mode = BOOT_NORMAL;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		debug("%s: no /config node?\n", __func__);
		return;
	}
#ifdef CONFIG_FASTBOOT
	if (gpio_request_by_name_nodev(node, "st,fastboot-gpios", 0,
				       &gpio, GPIOD_IS_IN)) {
		debug("%s: could not find a /config/st,fastboot-gpios\n",
		      __func__);
	} else {
		if (dm_gpio_get_value(&gpio)) {
			puts("Fastboot key pressed, ");
			boot_mode = BOOT_FASTBOOT;
		}

		dm_gpio_free(NULL, &gpio);
	}
#endif
#ifdef CONFIG_CMD_STM32PROG
	if (gpio_request_by_name_nodev(node, "st,stm32prog-gpios", 0,
				       &gpio, GPIOD_IS_IN)) {
		debug("%s: could not find a /config/st,stm32prog-gpios\n",
		      __func__);
	} else {
		if (dm_gpio_get_value(&gpio)) {
			puts("STM32Programmer key pressed, ");
			boot_mode = BOOT_STM32PROG;
		}
		dm_gpio_free(NULL, &gpio);
	}
#endif

	if (boot_mode != BOOT_NORMAL) {
		puts("entering download mode...\n");
		clrsetbits_le32(TAMP_BOOT_CONTEXT,
				TAMP_BOOT_FORCED_MASK,
				boot_mode);
	}
#endif
}

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)

#include <usb/dwc2_udc.h>
int g_dnl_board_usb_cable_connected(void)
{
	struct udevice *dwc2_udc_otg;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_USB_GADGET_GENERIC,
					  DM_GET_DRIVER(dwc2_udc_otg),
					  &dwc2_udc_otg);
	if (!ret)
		debug("dwc2_udc_otg init failed\n");

	return dwc2_udc_B_session_valid(dwc2_udc_otg);
}

#define STM32MP1_G_DNL_DFU_PRODUCT_NUM 0xdf11
#define STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM 0x0afb

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (!strcmp(name, "usb_dnl_dfu"))
		put_unaligned(STM32MP1_G_DNL_DFU_PRODUCT_NUM, &dev->idProduct);
	else if (!strcmp(name, "usb_dnl_fastboot"))
		put_unaligned(STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM,
			      &dev->idProduct);
	else
		put_unaligned(CONFIG_USB_GADGET_PRODUCT_NUM, &dev->idProduct);

	return 0;
}

#endif /* CONFIG_USB_GADGET */

#ifdef CONFIG_LED
static int get_led(struct udevice **dev, char *led_string)
{
	char *led_name;
	int ret;

	led_name = fdtdec_get_config_string(gd->fdt_blob, led_string);
	if (!led_name) {
		pr_debug("%s: could not find %s config string\n",
			 __func__, led_string);
		return -ENOENT;
	}
	ret = led_get_by_label(led_name, dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int setup_led(enum led_state_t cmd)
{
	struct udevice *dev;
	int ret;

	ret = get_led(&dev, "u-boot,boot-led");
	if (ret)
		return ret;

	ret = led_set_state(dev, cmd);
	return ret;
}
#endif

static void __maybe_unused led_error_blink(u32 nb_blink)
{
#ifdef CONFIG_LED
	int ret;
	struct udevice *led;
	u32 i;
#endif

	if (!nb_blink)
		return;

#ifdef CONFIG_LED
	ret = get_led(&led, "u-boot,error-led");
	if (!ret) {
		/* make u-boot,error-led blinking */
		/* if U32_MAX and 125ms interval, for 17.02 years */
		for (i = 0; i < 2 * nb_blink; i++) {
			led_set_state(led, LEDST_TOGGLE);
			mdelay(125);
			WATCHDOG_RESET();
		}
	}
#endif

	/* infinite: the boot process must be stopped */
	if (nb_blink == U32_MAX)
		hang();
}

static void sysconf_init(void)
{
#ifndef CONFIG_STM32MP1_TRUSTED
	u8 *syscfg;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *pwr_dev;
	struct udevice *pwr_reg;
	struct udevice *dev;
	int ret;
	u32 otp = 0;
#endif
	u32 bootr;

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	/* interconnect update : select master using the port 1 */
	/* LTDC = AXI_M9 */
	/* GPU  = AXI_M8 */
	/* today information is hardcoded in U-Boot */
	writel(BIT(9), syscfg + SYSCFG_ICNR);

	/* disable Pull-Down for boot pin connected to VDD */
	bootr = readl(syscfg + SYSCFG_BOOTR);
	bootr &= ~(SYSCFG_BOOTR_BOOT_MASK << SYSCFG_BOOTR_BOOTPD_SHIFT);
	bootr |= (bootr & SYSCFG_BOOTR_BOOT_MASK) << SYSCFG_BOOTR_BOOTPD_SHIFT;
	writel(bootr, syscfg + SYSCFG_BOOTR);

#ifdef CONFIG_DM_REGULATOR
	/* High Speed Low Voltage Pad mode Enable for SPI, SDMMC, ETH, QSPI
	 * and TRACE. Needed above ~50MHz and conditioned by AFMUX selection.
	 * The customer will have to disable this for low frequencies
	 * or if AFMUX is selected but the function not used, typically for
	 * TRACE. Otherwise, impact on power consumption.
	 *
	 * WARNING:
	 *   enabling High Speed mode while VDD>2.7V
	 *   with the OTP product_below_2v5 (OTP 18, BIT 13)
	 *   erroneously set to 1 can damage the IC!
	 *   => U-Boot set the register only if VDD < 2.7V (in DT)
	 *      but this value need to be consistent with board design
	 */
	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_GET_DRIVER(stm32mp_pwr_pmic),
					  &pwr_dev);
	if (!ret) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_GET_DRIVER(stm32mp_bsec),
						  &dev);
		if (ret) {
			pr_err("Can't find stm32mp_bsec driver\n");
			return;
		}

		ret = misc_read(dev, STM32_BSEC_SHADOW(18), &otp, 4);
		if (ret > 0)
			otp = otp & BIT(13);

		/* get VDD = vdd-supply */
		ret = device_get_supply_regulator(pwr_dev, "vdd-supply",
						  &pwr_reg);

		/* check if VDD is Low Voltage */
		if (!ret) {
			if (regulator_get_value(pwr_reg) < 2700000) {
				writel(SYSCFG_IOCTRLSETR_HSLVEN_TRACE |
				       SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI |
				       SYSCFG_IOCTRLSETR_HSLVEN_ETH |
				       SYSCFG_IOCTRLSETR_HSLVEN_SDMMC |
				       SYSCFG_IOCTRLSETR_HSLVEN_SPI,
				       syscfg + SYSCFG_IOCTRLSETR);

				if (!otp)
					pr_err("product_below_2v5=0: HSLVEN protected by HW\n");
			} else {
				if (otp)
					pr_err("product_below_2v5=1: HSLVEN update is destructive, no update as VDD>2.7V\n");
			}
		} else {
			debug("VDD unknown");
		}
	}
#endif

	/* activate automatic I/O compensation
	 * warning: need to ensure CSI enabled and ready in clock driver
	 */
	writel(SYSCFG_CMPENSETR_MPU_EN, syscfg + SYSCFG_CMPENSETR);

	while (!(readl(syscfg + SYSCFG_CMPCR) & SYSCFG_CMPCR_READY))
		;
	clrbits_le32(syscfg + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);
#endif
}

/* board dependent setup after realloc */
int board_init(void)
{
	struct udevice *dev;

	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	/* probe all PINCTRL for hog */
	for (uclass_first_device(UCLASS_PINCTRL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		pr_debug("probe pincontrol = %s\n", dev->name);
	}

	board_key_check();

#ifdef CONFIG_DM_REGULATOR
	regulators_enable_boot_on(_DEBUG);
#endif

	sysconf_init();

	if (CONFIG_IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

int board_late_init(void)
{
	char *boot_device;
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const void *fdt_compat;
	int fdt_compat_len;

	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len) {
		if (strncmp(fdt_compat, "st,", 3) != 0)
			env_set("board_name", fdt_compat);
		else
			env_set("board_name", fdt_compat + 3);
	}
#endif

	/* Check the boot-source to disable bootdelay */
	boot_device = env_get("boot_device");
	if (!strcmp(boot_device, "serial") || !strcmp(boot_device, "usb"))
		env_set("bootdelay", "0");

	return 0;
}

void board_quiesce_devices(void)
{
#ifdef CONFIG_LED
	setup_led(LEDST_OFF);
#endif
}

/* eth init function : weak called in eqos driver */
int board_interface_eth_init(struct udevice *dev,
			     phy_interface_t interface_type)
{
	u8 *syscfg;
	u32 value;
	bool eth_clk_sel_reg = false;
	bool eth_ref_clk_sel_reg = false;

	/* Gigabit Ethernet 125MHz clock selection. */
	eth_clk_sel_reg = dev_read_bool(dev, "st,eth_clk_sel");

	/* Ethernet 50Mhz RMII clock selection */
	eth_ref_clk_sel_reg =
		dev_read_bool(dev, "st,eth_ref_clk_sel");

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	if (!syscfg)
		return -ENODEV;

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
			SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		debug("%s: PHY_INTERFACE_MODE_MII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_GMII:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII;
		debug("%s: PHY_INTERFACE_MODE_GMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (eth_ref_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RMII |
				SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RMII;
		debug("%s: PHY_INTERFACE_MODE_RMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII;
		debug("%s: PHY_INTERFACE_MODE_RGMII\n", __func__);
		break;
	default:
		debug("%s: Do not manage %d interface\n",
		      __func__, interface_type);
		/* Do not manage others interfaces */
		return -EINVAL;
	}

	/* clear and set ETH configuration bits */
	writel(SYSCFG_PMCSETR_ETH_SEL_MASK | SYSCFG_PMCSETR_ETH_SELMII |
	       SYSCFG_PMCSETR_ETH_REF_CLK_SEL | SYSCFG_PMCSETR_ETH_CLK_SEL,
	       syscfg + SYSCFG_PMCCLRR);
	writel(value, syscfg + SYSCFG_PMCSETR);

	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio)
		return ENVL_UNKNOWN;

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	return ENVL_SPI_FLASH;
#else
	return ENVL_NOWHERE;
#endif
}

#ifdef CONFIG_SYS_MTDPARTS_RUNTIME

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

/**
 * The mtdparts_nand0 and mtdparts_nor0 variable tends to be long.
 * If we need to access it before the env is relocated, then we need
 * to use our own stack buffer. gd->env_buf will be too small.
 *
 * @param buf temporary buffer pointer MTDPARTS_LEN long
 * @return mtdparts variable string, NULL if not found
 */
static const char *env_get_mtdparts(const char *str, char *buf)
{
	if (gd->flags & GD_FLG_ENV_READY)
		return env_get(str);
	if (env_get_f(str, buf, MTDPARTS_LEN) != -1)
		return buf;

	return NULL;
}

/**
 * update the variables "mtdids" and "mtdparts" with content of mtdparts_<dev>
 */
static void board_get_mtdparts(const char *dev,
			       char *mtdids,
			       char *mtdparts)
{
	char env_name[32] = "mtdparts_";
	char tmp_mtdparts[MTDPARTS_LEN];
	const char *tmp;

	/* name of env variable to read = mtdparts_<dev> */
	strcat(env_name, dev);
	tmp = env_get_mtdparts(env_name, tmp_mtdparts);
	if (tmp) {
		/* mtdids: "<dev>=<dev>, ...." */
		if (mtdids[0] != '\0')
			strcat(mtdids, ",");
		strcat(mtdids, dev);
		strcat(mtdids, "=");
		strcat(mtdids, dev);

		/* mtdparts: "mtdparts=<dev>:<mtdparts_<dev>>;..." */
		if (mtdparts[0] != '\0')
			strncat(mtdparts, ";", MTDPARTS_LEN);
		else
			strcat(mtdparts, "mtdparts=");
		strncat(mtdparts, dev, MTDPARTS_LEN);
		strncat(mtdparts, ":", MTDPARTS_LEN);
		strncat(mtdparts, tmp, MTDPARTS_LEN);
	}
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct udevice *dev;
	static char parts[3 * MTDPARTS_LEN + 1];
	static char ids[MTDIDS_LEN + 1];
	static bool mtd_initialized;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	memset(parts, 0, sizeof(parts));
	memset(ids, 0, sizeof(ids));

	/* probe all MTD devices */
	for (uclass_first_device(UCLASS_MTD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		pr_debug("mtd device = %s\n", dev->name);
	}

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_mtdparts("nor0", ids, parts);

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}
#endif

#ifdef CONFIG_SET_DFU_ALT_INFO
#define DFU_ALT_BUF_LEN SZ_1K

static void board_get_alt_info(const char *dev, char *buff)
{
	char var_name[32] = "dfu_alt_info_";
	int ret;

	ALLOC_CACHE_ALIGN_BUFFER(char, tmp_alt, DFU_ALT_BUF_LEN);

	/* name of env variable to read = dfu_alt_info_<dev> */
	strcat(var_name, dev);
	ret = env_get_f(var_name, tmp_alt, DFU_ALT_BUF_LEN);
	if (ret) {
		if (buff[0] != '\0')
			strcat(buff, "&");
		strncat(buff, tmp_alt, DFU_ALT_BUF_LEN);
	}
}

void set_dfu_alt_info(char *interface, char *devstr)
{
	struct udevice *dev;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	/* probe all MTD devices */
	mtd_probe_devices();

	board_get_alt_info("ram", buf);

	if (!uclass_get_device(UCLASS_MMC, 0, &dev))
		board_get_alt_info("mmc0", buf);

	if (!uclass_get_device(UCLASS_MMC, 1, &dev))
		board_get_alt_info("mmc1", buf);

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_alt_info("nor0", buf);

	env_set("dfu_alt_info", buf);
	puts("DFU alt info setting: done\n");
}
#endif

static void board_copro_image_process(ulong fw_image, size_t fw_size)
{
	int ret, id = 0; /* Copro id fixed to 0 as only one coproc on mp1 */

	if (!rproc_is_initialized())
		if (rproc_init()) {
			printf("Remote Processor %d initialization failed\n",
			       id);
			return;
		}

	ret = rproc_load(id, fw_image, fw_size);
	printf("Load Remote Processor %d with data@addr=0x%08lx %u bytes:%s\n",
	       id, fw_image, fw_size, ret ? " Failed!" : " Success!");

	if (!ret) {
		rproc_start(id);
		env_set("copro_state", "booted");
	}
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_COPRO, board_copro_image_process);
