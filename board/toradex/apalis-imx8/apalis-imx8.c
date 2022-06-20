// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Toradex
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <env.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <linux/bitops.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define TDX_USER_FUSE_BLOCK1_A	276
#define TDX_USER_FUSE_BLOCK1_B	277
#define TDX_USER_FUSE_BLOCK2_A	278
#define TDX_USER_FUSE_BLOCK2_B	279

static iomux_cfg_t uart1_pads[] = {
	SC_P_UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

struct tdx_user_fuses {
	u16 pid4;
	u16 vers;
	u8 ramid;
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static uint32_t do_get_tdx_user_fuse(int a, int b)
{
	sc_err_t sciErr;
	u32 val_a = 0;
	u32 val_b = 0;

	sciErr = sc_misc_otp_fuse_read(-1, a, &val_a);
	if (sciErr != SC_ERR_NONE) {
		printf("Error reading out user fuse %d\n", a);
		return 0;
	}

	sciErr = sc_misc_otp_fuse_read(-1, b, &val_b);
	if (sciErr != SC_ERR_NONE) {
		printf("Error reading out user fuse %d\n", b);
		return 0;
	}

	return ((val_a & 0xffff) << 16) | (val_b & 0xffff);
}

static void get_tdx_user_fuse(struct tdx_user_fuses *tdxuserfuse)
{
	u32 fuse_block;

	fuse_block = do_get_tdx_user_fuse(TDX_USER_FUSE_BLOCK2_A,
					  TDX_USER_FUSE_BLOCK2_B);

	/*
	 * Fuse block 2 acts as a backup area, if this reads 0 we want to
	 * use fuse block 1
	 */
	if (fuse_block == 0)
		fuse_block = do_get_tdx_user_fuse(TDX_USER_FUSE_BLOCK1_A,
						  TDX_USER_FUSE_BLOCK1_B);

	tdxuserfuse->pid4 = (fuse_block >> 18) & GENMASK(13, 0);
	tdxuserfuse->vers = (fuse_block >> 4) & GENMASK(13, 0);
	tdxuserfuse->ramid = fuse_block & GENMASK(3, 0);
}

void board_mem_get_layout(u64 *phys_sdram_1_start,
			  u64 *phys_sdram_1_size,
			  u64 *phys_sdram_2_start,
			  u64 *phys_sdram_2_size)
{
	u32 is_quadplus = 0, val = 0;
	struct tdx_user_fuses tdxramfuses;
	sc_err_t scierr = sc_misc_otp_fuse_read(-1, 6, &val);

	if (scierr == SC_ERR_NONE) {
		/* QP has one A72 core disabled */
		is_quadplus = ((val >> 4) & 0x3) != 0x0;
	}

	get_tdx_user_fuse(&tdxramfuses);

	*phys_sdram_1_start = PHYS_SDRAM_1;
	*phys_sdram_1_size = PHYS_SDRAM_1_SIZE;
	*phys_sdram_2_start = PHYS_SDRAM_2;

	switch (tdxramfuses.ramid) {
	case 1:
		*phys_sdram_2_size = SZ_2G;
		break;
	case 2:
		*phys_sdram_2_size = 0x0UL;
		break;
	case 3:
		*phys_sdram_2_size = SZ_2G;
		break;
	case 4:
		*phys_sdram_2_size = SZ_4G + SZ_2G;
		break;
	default:
		if (is_quadplus)
			/* Our QP based SKUs only have 2 GB RAM (PHYS_SDRAM_1_SIZE) */
			*phys_sdram_2_size = 0x0UL;
		else
			*phys_sdram_2_size = PHYS_SDRAM_2_SIZE;
		break;
	}
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate = SC_80MHZ;
	sc_err_t err = 0;

	/* Set UART1 clock root to 80 MHz and enable it */
	err = sc_pm_setup_uart(SC_R_UART_1, rate);
	if (err != SC_ERR_NONE)
		return 0;

	setup_iomux_uart();

	return 0;
}

#if CONFIG_IS_ENABLED(DM_GPIO)
static void board_gpio_init(void)
{
	/* TODO */
}
#else
static inline void board_gpio_init(void) {}
#endif

#if IS_ENABLED(CONFIG_FEC_MXC)
#include <miiphy.h>

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

int checkboard(void)
{
	puts("Model: Toradex Apalis iMX8\n");

	build_info();
	print_bootinfo();

	return 0;
}

int board_init(void)
{
	board_gpio_init();

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	/* TODO */
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
/* TODO move to common */
	env_set("board_name", "Apalis iMX8QM");
	env_set("board_rev", "v1.0");
#endif

	return 0;
}
