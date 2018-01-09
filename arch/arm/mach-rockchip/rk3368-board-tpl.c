/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/bootrom.h>
#include <asm/arch/cru_rk3368.h>
#include <asm/arch/grf_rk3368.h>
#include <asm/arch/hardware.h>
#include <asm/arch/timer.h>
#include <syscon.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The SPL (and also the full U-Boot stage on the RK3368) will run in
 * secure mode (i.e. EL3) and an ATF will eventually be booted before
 * starting up the operating system... so we can initialize the SGRF
 * here and rely on the ATF installing the final (secure) policy
 * later.
 */
static inline uintptr_t sgrf_soc_con_addr(unsigned no)
{
	const uintptr_t SGRF_BASE =
		(uintptr_t)syscon_get_first_range(ROCKCHIP_SYSCON_SGRF);

	return SGRF_BASE + sizeof(u32) * no;
}

static inline uintptr_t sgrf_busdmac_addr(unsigned no)
{
	const uintptr_t SGRF_BASE =
		(uintptr_t)syscon_get_first_range(ROCKCHIP_SYSCON_SGRF);
	const uintptr_t SGRF_BUSDMAC_OFFSET = 0x100;
	const uintptr_t SGRF_BUSDMAC_BASE = SGRF_BASE + SGRF_BUSDMAC_OFFSET;

	return SGRF_BUSDMAC_BASE + sizeof(u32) * no;
}

static void sgrf_init(void)
{
	struct rk3368_cru * const cru =
		(struct rk3368_cru * const)rockchip_get_cru();
	const u16 SGRF_SOC_CON_SEC = GENMASK(15, 0);
	const u16 SGRF_BUSDMAC_CON0_SEC = BIT(2);
	const u16 SGRF_BUSDMAC_CON1_SEC = GENMASK(15, 12);

	/* Set all configurable IP to 'non secure'-mode */
	rk_setreg(sgrf_soc_con_addr(5), SGRF_SOC_CON_SEC);
	rk_setreg(sgrf_soc_con_addr(6), SGRF_SOC_CON_SEC);
	rk_setreg(sgrf_soc_con_addr(7), SGRF_SOC_CON_SEC);

	/*
	 * From rockchip-uboot/arch/arm/cpu/armv8/rk33xx/cpu.c
	 * Original comment: "ddr space set no secure mode"
	 */
	rk_clrreg(sgrf_soc_con_addr(8), SGRF_SOC_CON_SEC);
	rk_clrreg(sgrf_soc_con_addr(9), SGRF_SOC_CON_SEC);
	rk_clrreg(sgrf_soc_con_addr(10), SGRF_SOC_CON_SEC);

	/* Set 'secure dma' to 'non secure'-mode */
	rk_setreg(sgrf_busdmac_addr(0), SGRF_BUSDMAC_CON0_SEC);
	rk_setreg(sgrf_busdmac_addr(1), SGRF_BUSDMAC_CON1_SEC);

	dsb();  /* barrier */

	rk_setreg(&cru->softrst_con[1], DMA1_SRST_REQ);
	rk_setreg(&cru->softrst_con[4], DMA2_SRST_REQ);

	dsb();  /* barrier */
	udelay(10);

	rk_clrreg(&cru->softrst_con[1], DMA1_SRST_REQ);
	rk_clrreg(&cru->softrst_con[4], DMA2_SRST_REQ);
}

void board_debug_uart_init(void)
{
	/*
	 * N.B.: This is called before the device-model has been
	 *       initialised. For this reason, we can not access
	 *       the GRF address range using the syscon API.
	 */
	struct rk3368_grf * const grf =
		(struct rk3368_grf * const)0xff770000;

	enum {
		GPIO2D1_MASK            = GENMASK(3, 2),
		GPIO2D1_GPIO            = 0,
		GPIO2D1_UART0_SOUT      = (1 << 2),

		GPIO2D0_MASK            = GENMASK(1, 0),
		GPIO2D0_GPIO            = 0,
		GPIO2D0_UART0_SIN       = (1 << 0),
	};

#if defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff180000)
	/* Enable early UART0 on the RK3368 */
	rk_clrsetreg(&grf->gpio2d_iomux,
		     GPIO2D0_MASK, GPIO2D0_UART0_SIN);
	rk_clrsetreg(&grf->gpio2d_iomux,
		     GPIO2D1_MASK, GPIO2D1_UART0_SOUT);
#endif
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#define EARLY_UART
#ifdef EARLY_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
	printascii("U-Boot TPL board init\n");
#endif

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	/* Reset security, so we can use DMA in the MMC drivers */
	sgrf_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
}

void board_return_to_bootrom(void)
{
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOOTROM;
}
