/*
 * K2G EVM : Board initialization
 *
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/ti-common/keystone_net.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/mmc_host_def.h>
#include <fdtdec.h>
#include <i2c.h>
#include <remoteproc.h>
#include "mux-k2g.h"
#include "../common/board_detect.h"

#define K2G_GP_AUDIO_CODEC_ADDRESS	0x1B

const unsigned int sysclk_array[MAX_SYSCLK] = {
	19200000,
	24000000,
	25000000,
	26000000,
};

unsigned int get_external_clk(u32 clk)
{
	unsigned int clk_freq;
	u8 sysclk_index = get_sysclk_index();

	switch (clk) {
	case sys_clk:
		clk_freq = sysclk_array[sysclk_index];
		break;
	case pa_clk:
		clk_freq = sysclk_array[sysclk_index];
		break;
	case tetris_clk:
		clk_freq = sysclk_array[sysclk_index];
		break;
	case ddr3a_clk:
		clk_freq = sysclk_array[sysclk_index];
		break;
	case uart_clk:
		clk_freq = sysclk_array[sysclk_index];
		break;
	default:
		clk_freq = 0;
		break;
	}

	return clk_freq;
}

static int arm_speeds[DEVSPEED_NUMSPDS] = {
	SPD400,
	SPD600,
	SPD800,
	SPD900,
	SPD1000,
	SPD900,
	SPD800,
	SPD600,
	SPD400,
	SPD200,
};

static int dev_speeds[DEVSPEED_NUMSPDS] = {
	SPD600,
	SPD800,
	SPD900,
	SPD1000,
	SPD900,
	SPD800,
	SPD600,
	SPD400,
};

static struct pll_init_data main_pll_config[MAX_SYSCLK][NUM_SPDS] = {
	[SYSCLK_19MHz] = {
		[SPD400]	= {MAIN_PLL, 125, 3, 2},
		[SPD600]	= {MAIN_PLL, 125, 2, 2},
		[SPD800]	= {MAIN_PLL, 250, 3, 2},
		[SPD900]	= {MAIN_PLL, 187, 2, 2},
		[SPD1000]	= {MAIN_PLL, 104, 1, 2},
	},
	[SYSCLK_24MHz] = {
		[SPD400]	= {MAIN_PLL, 100, 3, 2},
		[SPD600]	= {MAIN_PLL, 300, 6, 2},
		[SPD800]	= {MAIN_PLL, 200, 3, 2},
		[SPD900]	= {MAIN_PLL, 75, 1, 2},
		[SPD1000]	= {MAIN_PLL, 250, 3, 2},
	},
	[SYSCLK_25MHz] = {
		[SPD400]	= {MAIN_PLL, 32, 1, 2},
		[SPD600]	= {MAIN_PLL, 48, 1, 2},
		[SPD800]	= {MAIN_PLL, 64, 1, 2},
		[SPD900]	= {MAIN_PLL, 72, 1, 2},
		[SPD1000]	= {MAIN_PLL, 80, 1, 2},
	},
	[SYSCLK_26MHz] = {
		[SPD400]	= {MAIN_PLL, 400, 13, 2},
		[SPD600]	= {MAIN_PLL, 230, 5, 2},
		[SPD800]	= {MAIN_PLL, 123, 2, 2},
		[SPD900]	= {MAIN_PLL, 69, 1, 2},
		[SPD1000]	= {MAIN_PLL, 384, 5, 2},
	},
};

static struct pll_init_data tetris_pll_config[MAX_SYSCLK][NUM_SPDS] = {
	[SYSCLK_19MHz] = {
		[SPD200]	= {TETRIS_PLL, 625, 6, 10},
		[SPD400]	= {TETRIS_PLL, 125, 1, 6},
		[SPD600]	= {TETRIS_PLL, 125, 1, 4},
		[SPD800]	= {TETRIS_PLL, 333, 2, 4},
		[SPD900]	= {TETRIS_PLL, 187, 2, 2},
		[SPD1000]	= {TETRIS_PLL, 104, 1, 2},
	},
	[SYSCLK_24MHz] = {
		[SPD200]	= {TETRIS_PLL, 250, 3, 10},
		[SPD400]	= {TETRIS_PLL, 100, 1, 6},
		[SPD600]	= {TETRIS_PLL, 100, 1, 4},
		[SPD800]	= {TETRIS_PLL, 400, 3, 4},
		[SPD900]	= {TETRIS_PLL, 75, 1, 2},
		[SPD1000]	= {TETRIS_PLL, 250, 3, 2},
	},
	[SYSCLK_25MHz] = {
		[SPD200]	= {TETRIS_PLL, 80, 1, 10},
		[SPD400]	= {TETRIS_PLL, 96, 1, 6},
		[SPD600]	= {TETRIS_PLL, 96, 1, 4},
		[SPD800]	= {TETRIS_PLL, 128, 1, 4},
		[SPD900]	= {TETRIS_PLL, 72, 1, 2},
		[SPD1000]	= {TETRIS_PLL, 80, 1, 2},
	},
	[SYSCLK_26MHz] = {
		[SPD200]	= {TETRIS_PLL, 307, 4, 10},
		[SPD400]	= {TETRIS_PLL, 369, 4, 6},
		[SPD600]	= {TETRIS_PLL, 369, 4, 4},
		[SPD800]	= {TETRIS_PLL, 123, 1, 4},
		[SPD900]	= {TETRIS_PLL, 69, 1, 2},
		[SPD1000]	= {TETRIS_PLL, 384, 5, 2},
	},
};

static struct pll_init_data uart_pll_config[MAX_SYSCLK] = {
	[SYSCLK_19MHz] = {UART_PLL, 160, 1, 8},
	[SYSCLK_24MHz] = {UART_PLL, 128, 1, 8},
	[SYSCLK_25MHz] = {UART_PLL, 768, 5, 10},
	[SYSCLK_26MHz] = {UART_PLL, 384, 13, 2},
};

static struct pll_init_data nss_pll_config[MAX_SYSCLK] = {
	[SYSCLK_19MHz] = {NSS_PLL, 625, 6, 2},
	[SYSCLK_24MHz] = {NSS_PLL, 250, 3, 2},
	[SYSCLK_25MHz] = {NSS_PLL, 80, 1, 2},
	[SYSCLK_26MHz] = {NSS_PLL, 1000, 13, 2},
};

static struct pll_init_data ddr3_pll_config[MAX_SYSCLK] = {
	[SYSCLK_19MHz] = {DDR3A_PLL, 167, 1, 16},
	[SYSCLK_24MHz] = {DDR3A_PLL, 133, 1, 16},
	[SYSCLK_25MHz] = {DDR3A_PLL, 128, 1, 16},
	[SYSCLK_26MHz] = {DDR3A_PLL, 123, 1, 16},
};

struct pll_init_data *get_pll_init_data(int pll)
{
	int speed;
	struct pll_init_data *data = NULL;
	u8 sysclk_index = get_sysclk_index();

	switch (pll) {
	case MAIN_PLL:
		speed = get_max_dev_speed(dev_speeds);
		data = &main_pll_config[sysclk_index][speed];
		break;
	case TETRIS_PLL:
		speed = get_max_arm_speed(arm_speeds);
		data = &tetris_pll_config[sysclk_index][speed];
		break;
	case NSS_PLL:
		data = &nss_pll_config[sysclk_index];
		break;
	case UART_PLL:
		data = &uart_pll_config[sysclk_index];
		break;
	case DDR3_PLL:
		data = &ddr3_pll_config[sysclk_index];
		break;
	default:
		data = NULL;
	}

	return data;
}

s16 divn_val[16] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	if (psc_enable_module(KS2_LPSC_MMC)) {
		printf("%s module enabled failed\n", __func__);
		return -1;
	}

	if (board_is_k2g_gp())
		omap_mmc_init(0, 0, 0, -1, -1);

	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	bool eeprom_read = board_ti_was_eeprom_read();

	if (!strcmp(name, "keystone-k2g-generic") && !eeprom_read)
		return 0;
	else if (!strcmp(name, "keystone-k2g-evm") && board_ti_is("66AK2GGP"))
		return 0;
	else if (!strcmp(name, "keystone-k2g-ice") && board_ti_is("66AK2GIC"))
		return 0;
	else
		return -1;
}
#endif

#if defined(CONFIG_DTB_RESELECT)
static int k2g_alt_board_detect(void)
{
	int rc;

	rc = i2c_set_bus_num(1);
	if (rc)
		return rc;

	rc = i2c_probe(K2G_GP_AUDIO_CODEC_ADDRESS);
	if (rc)
		return rc;

	ti_i2c_eeprom_am_set("66AK2GGP", "1.0X");

	return 0;
}

static void k2g_reset_mux_config(void)
{
	/* Unlock the reset mux register */
	clrbits_le32(KS2_RSTMUX8, RSTMUX_LOCK8_MASK);

	/* Configure BOOTCFG_RSTMUX8 for WDT event to cause a device reset */
	clrsetbits_le32(KS2_RSTMUX8, RSTMUX_OMODE8_MASK,
			RSTMUX_OMODE8_DEV_RESET << RSTMUX_OMODE8_SHIFT);

	/* lock the reset mux register to prevent any spurious writes. */
	setbits_le32(KS2_RSTMUX8, RSTMUX_LOCK8_MASK);
}

int embedded_dtb_select(void)
{
	int rc;
	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
			CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc) {
		rc = k2g_alt_board_detect();
		if (rc) {
			printf("Unable to do board detection\n");
			return -1;
		}
	}

	fdtdec_setup();

	k2g_mux_config();

	k2g_reset_mux_config();

	if (board_is_k2g_gp()) {
		/* deassert FLASH_HOLD */
		clrbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_DIR_OFFSET,
			     BIT(9));
		setbits_le32(K2G_GPIO1_BANK2_BASE + K2G_GPIO_SETDATA_OFFSET,
			     BIT(9));
	}

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_TI_I2C_BOARD_DETECT)
	int rc;

	rc = ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS,
			CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	board_ti_set_ethaddr(1);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	if (board_is_k2g_gp())
		env_set("board_name", "66AK2GGP\0");
	else if (board_is_k2g_ice())
		env_set("board_name", "66AK2GIC\0");
#endif
	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	init_plls();

	k2g_mux_config();

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_init_keystone_plls(void)
{
	init_plls();
}
#endif

#ifdef CONFIG_DRIVER_TI_KEYSTONE_NET
struct eth_priv_t eth_priv_cfg[] = {
	{
		.int_name	= "K2G_EMAC",
		.rx_flow	= 0,
		.phy_addr	= 0,
		.slave_port	= 1,
		.sgmii_link_type = SGMII_LINK_MAC_PHY,
		.phy_if          = PHY_INTERFACE_MODE_RGMII,
	},
};

int get_num_eth_ports(void)
{
	return sizeof(eth_priv_cfg) / sizeof(struct eth_priv_t);
}
#endif

#ifdef CONFIG_TI_SECURE_DEVICE
void board_pmmc_image_process(ulong pmmc_image, size_t pmmc_size)
{
	int id = getenv_ulong("dev_pmmc", 10, 0);
	int ret;

	if (!rproc_is_initialized())
		rproc_init();

	ret = rproc_load(id, pmmc_image, pmmc_size);
	printf("Load Remote Processor %d with data@addr=0x%08lx %u bytes:%s\n",
	       id, pmmc_image, pmmc_size, ret ? " Failed!" : " Success!");

	if (!ret)
		rproc_start(id);
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_PMMC, board_pmmc_image_process);
#endif
