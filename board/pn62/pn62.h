/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
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

#ifndef _PN62_H_
#define _PN62_H_

/*
 * Definitions for the Intel Bridge 21554 or 21555.
 */
#define I2155X_VPD_ADDR 	0xe6
#define I2155X_VPD_DATA 	0xe8

#define I2155X_VPD_START	0x80
#define I2155X_VPD_SN_START	0x80
#define I2155X_VPD_SN_SIZE	0x10
#define I2155X_VPD_MAC0_START	0x90
#define I2155X_VPD_MAC1_START	0x96

#define I2155X_SCRAPAD_ADDR	0xa8
#define I2155X_SCRAPAD_MAX	8

#define I2155X_BAR2_BASE	0x98
#define I2155X_BAR3_BASE	0x9c
#define I2155X_BAR4_BASE	0xa0

#define I2155X_BAR2_SETUP	0xb0
#define I2155X_BAR3_SETUP	0xb4
#define I2155X_BAR4_SETUP	0xb8

/*
 * Interrupt request numbers
 */
#define PN62_IRQ_HOST		0x0
#define PN62_IRQ_PLX9054	0x1
#define PN62_IRQ_ETH0		0x2
#define PN62_IRQ_ETH1		0x3
#define PN62_IRQ_COM1		0x4
#define PN62_IRQ_COM2		0x4

/*
 * Miscellaneous definitons.
 */
#define PN62_SMEM_DEFAULT	0x1f00000

/*
 * Definitions for boot protocol using Scratchpad registers.
 */
#define BOOT_DONE		0
#define BOOT_DONE_CLEAR  	  0x00dead00
#define BOOT_DONE_ERROR  	  0xbad0dead
#define BOOT_DONE_U_BOOT 	  0x12345678
#define BOOT_DONE_LINUX   	  0x87654321
#define BOOT_CMD 		1
#define BOOT_CMD_MOVE   	  0x1
#define BOOT_CMD_BOOT		  0x2
#define BOOT_DATA		2
#define BOOT_PROTO		3
#define BOOT_PROTO_READY	  0x23456789
#define BOOT_PROTO_CLEAR	  0x00000000
#define BOOT_STATUS		4

/*
 * LED Definitions:
 */
#define PN62_LED_BASE		0xff800300
#define PN62_LED_MAX		12

/*
 * LED0 - 7 mounted on top of board, D1 - D8
 * LED8 - 11 upper four LEDs on the front panel of the board.
 */
#define LED_0			0x00	/* OFF */
#define LED_1			0x01	/* ON */
#define LED_SLOW_CLOCK		0x02	/* SLOW 1Hz ish */
#define LED_nSLOW_CLOCK		0x03	/* inverse of above */
#define LED_WATCHDOG_OUT	0x06	/* Reset Watchdog level */
#define LED_WATCHDOG_CLOCK	0x07	/* clock to watchdog */

/*
 * LED's currently setup in AMD79C973 device as the following:
 * LED0 100Mbit
 * LED1 LNKSE
 * LED2 TX Activity
 * LED3 RX Activity
 */
#define LED_E0_LED0		0x08	/* Ethernet Port 0 LED 0 */
#define LED_E0_LED1		0x09	/* Ethernet Port 0 LED 1 */
#define LED_E0_LED2		0x0A	/* Ethernet Port 0 LED 2 */
#define LED_E0_LED3		0x0B	/* Ethernet Port 0 LED 3 */
#define LED_E1_LED0		0x0C	/* Ethernet Port 1 LED 0 */
#define LED_E1_LED1		0x0D	/* Ethernet Port 1 LED 1 */
#define LED_E1_LED2		0x0E	/* Ethernet Port 1 LED 2 */
#define LED_E1_LED3		0x0F	/* Ethernet Port 1 LED 3 */
#define LED_STROBE0		0x10	/* Processor Strobe 0 */
#define LED_STROBE1		0x11	/* Processor Strobe 1 */
#define LED_STROBE2		0x12	/* Processor Strobe 2 */
#define LED_STROBE3		0x13	/* Processor Strobe 3 */
#define LED_STROBE4		0x14	/* Processor Strobe 4 */
#define LED_STROBE5		0x15	/* Processor Strobe 5 */
#define LED_STROBE6		0x16	/* Processor Strobe 6 */
#define LED_STROBE7		0x17	/* Processor Strobe 7 */
#define LED_HOST_STROBE0	0x18	/* Host strobe 0 */
#define LED_HOST_STROBE1	0x19	/* Host strobe 1 */
#define LED_HOST_STROBE2	0x1A	/* Host strobe 2 */
#define LED_HOST_STROBE3	0x1B	/* Host strobe 3 */
#define LED_HOST_STROBE4	0x1C	/* Host strobe 4 */
#define LED_HOST_STROBE5	0x1D	/* Host strobe 5 */
#define LED_HOST_STROBE6	0x1E	/* Host strobe 6 */
#define LED_HOST_STROBE7	0x1F	/* Host strobe 7 */
#define LED_MPC_INT0		0x20	/* MPC8240 INT 0 */
#define LED_MPC_INT1		0x21	/* MPC8240 INT 1 */
#define	LED_MPC_INT2		0x22	/* MPC8240 INT 2 */
#define	LED_MPC_INT3		0x23	/* MPC8240 INT 3 */
#define	LED_MPC_INT4		0x24	/* MPC8240 INT 4 */
#define	LED_UART0_CS		0x25	/* UART 0 Chip Select */
#define	LED_UART1_CS		0x26	/* UART 1 Chip Select */
#define	LED_SRAM_CS		0x27	/* SRAM Chip Select */
#define	LED_SRAM_WR		0x28	/* SRAM WR Signal */
#define	LED_SRAM_RD		0x29	/* SRAM RD Signal */
#define	LED_MPC_RCS0		0x2A	/* MPC8240 RCS0 Signal */
#define	LED_S_PCI_FRAME		0x2B	/* Secondary PCI Frame Signal */
#define	LED_MPC_CS0		0x2C	/* MPC8240 CS0 Signal */
#define	LED_HOST_INT		0x2D	/* MPC8240 to Host Interrupt signal */
#define LED_LAST_FUNCTION	LED_HOST_INT	/* last function */

/*
 * Forward declarations
 */
int  i2155x_init         (void);
void i2155x_write_scrapad(int idx, u32 val);
u32  i2155x_read_scrapad (int idx);
void i2155x_set_bar_base (int bar, u32 addr);
int  i2155x_read_vpd     (int offset, int size, unsigned char *data);

int  am79c95x_init	 (void);

void set_led             (unsigned int number, unsigned int function);
void fatal_error	 (unsigned int error_code);
void show_startup_phase  (int phase);


#endif /* _PN62_H_ */
