/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6x_pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |	       \
       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |	       \
       PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |	       \
       PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |	       \
       PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED	  |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

int dram_init(void)
{
       gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

       return 0;
}

iomux_v3_cfg_t uart1_pads[] = {
	MX6Q_PAD_SD3_DAT6__UART1_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6Q_PAD_SD3_DAT7__UART1_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t uart2_pads[] = {
       MX6Q_PAD_EIM_D26__UART2_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
       MX6Q_PAD_EIM_D27__UART2_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/* I2C1, SGTL5000 */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6Q_PAD_EIM_D21__GPIO_3_21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6Q_PAD_EIM_D28__GPIO_3_28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

/* I2C2 Camera, MIPI */
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO_4_12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO_4_13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

/* I2C3, J15 - RGB connector */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_GPIO_5__I2C3_SCL | PC,
		.gpio_mode = MX6Q_PAD_GPIO_5__GPIO_1_5 | PC,
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_GPIO_16__I2C3_SDA | PC,
		.gpio_mode = MX6Q_PAD_GPIO_16__GPIO_7_11 | PC,
		.gp = IMX_GPIO_NR(7, 11)
	}
};

iomux_v3_cfg_t usdhc3_pads[] = {
       MX6Q_PAD_SD3_CLK__USDHC3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_CMD__USDHC3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT5__GPIO_7_0    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

iomux_v3_cfg_t usdhc4_pads[] = {
       MX6Q_PAD_SD4_CLK__USDHC4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_CMD__USDHC4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT0__USDHC4_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT1__USDHC4_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT2__USDHC4_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT3__USDHC4_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_NANDF_D6__GPIO_2_6    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

iomux_v3_cfg_t enet_pads1[] = {
	MX6Q_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TXC__ENET_RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TD0__ENET_RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TD1__ENET_RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TD2__ENET_RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TD3__ENET_RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* pin 35 - 1 (PHY_AD2) on reset */
	MX6Q_PAD_RGMII_RXC__GPIO_6_30		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 32 - 1 - (MODE0) all */
	MX6Q_PAD_RGMII_RD0__GPIO_6_25		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 31 - 1 - (MODE1) all */
	MX6Q_PAD_RGMII_RD1__GPIO_6_27		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 28 - 1 - (MODE2) all */
	MX6Q_PAD_RGMII_RD2__GPIO_6_28		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 27 - 1 - (MODE3) all */
	MX6Q_PAD_RGMII_RD3__GPIO_6_29		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
	MX6Q_PAD_RGMII_RX_CTL__GPIO_6_24	| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 42 PHY nRST */
	MX6Q_PAD_EIM_D23__GPIO_3_23		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t enet_pads2[] = {
	MX6Q_PAD_RGMII_RXC__ENET_RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_RD0__ENET_RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_RD1__ENET_RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_RD2__ENET_RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_RD3__ENET_RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6Q_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

/* Button assignments for J14 */
static iomux_v3_cfg_t button_pads[] = {
	/* Menu */
	MX6Q_PAD_NANDF_D1__GPIO_2_1	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
	/* Back */
	MX6Q_PAD_NANDF_D2__GPIO_2_2	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
	/* Labelled Search (mapped to Power under Android) */
	MX6Q_PAD_NANDF_D3__GPIO_2_3	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
	/* Home */
	MX6Q_PAD_NANDF_D4__GPIO_2_4	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
	/* Volume Down */
	MX6Q_PAD_GPIO_19__GPIO_4_5	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
	/* Volume Up */
	MX6Q_PAD_GPIO_18__GPIO_7_13	| MUX_PAD_CTRL(BUTTON_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);
	gpio_direction_output(IMX_GPIO_NR(6, 30), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 27), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1);
	imx_iomux_v3_setup_multiple_pads(enet_pads1, ARRAY_SIZE(enet_pads1));
	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);

	/* Need delay 10ms according to KSZ9021 spec */
	udelay(1000 * 10);
	gpio_set_value(IMX_GPIO_NR(3, 23), 1);

	imx_iomux_v3_setup_multiple_pads(enet_pads2, ARRAY_SIZE(enet_pads2));
}

iomux_v3_cfg_t usb_pads[] = {
	MX6Q_PAD_GPIO_17__GPIO_7_12 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
       imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));

	/* Reset USB hub */
	gpio_direction_output(IMX_GPIO_NR(7, 12), 0);
	mdelay(2);
	gpio_set_value(IMX_GPIO_NR(7, 12), 1);

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[2] = {
       {USDHC3_BASE_ADDR},
       {USDHC4_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
       struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
       int ret;

       if (cfg->esdhc_base == USDHC3_BASE_ADDR) {
		gpio_direction_input(IMX_GPIO_NR(7, 0));
		ret = !gpio_get_value(IMX_GPIO_NR(7, 0));
       } else {
		gpio_direction_input(IMX_GPIO_NR(2, 6));
		ret = !gpio_get_value(IMX_GPIO_NR(2, 6));
       }

       return ret;
}

int board_mmc_init(bd_t *bis)
{
       s32 status = 0;
       u32 index = 0;

       for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
	       switch (index) {
	       case 0:
		       imx_iomux_v3_setup_multiple_pads(
			       usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
		       break;
	       case 1:
		       imx_iomux_v3_setup_multiple_pads(
			       usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
		       break;
	       default:
		       printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       index + 1, CONFIG_SYS_FSL_USDHC_NUM);
		       return status;
	       }

	       status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
       }

       return status;
}
#endif

u32 get_board_rev(void)
{
	return 0x63000 ;
}

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t ecspi1_pads[] = {
	/* SS1 */
	MX6Q_PAD_EIM_D19__GPIO_3_19   | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6Q_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6Q_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6Q_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

void setup_spi(void)
{
	gpio_direction_output(CONFIG_SF_DEFAULT_CS, 1);
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
					 ARRAY_SIZE(ecspi1_pads));
}
#endif

int board_phy_config(struct phy_device *phydev)
{
	/* min rx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0x0);
	/* min tx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0x0);
	/* max rx/tx clock delay, min rx/tx control */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0xf0f0);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int ret;

	setup_iomux_enet();

	ret = cpu_eth_init(bis);
	if (ret)
		printf("FEC MXC: %s:failed\n", __func__);

	return 0;
}

static void setup_buttons(void)
{
	imx_iomux_v3_setup_multiple_pads(button_pads,
					 ARRAY_SIZE(button_pads));
}

#ifdef CONFIG_CMD_SATA

int setup_sata(void)
{
	struct iomuxc_base_regs *const iomuxc_regs
		= (struct iomuxc_base_regs *) IOMUXC_BASE_ADDR;
	int ret = enable_sata_clock();
	if (ret)
		return ret;

	clrsetbits_le32(&iomuxc_regs->gpr[13],
			IOMUXC_GPR13_SATA_MASK,
			IOMUXC_GPR13_SATA_PHY_8_RXEQ_3P0DB
			|IOMUXC_GPR13_SATA_PHY_7_SATA2M
			|IOMUXC_GPR13_SATA_SPEED_3G
			|(3<<IOMUXC_GPR13_SATA_PHY_6_SHIFT)
			|IOMUXC_GPR13_SATA_SATA_PHY_5_SS_DISABLED
			|IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_9_16
			|IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P00_DB
			|IOMUXC_GPR13_SATA_PHY_2_TX_1P104V
			|IOMUXC_GPR13_SATA_PHY_1_SLOW);

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_buttons();

	return 0;
}

int board_init(void)
{
       /* address of boot parameters */
       gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

       return 0;
}

int checkboard(void)
{
       puts("Board: MX6Q-Sabre Lite\n");

       return 0;
}

struct button_key {
	char const	*name;
	unsigned	gpnum;
	char		ident;
};

static struct button_key const buttons[] = {
	{"back",	IMX_GPIO_NR(2, 2),	'B'},
	{"home",	IMX_GPIO_NR(2, 4),	'H'},
	{"menu",	IMX_GPIO_NR(2, 1),	'M'},
	{"search",	IMX_GPIO_NR(2, 3),	'S'},
	{"volup",	IMX_GPIO_NR(7, 13),	'V'},
	{"voldown",	IMX_GPIO_NR(4, 5),	'v'},
};

/*
 * generate a null-terminated string containing the buttons pressed
 * returns number of keys pressed
 */
static int read_keys(char *buf)
{
	int i, numpressed = 0;
	for (i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (!gpio_get_value(buttons[i].gpnum))
			buf[numpressed++] = buttons[i].ident;
	}
	buf[numpressed] = '\0';
	return numpressed;
}

static int do_kbd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char envvalue[ARRAY_SIZE(buttons)+1];
	int numpressed = read_keys(envvalue);
	setenv("keybd", envvalue);
	return numpressed == 0;
}

U_BOOT_CMD(
	kbd, 1, 1, do_kbd,
	"Tests for keypresses, sets 'keybd' environment variable",
	"Returns 0 (true) to shell if key is pressed."
);

#ifdef CONFIG_PREBOOT
static char const kbd_magic_prefix[] = "key_magic";
static char const kbd_command_prefix[] = "key_cmd";

static void preboot_keys(void)
{
	int numpressed;
	char keypress[ARRAY_SIZE(buttons)+1];
	numpressed = read_keys(keypress);
	if (numpressed) {
		char *kbd_magic_keys = getenv("magic_keys");
		char *suffix;
		/*
		 * loop over all magic keys
		 */
		for (suffix = kbd_magic_keys; *suffix; ++suffix) {
			char *keys;
			char magic[sizeof(kbd_magic_prefix) + 1];
			sprintf(magic, "%s%c", kbd_magic_prefix, *suffix);
			keys = getenv(magic);
			if (keys) {
				if (!strcmp(keys, keypress))
					break;
			}
		}
		if (*suffix) {
			char cmd_name[sizeof(kbd_command_prefix) + 1];
			char *cmd;
			sprintf(cmd_name, "%s%c", kbd_command_prefix, *suffix);
			cmd = getenv(cmd_name);
			if (cmd) {
				setenv("preboot", cmd);
				return;
			}
		}
	}
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_PREBOOT
	preboot_keys();
#endif

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}
