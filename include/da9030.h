/*
 * (C) Copyright 2006 DENX Software Engineering
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* DA9030 register definitions */
#define CID			0x00
#define EVENT_A			0x01
#define EVENT_B			0x02
#define EVENT_C			0x03
#define STATUS			0x04
#define IRQ_MASK_A		0x05
#define IRQ_MASK_B		0x06
#define IRQ_MASK_C		0x07
#define SYS_CONTROL_A		0x08
#define SYS_CONTROL_B		0x09
#define FAULT_LOG		0x0A
#define LDO_10_11		0x10
#define LDO_15			0x11
#define LDO_14_16		0x12
#define LDO_18_19		0x13
#define LDO_17_SIMCP0		0x14
#define BUCK2_DVC1		0x15
#define BUCK2_DVC2		0x16
#define REG_CONTROL_1_17	0x17
#define REG_CONTROL_2_18	0x18
#define USBPUMP			0x19
#define SLEEP_CONTROL		0x1A
#define STARTUP_CONTROL		0x1B
#define LED1_CONTROL		0x20
#define LED2_CONTROL		0x21
#define LED3_CONTROL		0x22
#define LED4_CONTROL		0x23
#define LEDPC_CONTROL		0x24
#define WLED_CONTROL		0x25
#define MISC_CONTROLA		0x26
#define MISC_CONTROLB		0x27
#define CHARGE_CONTROL		0x28
#define CCTR_CONTROL		0x29
#define TCTR_CONTROL		0x2A
#define CHARGE_PULSE		0x2B

/* ... some missing ...*/

#define LDO1			0x90
#define LDO2_3			0x91
#define LDO4_5			0x92
#define LDO6_SIMCP		0x93
#define LDO7_8			0x94
#define LDO9_12			0x95
#define BUCK			0x96
#define REG_CONTROL_1_97	0x97
#define REG_CONTROL_2_98	0x98
#define REG_SLEEP_CONTROL1	0x99
#define REG_SLEEP_CONTROL2	0x9A
#define REG_SLEEP_CONTROL3	0x9B
#define ADC_MAN_CONTROL		0xA0
#define ADC_AUTO_CONTROL	0xA1
#define VBATMON			0xA2
#define VBATMONTXMON		0xA3
#define TBATHIGHP		0xA4
#define TBATHIGHN		0xA5
#define TBATLOW			0xA6
#define MAN_RES			0xB0
#define VBAT_RES		0xB1
#define VBATMIN_RES		0xB2
#define VBATMINTXON_RES		0xB3
#define ICHMAX_RES		0xB4
#define ICHMIN_RES		0xB5
#define ICHAVERAGE_RES		0xB6
#define VCHMAX_RES		0xB7
#define VCHMIN_RES		0xB8
#define TBAT_RES		0xB9
#define ADC_IN4_RES		0xBA

#define STATUS_ONKEY_N		0x1	/* current ONKEY_N value */
#define STATUS_PWREN1		(1<<1)	/* PWREN1 value */
#define STATUS_EXTON		(1<<2)	/* EXTON value */
#define STATUS_CHDET		(1<<3)	/* Charger detection status */
#define STATUS_TBAT		(1<<4)	/* Battery over/under temperature status */
#define STATUS_VBATMON		(1<<5)	/* VBATMON comparison status */
#define STATUS_VBATMONTXON	(1<<6)	/* VBATMONTXON comparison status */
#define STATUS_CHIOVER		(1<<7)	/* Charge overcurrent */

#define SYS_CONTROL_A_SLEEP_N_PIN_ENABLE	0x1
#define SYS_CONTROL_A_SHUT_DOWN			(1<<1)
#define SYS_CONTROL_A_HWRES_ENABLE		(1<<2)
#define SYS_CONTROL_A_WDOG_ACTION		(1<<3)
#define SYS_CONTROL_A_WATCHDOG			(1<<7)
