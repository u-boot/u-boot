/*
 *(C) Copyright 2005-2008 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include  <common.h>
#include  <ppc4xx.h>
#include  <asm/processor.h>
#include  <asm/io.h>
#include  <asm-ppc/u-boot.h>
#include  "../common/nm.h"

DECLARE_GLOBAL_DATA_PTR;

#define HCU_MACH_VERSIONS_REGISTER	(0x7C000000 + 0xF00000)
#define HCU_SLOT_ADDRESS		(0x7C000000 + 0x400000)
#define HCU_DIGITAL_IO_REGISTER		(0x7C000000 + 0x500000)
#define HCU_SW_INSTALL_REQUESTED	0x10

/*
 * This function is run very early, out of flash, and before devices are
 * initialized. It is called by lib_ppc/board.c:board_init_f by virtue
 * of being in the init_sequence array.
 *
 * The SDRAM has been initialized already -- start.S:start called
 * init.S:init_sdram early on -- but it is not yet being used for
 * anything, not even stack. So be careful.
 */

/* Attention: If you want 1 microsecs times from the external oscillator
 * 0x00004051 is okay for u-boot/linux, but different from old vxworks values
 * 0x00804051 causes problems with u-boot and linux!
 */
#define CPC0_CR0_VALUE	0x0030103c
#define CPC0_CR1_VALUE	0x00004051

int board_early_init_f (void)
{
	/*
	 * Interrupt controller setup for the HCU4 board.
	 * Note: IRQ 0-15  405GP internally generated; high; level sensitive
	 *       IRQ 16    405GP internally generated; low; level sensitive
	 *      IRQ 17-24 RESERVED/UNUSED
	 *      IRQ 31 (EXT IRQ 6) (unused)
	 */
	mtdcr(uicsr, 0xFFFFFFFF); /* clear all ints */
	mtdcr(uicer, 0x00000000); /* disable all ints */
	mtdcr(uiccr, 0x00000000); /* set all to be non-critical */
	mtdcr(uicpr, 0xFFFFE000); /* set int polarities */
	mtdcr(uictr, 0x00000000); /* set int trigger levels */
	mtdcr(uicsr, 0xFFFFFFFF); /* clear all ints */

	mtdcr(CPC0_CR1, CPC0_CR1_VALUE);
	mtdcr(CPC0_ECR, 0x60606000);
	mtdcr(CPC0_EIRR, 0x7C000000);

	return 0;
}

#ifdef CONFIG_BOARD_PRE_INIT
int board_pre_init (void)
{
	return board_early_init_f ();
}
#endif

int sys_install_requested(void)
{
	u16 ioValue = in_be16((u16 *)HCU_DIGITAL_IO_REGISTER);
	return (ioValue & HCU_SW_INSTALL_REQUESTED) != 0;
}

int checkboard (void)
{
	u16 boardVersReg = in_be16((u16 *)HCU_MACH_VERSIONS_REGISTER);
	u16 generation = boardVersReg & 0xf0;
	u16 index      = boardVersReg & 0x0f;

	/* Cannot be done in board_early_init */
	mtdcr(cntrl0,  CPC0_CR0_VALUE);

	/* Force /RTS to active. The board it not wired quite
	 *  correctly to use cts/rtc flow control, so just force the
	 *  /RST active and forget about it.
	 */
	writeb (readb (0xef600404) | 0x03, 0xef600404);
	nm_show_print(generation, index, 0);

	return 0;
}

u32 hcu_led_get(void)
{
	return (~(in_be32((u32 *)GPIO0_OR)) >> 23) & 0xff;
}

/*
 * hcu_led_set  value to be placed into the LEDs (max 6 bit)
 */
void hcu_led_set(u32 value)
{
	u32   tmp = ~value;

	tmp = (tmp << 23) | 0x7FFFFF;
	out_be32((u32 *)GPIO0_OR, tmp);
}

/*
 * sdram_init - Dummy implementation for start.S, spd_sdram  or initdram
 *		used for HCUx
 */
void sdram_init(void)
{
	return;
}

/*
 * hcu_get_slot
 */
u32 hcu_get_slot(void)
{
	u16 slot = in_be16((u16 *)HCU_SLOT_ADDRESS);
	return slot & 0x7f;
}

/*
 * get_serial_number
 */
u32 get_serial_number(void)
{
	u32 serial = in_be32((u32 *)CFG_FLASH_BASE);

	if (serial == 0xffffffff)
		return 0;

	return serial;
}


/*
 * misc_init_r.
 */

int misc_init_r(void)
{
	common_misc_init_r();
	set_params_for_sw_install( sys_install_requested(), "hcu4" );
	return 0;
}

long int initdram(int board_type)
{
	long dram_size = 0;
	u16 boardVersReg = in_be16((u16 *)HCU_MACH_VERSIONS_REGISTER);
	u16 generation = boardVersReg & 0xf0;
	u16 index      = boardVersReg & 0x0f;

	if (generation == HW_GENERATION_HCU3 && index < 0xf)
		dram_size = 32 << 20;	/* 32 MB - RAM */
	else
		dram_size = 64 << 20;	/* 64 MB - RAM */
	init_ppc405_sdram(dram_size);

#ifdef DEBUG
	show_sdram_registers();
#endif

	return dram_size;
}

#if defined(CONFIG_POST)
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}
#endif /* CONFIG_POST */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

/*
 * Hardcoded flash setup:
 * Flash 0 is a non-CFI AMD AM29F040 flash, 8 bit flash / 8 bit bus.
 */
ulong board_flash_get_legacy (ulong base, int banknum, flash_info_t * info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = 1;
		info->chipwidth = 1;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else
		return 0;
}
