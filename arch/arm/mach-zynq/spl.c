/*
 * (C) Copyright 2014 Xilinx, Inc. Michal Simek
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong dummy)
{
	ps7_init();

	arch_cpu_init();
	/*
	 * The debug UART can be used from this point:
	 * debug_uart_init();
	 * printch('x');
	 */
}

#ifdef CONFIG_SPL_BOARD_INIT
void spl_board_init(void)
{
	preloader_console_init();
	board_init();
}
#endif

u32 spl_boot_device(void)
{
	u32 mode;

	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
#ifdef CONFIG_SPL_SPI_SUPPORT
	case ZYNQ_BM_QSPI:
		puts("qspi boot\n");
		mode = BOOT_DEVICE_SPI;
		break;
#endif
	case ZYNQ_BM_NAND:
		mode = BOOT_DEVICE_NAND;
		break;
	case ZYNQ_BM_NOR:
		mode = BOOT_DEVICE_NOR;
		break;
#ifdef CONFIG_SPL_MMC_SUPPORT
	case ZYNQ_BM_SD:
		puts("mmc boot\n");
		mode = BOOT_DEVICE_MMC1;
		break;
#endif
	case ZYNQ_BM_JTAG:
		mode = BOOT_DEVICE_RAM;
		break;
	default:
		puts("Unsupported boot mode selected\n");
		hang();
	}

	return mode;
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(void)
{
	return MMCSD_MODE_FS;
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* boot linux */
	return 0;
}
#endif

__weak void ps7_init(void)
{
	/*
	 * This function is overridden by the one in
	 * board/xilinx/zynq/(platform)/ps7_init_gpl.c, if it exists.
	 */
}
