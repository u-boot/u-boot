/*
 * U-boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include "bf533-stamp.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF533 Stamp board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

/* PF0 and PF1 are used to switch between the ethernet and flash:
 *         PF0  PF1
 *  flash:  0    0
 *  ether:  1    0
 */
void swap_to(int device_id)
{
	bfin_write_FIO_DIR(bfin_read_FIO_DIR() | PF1 | PF0);
	SSYNC();
	bfin_write_FIO_FLAG_C(PF1);
	if (device_id == ETHERNET)
		bfin_write_FIO_FLAG_S(PF0);
	else if (device_id == FLASH)
		bfin_write_FIO_FLAG_C(PF0);
	else
		printf("Unknown device to switch\n");
	SSYNC();
}

#if defined(CONFIG_MISC_INIT_R)
/* miscellaneous platform dependent initialisations */
int misc_init_r(void)
{
#ifdef CONFIG_STAMP_CF
	cf_ide_init();
#endif

	return 0;
}
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS

#define STATUS_LED_OFF 0
#define STATUS_LED_ON  1

static void stamp_led_set(int LED1, int LED2, int LED3)
{
	bfin_write_FIO_INEN(bfin_read_FIO_INEN() & ~(PF2 | PF3 | PF4));
	bfin_write_FIO_DIR(bfin_read_FIO_DIR() | (PF2 | PF3 | PF4));

	if (LED1 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF2;
	else
		*pFIO_FLAG_C = PF2;
	if (LED2 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF3;
	else
		*pFIO_FLAG_C = PF3;
	if (LED3 == STATUS_LED_OFF)
		*pFIO_FLAG_S = PF4;
	else
		*pFIO_FLAG_C = PF4;
	SSYNC();
}

void show_boot_progress(int status)
{
	switch (status) {
	case 1:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_OFF, STATUS_LED_ON);
		break;
	case 2:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_ON, STATUS_LED_OFF);
		break;
	case 3:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_ON, STATUS_LED_ON);
		break;
	case 4:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_OFF, STATUS_LED_OFF);
		break;
	case 5:
	case 6:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_OFF, STATUS_LED_ON);
		break;
	case 7:
	case 8:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_ON, STATUS_LED_OFF);
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		stamp_led_set(STATUS_LED_OFF, STATUS_LED_OFF, STATUS_LED_OFF);
		break;
	default:
		stamp_led_set(STATUS_LED_ON, STATUS_LED_ON, STATUS_LED_ON);
		break;
	}
}
#endif

#ifdef CONFIG_STATUS_LED
#include <status_led.h>

static void set_led(int pf, int state)
{
	switch (state) {
		case STATUS_LED_OFF:      bfin_write_FIO_FLAG_S(pf); break;
		case STATUS_LED_BLINKING: bfin_write_FIO_FLAG_T(pf); break;
		case STATUS_LED_ON:       bfin_write_FIO_FLAG_C(pf); break;
	}
}

static void set_leds(led_id_t mask, int state)
{
	if (mask & 0x1) set_led(PF2, state);
	if (mask & 0x2) set_led(PF3, state);
	if (mask & 0x4) set_led(PF4, state);
}

void __led_init(led_id_t mask, int state)
{
	bfin_write_FIO_INEN(bfin_read_FIO_INEN() & ~(PF2 | PF3 | PF4));
	bfin_write_FIO_DIR(bfin_read_FIO_DIR() | (PF2 | PF3 | PF4));
}

void __led_set(led_id_t mask, int state)
{
	set_leds(mask, state);
}

void __led_toggle(led_id_t mask)
{
	set_leds(mask, STATUS_LED_BLINKING);
}

#endif

#ifdef CONFIG_SMC91111
int board_eth_init(bd_t *bis)
{
	return smc91111_initialize(0, CONFIG_SMC91111_BASE);
}
#endif
