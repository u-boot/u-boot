/*
 * (C) Copyright 2004
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __KUP_H
#define __KUP_H

#define PA_8	0x0080
#define PA_9	0x0040
#define PA_10	0x0020
#define PA_11	0x0010
#define PA_12	0x0008

#define PB_14	0x00020000
#define PB_15	0x00010000
#define PB_16	0x00008000
#define PB_17	0x00004000

#define PC_4	0x0800
#define PC_5	0x0400
#define PC_9	0x0040

#define PA_RS485	PA_11	/* SCC1: 0=RS232 1=RS485 */
#define PA_LED_YELLOW	PA_8
#define PA_RESET_IO_01	PA_9	/* Reset left IO */
#define PA_RESET_IO_02	PA_10	/* Reset right IO */
#define PB_PROG_IO_01	PB_15	/* Program left IO */
#define PB_PROG_IO_02	PB_16	/* Program right IO */
#define BP_USB_VCC	PB_14	/* VCC for USB devices 0=vcc on, 1=vcc off */
#define PB_LCD_PWM	PB_17	/* PB 17 */
#define PC_SWITCH1	PC_9	/* Reboot switch */


extern void poweron_key(void);
extern void load_sernum_ethaddr(void);

#endif	/* __KUP_H */
