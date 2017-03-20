/*
 * (C) Copyright 2015 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/bootrom.h>
#include <asm/arch/pmu_rk3188.h>

DECLARE_GLOBAL_DATA_PTR;

/* track how often we were entered */
static int rk3188_num_entries __attribute__ ((section(".data")));

#define PMU_BASE	0x20004000
#define SPL_ENTRY	0x10080C00

static void jump_to_spl(void)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	struct rk3188_pmu * const pmu = (void *)PMU_BASE;
	image_entry_noargs_t tpl_entry =
		(image_entry_noargs_t)(unsigned long)SPL_ENTRY;

	/* Store the SAVE_SP_ADDR in a location shared with SPL. */
	writel(SAVE_SP_ADDR, &pmu->sys_reg[2]);
	tpl_entry();
}

void board_init_f(ulong dummy)
{
	/* Example code showing how to enable the debug UART on RK3188 */
#ifdef EARLY_UART
#include <asm/arch/grf_rk3188.h>
	/* Enable early UART on the RK3188 */
#define GRF_BASE	0x20008000
	struct rk3188_grf * const grf = (void *)GRF_BASE;

	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK << GPIO1B1_SHIFT |
		     GPIO1B0_MASK << GPIO1B0_SHIFT,
		     GPIO1B1_UART2_SOUT << GPIO1B1_SHIFT |
		     GPIO1B0_UART2_SIN << GPIO1B0_SHIFT);
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();

	printch('t');
	printch('p');
	printch('l');
	printch('-');
	printch(rk3188_num_entries + 1 + '0');
	printch('\n');
#endif

	rk3188_num_entries++;

	if (rk3188_num_entries == 1) {
		/*
		 * The original loader did some very basic integrity
		 * checking at this point, but the remaining few bytes
		 * could be used for any improvement making sense
		 * really early on.
		 */

		back_to_bootrom();
	} else {
		/*
		 * TPL part of the loader should now wait for us
		 * at offset 0xC00 in the sram. Should never return
		 * from there.
		 */
		jump_to_spl();
	}
}
