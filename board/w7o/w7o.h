/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
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

#ifndef _W7O_H_
#define _W7O_H_
#include <config.h>

/* AMCC 405GP PowerPC GPIO registers */
#define PPC405GP_GPIO0_OR	0xef600700L	/* GPIO Output */
#define PPC405GP_GPIO0_TCR	0xef600704L	/* GPIO Three-State Control */
#define PPC405GP_GPIO0_ODR	0xef600718L	/* GPIO Open Drain */
#define PPC405GP_GPIO0_IR	0xef60071cL	/* GPIO Input */

/* LMG FPGA <=> CPU GPIO signals */
#define LMG_XCV_INIT		0x10000000L
#define LMG_XCV_PROG		0x04000000L
#define LMG_XCV_DONE		0x00400000L
#define LMG_XCV_CNFG_0		0x08000000L
#define LMG_XCV_IRQ_0		0x0L

/* LMC FPGA <=> CPU GPIO signals */
#define LMC_XCV_INIT		0x00800000L
#define LMC_XCV_PROG		0x40000000L
#define LMC_XCV_DONE		0x01000000L
#define LMC_XCV_CNFG_0		0x00004000L	/* Shared with IRQ 0 */
#define LMC_XCV_CNFG_1		0x00002000L	/* Shared with IRQ 1 */
#define LMC_XCV_CNFG_2		0x00001000L	/* Shared with IRQ 2 */
#define LMC_XCV_IRQ_0		0x00080000L	/* Shared with GPIO 17 */
#define LMC_XCV_IRQ_1		0x00040000L	/* Shared with GPIO 18 */
#define LMC_XCV_IRQ_3		0x00020000L	/* Shared tiwht GPIO 19 */


/*
 * Setup FPGA <=> GPIO mappings
 */
#if defined(CONFIG_W7OLMG)
# define GPIO_XCV_INIT		LMG_XCV_INIT
# define GPIO_XCV_PROG		LMG_XCV_PROG
# define GPIO_XCV_DONE		LMG_XCV_DONE
# define GPIO_XCV_CNFG		LMG_XCV_CNFG_0
# define GPIO_XCV_IRQ		LMG_XCV_IRQ_0
# define GPIO_GPIO_1		0x40000000L
# define GPIO_GPIO_6		0x02000000L
# define GPIO_GPIO_7		0x01000000L
# define GPIO_GPIO_8		0x00800000L
#elif defined(CONFIG_W7OLMC)
# define GPIO_XCV_INIT		LMC_XCV_INIT
# define GPIO_XCV_PROG		LMC_XCV_PROG
# define GPIO_XCV_DONE		LMC_XCV_DONE
# define GPIO_XCV_CNFG		LMC_XCV_CNFG_0
# define GPIO_XCV_IRQ		LMC_XCV_IRQ_0
#else
# error "Unknown W7O board configuration"
#endif

/* Power On Self Tests */
extern void post2(void);
extern int test_led(void);
extern int test_sdram(unsigned long size);
extern void test_fpga(unsigned short *daddr);

/* FGPA */
extern int init_fpga(void);

/* Misc */
extern int temp_uart_init(void);
extern void init_fsboot(void);

#endif /* _W7O_H_ */
