/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

#ifndef _STATUS_LED_H_
#define	_STATUS_LED_H_

#ifdef CONFIG_STATUS_LED

#define STATUS_LED_OFF		0
#define STATUS_LED_BLINKING	1
#define STATUS_LED_ON		2

void status_led_tick (unsigned long timestamp);
void status_led_set  (int led, int state);

/*****  TQM8xxL  ********************************************************/
#if defined(CONFIG_TQM8xxL) && !defined(CONFIG_HMI10)
# define STATUS_LED_PAR		im_cpm.cp_pbpar
# define STATUS_LED_DIR		im_cpm.cp_pbdir
# define STATUS_LED_ODR		im_cpm.cp_pbodr
# define STATUS_LED_DAT		im_cpm.cp_pbdat

# define STATUS_LED_BIT		0x00000001
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  MVS v1  **********************************************************/
#elif (defined(CONFIG_MVS) && CONFIG_MVS < 2)
# define STATUS_LED_PAR		im_ioport.iop_pdpar
# define STATUS_LED_DIR		im_ioport.iop_pddir
# undef  STATUS_LED_ODR
# define STATUS_LED_DAT		im_ioport.iop_pddat

# define STATUS_LED_BIT		0x00000001
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  ETX_094  ********************************************************/
#elif defined(CONFIG_ETX094)

# define STATUS_LED_PAR		im_ioport.iop_pdpar
# define STATUS_LED_DIR		im_ioport.iop_pddir
# undef  STATUS_LED_ODR
# define STATUS_LED_DAT		im_ioport.iop_pddat

# define STATUS_LED_BIT		0x00000001
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  GEN860T  *********************************************************/
#elif defined(CONFIG_GEN860T)

# define STATUS_LED_PAR			im_ioport.iop_papar
# define STATUS_LED_DIR			im_ioport.iop_padir
# define STATUS_LED_ODR			im_ioport.iop_paodr
# define STATUS_LED_DAT			im_ioport.iop_padat

# define STATUS_LED_BIT			0x0800	/* Red LED 0 is on PA.4	*/
# define STATUS_LED_PERIOD		(CFG_HZ / 4)
# define STATUS_LED_STATE		STATUS_LED_OFF
# define STATUS_LED_BIT1		0x0400	/* Grn LED 1 is on PA.5	*/
# define STATUS_LED_PERIOD1		(CFG_HZ / 8)
# define STATUS_LED_STATE1		STATUS_LED_BLINKING
# define STATUS_LED_BIT2		0x0080	/* Red LED 2 is on PA.8	*/
# define STATUS_LED_PERIOD2		(CFG_HZ / 4)
# define STATUS_LED_STATE2		STATUS_LED_OFF
# define STATUS_LED_BIT3		0x0040	/* Grn LED 3 is on PA.9	*/
# define STATUS_LED_PERIOD3		(CFG_HZ / 4)
# define STATUS_LED_STATE3		STATUS_LED_OFF

# define STATUS_LED_ACTIVE		1	/* LED on for bit == 1	*/
# define STATUS_LED_BOOT		1	/* Boot status on LED 1	*/

/*****  IVMS8  **********************************************************/
#elif defined(CONFIG_IVMS8)

# define STATUS_LED_PAR		im_cpm.cp_pbpar
# define STATUS_LED_DIR		im_cpm.cp_pbdir
# define STATUS_LED_ODR		im_cpm.cp_pbodr
# define STATUS_LED_DAT		im_cpm.cp_pbdat

# define STATUS_LED_BIT		0x00000010	/* LED 0 is on PB.27	*/
# define STATUS_LED_PERIOD	(1 * CFG_HZ)
# define STATUS_LED_STATE	STATUS_LED_OFF
# define STATUS_LED_BIT1	0x00000020	/* LED 1 is on PB.26	*/
# define STATUS_LED_PERIOD1	(1 * CFG_HZ)
# define STATUS_LED_STATE1	STATUS_LED_OFF
/* IDE LED usable for other purposes, too */
# define STATUS_LED_BIT2	0x00000008	/* LED 2 is on PB.28	*/
# define STATUS_LED_PERIOD2	(1 * CFG_HZ)
# define STATUS_LED_STATE2	STATUS_LED_OFF

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_ILOCK_SWITCH	0x00800000	/* ILOCK switch in IRQ4	*/

# define STATUS_ILOCK_PERIOD	(CFG_HZ / 10)	/* about every 100 ms	*/

# define STATUS_LED_YELLOW	0
# define STATUS_LED_GREEN	1
# define STATUS_LED_BOOT	2		/* IDE LED used for boot status */

/*****  IVML24  *********************************************************/
#elif defined(CONFIG_IVML24)

# define STATUS_LED_PAR		im_cpm.cp_pbpar
# define STATUS_LED_DIR		im_cpm.cp_pbdir
# define STATUS_LED_ODR		im_cpm.cp_pbodr
# define STATUS_LED_DAT		im_cpm.cp_pbdat

# define STATUS_LED_BIT		0x00000010	/* LED 0 is on PB.27	*/
# define STATUS_LED_PERIOD	(1 * CFG_HZ)
# define STATUS_LED_STATE	STATUS_LED_OFF
# define STATUS_LED_BIT1	0x00000020	/* LED 1 is on PB.26	*/
# define STATUS_LED_PERIOD1	(1 * CFG_HZ)
# define STATUS_LED_STATE1	STATUS_LED_OFF
/* IDE LED usable for other purposes, too */
# define STATUS_LED_BIT2	0x00000008	/* LED 2 is on PB.28	*/
# define STATUS_LED_PERIOD2	(1 * CFG_HZ)
# define STATUS_LED_STATE2	STATUS_LED_OFF

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_ILOCK_SWITCH	0x00004000	/* ILOCK is on PB.17	*/

# define STATUS_ILOCK_PERIOD	(CFG_HZ / 10)	/* about every 100 ms	*/

# define STATUS_LED_YELLOW	0
# define STATUS_LED_GREEN	1
# define STATUS_LED_BOOT	2		/* IDE LED used for boot status */

/*****  LANTEC  *********************************************************/
#elif defined(CONFIG_LANTEC)

# define STATUS_LED_PAR		im_ioport.iop_pdpar
# define STATUS_LED_DIR		im_ioport.iop_pddir
# undef  STATUS_LED_ODR
# define STATUS_LED_DAT		im_ioport.iop_pddat

# if CONFIG_LATEC < 2
#  define STATUS_LED_BIT	0x1000
# else
#  define STATUS_LED_BIT	0x0800
# endif
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0 */

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  PCU E  and  CCM  ************************************************/
#elif (defined(CONFIG_PCU_E) || defined(CONFIG_CCM))

# define STATUS_LED_PAR		im_cpm.cp_pbpar
# define STATUS_LED_DIR		im_cpm.cp_pbdir
# define STATUS_LED_ODR		im_cpm.cp_pbodr
# define STATUS_LED_DAT		im_cpm.cp_pbdat

# define STATUS_LED_BIT		0x00010000	/* green LED is on PB.15 */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1 */

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  ICU862   ********************************************************/
#elif defined(CONFIG_ICU862)

# define STATUS_LED_PAR		im_ioport.iop_papar
# define STATUS_LED_DIR		im_ioport.iop_padir
# define STATUS_LED_ODR		im_ioport.iop_paodr
# define STATUS_LED_DAT		im_ioport.iop_padat

# define STATUS_LED_BIT		0x4000		/* LED 0 is on PA.1 */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING
# define STATUS_LED_BIT1	0x1000		/* LED 1 is on PA.3 */
# define STATUS_LED_PERIOD1	(CFG_HZ)
# define STATUS_LED_STATE1	STATUS_LED_OFF

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  Someone else defines these  *************************************/
#elif defined(STATUS_LED_PAR)

  /*
   * ADVICE: Define in your board configuration file rather than
   * filling this file up with lots of custom board stuff.
   */

/*****  NetVia   ********************************************************/
#elif defined(CONFIG_NETVIA)

#if !defined(CONFIG_NETVIA_VERSION) || CONFIG_NETVIA_VERSION == 1

#define STATUS_LED_PAR		im_ioport.iop_pdpar
#define STATUS_LED_DIR		im_ioport.iop_pddir
#undef  STATUS_LED_ODR
#define STATUS_LED_DAT		im_ioport.iop_pddat

# define STATUS_LED_BIT		0x0080			/* PD.8 */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_BIT1	0x0040			/* PD.9 */
# define STATUS_LED_PERIOD1	(CFG_HZ / 2)
# define STATUS_LED_STATE1	STATUS_LED_OFF

# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/
# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

#endif

/*****  CMI   ********************************************************/
#elif defined(CONFIG_CMI)
# define STATUS_LED_DIR		im_mios.mios_mpiosm32ddr
# define STATUS_LED_DAT		im_mios.mios_mpiosm32dr

# define STATUS_LED_BIT		0x2000		/* Select one of the 16 possible*/
						/* MIOS outputs */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)	/* Blinking periode is 500 ms */
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 0	*/
# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  KUP4K, KUP4X  ****************************************************/
#elif defined(CONFIG_KUP4K) || defined(CONFIG_KUP4X) || defined(CONFIG_CCM)

# define STATUS_LED_PAR		im_ioport.iop_papar
# define STATUS_LED_DIR		im_ioport.iop_padir
# define STATUS_LED_ODR		im_ioport.iop_paodr
# define STATUS_LED_DAT		im_ioport.iop_padat

# define STATUS_LED_BIT		0x00000300  /*  green + red    PA[8]=yellow,  PA[7]=red,  PA[6]=green */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

#elif defined(CONFIG_SVM_SC8xx)
# define STATUS_LED_PAR         im_cpm.cp_pbpar
# define STATUS_LED_DIR         im_cpm.cp_pbdir
# define STATUS_LED_ODR         im_cpm.cp_pbodr
# define STATUS_LED_DAT         im_cpm.cp_pbdat

# define STATUS_LED_BIT         0x00000001
# define STATUS_LED_PERIOD      (CFG_HZ / 2)
# define STATUS_LED_STATE       STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE      1               /* LED on for bit == 1  */

# define STATUS_LED_BOOT        0               /* LED 0 used for boot status */

/*****  RBC823    ********************************************************/
#elif defined(CONFIG_RBC823)

# define STATUS_LED_PAR         im_ioport.iop_pcpar
# define STATUS_LED_DIR         im_ioport.iop_pcdir
#  undef STATUS_LED_ODR
# define STATUS_LED_DAT         im_ioport.iop_pcdat

# define STATUS_LED_BIT         0x0002          /* LED 0 is on PC.14 */
# define STATUS_LED_PERIOD      (CFG_HZ / 2)
# define STATUS_LED_STATE       STATUS_LED_BLINKING
# define STATUS_LED_BIT1        0x0004          /* LED 1 is on PC.13 */
# define STATUS_LED_PERIOD1     (CFG_HZ)
# define STATUS_LED_STATE1      STATUS_LED_OFF

# define STATUS_LED_ACTIVE      1               /* LED on for bit == 1  */

# define STATUS_LED_BOOT        0               /* LED 0 used for boot status */

/*****  HMI10  **********************************************************/
#elif defined(CONFIG_HMI10)
# define STATUS_LED_PAR		im_ioport.iop_papar
# define STATUS_LED_DIR		im_ioport.iop_padir
# define STATUS_LED_ODR		im_ioport.iop_paodr
# define STATUS_LED_DAT		im_ioport.iop_padat

# define STATUS_LED_BIT		0x00000001	/* LED is on PA15 */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

/*****  NetPhone   ********************************************************/
#elif defined(CONFIG_NETPHONE) || defined(CONFIG_NETTA2)
/* XXX empty just to avoid the error */
/*****  STx XTc    ********************************************************/
#elif defined(CONFIG_STXXTC)
/* XXX empty just to avoid the error */
/*****  sbc8240   ********************************************************/
#elif defined(CONFIG_WRSBC8240)
/* XXX empty just to avoid the error */
/************************************************************************/
#elif defined(CONFIG_NIOS2)
/* XXX empty just to avoid the error */
/************************************************************************/
#elif defined(CONFIG_V38B)

# define STATUS_LED_BIT		0x0010			/* Timer7 GPIO */
# define STATUS_LED_PERIOD	(CFG_HZ / 2)
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0 */
# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

#elif defined(CONFIG_MOTIONPRO)

#define STATUS_LED_BIT		((vu_long *) MPC5XXX_GPT6_ENABLE)
#define STATUS_LED_PERIOD	(CFG_HZ / 10)
#define STATUS_LED_STATE	STATUS_LED_BLINKING

#define STATUS_LED_BIT1		((vu_long *) MPC5XXX_GPT7_ENABLE)
#define STATUS_LED_PERIOD1	(CFG_HZ / 10)
#define STATUS_LED_STATE1	STATUS_LED_OFF

#define STATUS_LED_BOOT		0	/* LED 0 used for boot status */

#else
# error Status LED configuration missing
#endif
/************************************************************************/

#ifndef CONFIG_BOARD_SPECIFIC_LED
# include <asm/status_led.h>
#endif

#endif	/* CONFIG_STATUS_LED	*/

#endif	/* _STATUS_LED_H_	*/
