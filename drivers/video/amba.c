/*
 * Driver for AMBA PrimeCell CLCD
 *
 * Copyright (C) 2009 Alessandro Rubini
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <lcd.h>
#include <amba_clcd.h>

/* These variables are required by lcd.c -- although it sets them by itself */
int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;
void *lcd_base;
void *lcd_console_address;
short console_col;
short console_row;

/*
 * To use this driver you need to provide the following in board files:
 *	a panel_info definition
 *	an lcd_enable function (can't define a weak default with current code)
 */

/* There is nothing to do with color registers, we use true color */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}

/* Low level initialization of the logic cell: depends on panel_info */
void lcd_ctrl_init(void *lcdbase)
{
	struct clcd_config *config;
	struct clcd_registers *regs;
	u32 cntl;

	config = panel_info.priv;
	regs = config->address;
	cntl = config->cntl & ~CNTL_LCDEN;

	/* Lazily, just copy the registers over: first control with disable */
	writel(cntl, &regs->cntl);

	writel(config->tim0, &regs->tim0);
	writel(config->tim1, &regs->tim1);
	writel(config->tim2, &regs->tim2);
	writel(config->tim3, &regs->tim3);
	writel((u32)lcdbase, &regs->ubas);
	/* finally, enable */
	writel(cntl | CNTL_LCDEN, &regs->cntl);
}

/* This is trivial, and copied from atmel_lcdfb.c */
ulong calc_fbsize(void)
{
	return ((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8) + PAGE_SIZE;
}
