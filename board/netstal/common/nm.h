/*
 *(C) Copyright 2005-2007 Netstal Maschinen AG
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

extern void hcu_led_set(u32 value);
extern u32 get_serial_number(void);
extern u32 hcu_get_slot(void);
extern int board_with_pci(void);
extern void nm_show_print(int generation, int index, int hw_capabilities);
extern void set_params_for_sw_install(int install_requested, char *board_name );
extern void common_misc_init_r(void);

enum {
	/* HW_GENERATION_HCU1 is no longer supported */
	HW_GENERATION_HCU2  = 0x10,
	HW_GENERATION_HCU3  = 0x10,
	HW_GENERATION_HCU4  = 0x20,
	HW_GENERATION_HCU5  = 0x30,
	HW_GENERATION_MCU   = 0x08,
	HW_GENERATION_MCU20 = 0x0a,
	HW_GENERATION_MCU25 = 0x09,
};
