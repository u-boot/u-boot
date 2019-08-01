// SPDX-License-Identifier: GPL-2.0+
/*
 * DHCOM DH-iMX6 PDK board support
 *
 * Copyright (C) 2017 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <ahci.h>
#include <dwc_ahsata.h>
#include <env.h>
#include <environment.h>
#include <errno.h>
#include <fsl_esdhc_imx.h>
#include <fuse.h>
#include <i2c_eeprom.h>
#include <miiphy.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <usb.h>
#include <usb/ehci-ci.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

#ifdef CONFIG_FEC_MXC
static void eth_phy_reset(void)
{
	/* Reset PHY */
	gpio_direction_output(IMX_GPIO_NR(5, 0) , 0);
	udelay(500);
	gpio_set_value(IMX_GPIO_NR(5, 0), 1);

	/* Enable VIO */
	gpio_direction_output(IMX_GPIO_NR(1, 7) , 0);

	/*
	 * KSZ9021 PHY needs at least 10 mSec after PHY reset
	 * is released to stabilize
	 */
	mdelay(10);
}

static int setup_fec_clock(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* set gpr1[21] to select anatop clock */
	clrsetbits_le32(&iomuxc_regs->gpr[1], 0x1 << 21, 0x1 << 21);

	return enable_fec_anatop_clock(0, ENET_50MHZ);
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;

	gpio_request(IMX_GPIO_NR(5, 0), "PHY-reset");
	gpio_request(IMX_GPIO_NR(1, 7), "VIO");

	setup_fec_clock();

	eth_phy_reset();

	bus = fec_get_miibus(base, -1);
	if (!bus)
		return -EINVAL;

	/* Scan PHY 0 */
	phydev = phy_find_by_mask(bus, 0xf, PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		printf("Ethernet PHY not found!\n");
		return -EINVAL;
	}

	return fec_probe(bis, -1, base, bus, phydev);
}
#endif

#ifdef CONFIG_USB_EHCI_MX6
static void setup_usb(void)
{
	/*
	 * Set daisy chain for otg_pin_id on MX6Q.
	 * For MX6DL, this bit is reserved.
	 */
	imx_iomux_set_gpr_register(1, 13, 1, 0);
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return USB_INIT_DEVICE;
}
#endif

static int setup_dhcom_mac_from_fuse(void)
{
	struct udevice *dev;
	ofnode eeprom;
	unsigned char enetaddr[6];
	int ret;

	ret = eth_env_get_enetaddr("ethaddr", enetaddr);
	if (ret)	/* ethaddr is already set */
		return 0;

	imx_get_mac_from_fuse(0, enetaddr);

	if (is_valid_ethaddr(enetaddr)) {
		eth_env_set_enetaddr("ethaddr", enetaddr);
		return 0;
	}

	eeprom = ofnode_path("/soc/aips-bus@2100000/i2c@21a8000/eeprom@50");
	if (!ofnode_valid(eeprom)) {
		printf("Invalid hardware path to EEPROM!\n");
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, eeprom, &dev);
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

int board_early_init_f(void)
{
#ifdef CONFIG_USB_EHCI_MX6
	setup_usb();
#endif

	return 0;
}

int board_init(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* Enable eim_slow clocks */
	setbits_le32(&mxc_ccm->CCGR6, 0x1 << MXC_CCM_CCGR6_EMI_SLOW_OFFSET);

	setup_dhcom_mac_from_fuse();

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"sd3",	 MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

#define HW_CODE_BIT_0	IMX_GPIO_NR(2, 19)
#define HW_CODE_BIT_1	IMX_GPIO_NR(6, 6)
#define HW_CODE_BIT_2	IMX_GPIO_NR(2, 16)

static int board_get_hwcode(void)
{
	int hw_code;

	gpio_request(HW_CODE_BIT_0, "HW-code-bit-0");
	gpio_request(HW_CODE_BIT_1, "HW-code-bit-1");
	gpio_request(HW_CODE_BIT_2, "HW-code-bit-2");

	gpio_direction_input(HW_CODE_BIT_0);
	gpio_direction_input(HW_CODE_BIT_1);
	gpio_direction_input(HW_CODE_BIT_2);

	/* HW 100 + HW 200 = 00b; HW 300 = 01b */
	hw_code = ((gpio_get_value(HW_CODE_BIT_2) << 2) |
		   (gpio_get_value(HW_CODE_BIT_1) << 1) |
		    gpio_get_value(HW_CODE_BIT_0)) + 2;

	return hw_code;
}

int board_late_init(void)
{
	u32 hw_code;
	char buf[16];

	hw_code = board_get_hwcode();

	switch (get_cpu_type()) {
	case MXC_CPU_MX6SOLO:
		snprintf(buf, sizeof(buf), "imx6s-dhcom%1d", hw_code);
		break;
	case MXC_CPU_MX6DL:
		snprintf(buf, sizeof(buf), "imx6dl-dhcom%1d", hw_code);
		break;
	case MXC_CPU_MX6D:
		snprintf(buf, sizeof(buf), "imx6d-dhcom%1d", hw_code);
		break;
	case MXC_CPU_MX6Q:
		snprintf(buf, sizeof(buf), "imx6q-dhcom%1d", hw_code);
		break;
	default:
		snprintf(buf, sizeof(buf), "UNKNOWN%1d", hw_code);
		break;
	}

	env_set("dhcom", buf);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: DHCOM i.MX6\n");
	return 0;
}

#ifdef CONFIG_MULTI_DTB_FIT
int board_fit_config_name_match(const char *name)
{
	if (is_mx6dq()) {
		if (!strcmp(name, "imx6q-dhcom-pdk2"))
			return 0;
	} else if (is_mx6sdl()) {
		if (!strcmp(name, "imx6dl-dhcom-pdk2"))
			return 0;
	}

	return -1;
}
#endif
