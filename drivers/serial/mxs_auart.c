/*
 * Freescale i.MX23/i.MX28 AUART driver
 *
 * Copyright (C) 2013 Andreas Wass <andreas.wass@dalelven.com>
 *
 * Based on the MXC serial driver:
 *
 * (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>
 *
 * Further based on the Linux mxs-auart.c driver:
 *
 * Freescale STMP37XX/STMP38X Application UART drkiver
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/regs-base.h>
#include <asm/arch/regs-uartapp.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_MXS_AUART_BASE
#error "CONFIG_MXS_AUART_BASE must be set to the base UART to use"
#endif

/* AUART clock always supplied by XTAL and always 24MHz */
#define MXS_AUART_CLK 24000000

static struct mxs_uartapp_regs *get_uartapp_registers(void)
{
	return (struct mxs_uartapp_regs *)CONFIG_MXS_AUART_BASE;
}

/**
 * Sets the baud rate and settings.
 * The settings are: 8 data bits, no parit and 1 stop bit.
 */
static void mxs_auart_setbrg(void)
{
	u32 div;
	u32 linectrl = 0;
	struct mxs_uartapp_regs *regs = get_uartapp_registers();

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	/*
	 * From i.MX28 datasheet:
	 * div is calculated by calculating UARTCLK*32/baudrate, rounded to int
	 * div must be between 0xEC and 0x003FFFC0 inclusive
	 * Lowest 6 bits of div goes in BAUD_DIVFRAC part of LINECTRL register
	 * Next 16 bits goes in BAUD_DIVINT part of LINECTRL register
	 */
	div = (MXS_AUART_CLK * 32) / gd->baudrate;
	if (div < 0xEC || div > 0x003FFFC0)
		return;

	linectrl |= ((div & UARTAPP_LINECTRL_EXTRACT_BAUD_DIVFRAC_MASK) <<
		UARTAPP_LINECTRL_BAUD_DIVFRAC_OFFSET) &
		UARTAPP_LINECTRL_BAUD_DIVFRAC_MASK;
	linectrl |= ((div >> UARTAPP_LINECTRL_EXTRACT_BAUD_DIVINT_OFFSET) <<
		UARTAPP_LINECTRL_BAUD_DIVINT_OFFSET) &
		UARTAPP_LINECTRL_BAUD_DIVINT_MASK;

	/* Word length: 8 bits */
	linectrl |= UARTAPP_LINECTRL_WLEN_8BITS;

	/* Enable FIFOs. */
	linectrl |= UARTAPP_LINECTRL_FEN_MASK;

	/* Write above settings, no parity, 1 stop bit */
	writel(linectrl, &regs->hw_uartapp_linectrl);
}

static int mxs_auart_init(void)
{
	struct mxs_uartapp_regs *regs = get_uartapp_registers();
	/* Reset everything */
	mxs_reset_block(&regs->hw_uartapp_ctrl0_reg);
	/* Disable interrupts */
	writel(0, &regs->hw_uartapp_intr);
	/* Set baud rate and settings */
	serial_setbrg();
	/* Disable RTS and CTS, ignore LINECTRL2 register */
	writel(UARTAPP_CTRL2_RTSEN_MASK |
			UARTAPP_CTRL2_CTSEN_MASK |
			UARTAPP_CTRL2_USE_LCR2_MASK,
			&regs->hw_uartapp_ctrl2_clr);
	/* Enable receiver, transmitter and UART */
	writel(UARTAPP_CTRL2_RXE_MASK |
			UARTAPP_CTRL2_TXE_MASK |
			UARTAPP_CTRL2_UARTEN_MASK,
			&regs->hw_uartapp_ctrl2_set);
	return 0;
}

static void mxs_auart_putc(const char c)
{
	struct mxs_uartapp_regs *regs = get_uartapp_registers();
	/* Wait in loop while the transmit FIFO is full */
	while (readl(&regs->hw_uartapp_stat) & UARTAPP_STAT_TXFF_MASK)
		;

	writel(c, &regs->hw_uartapp_data);

	if (c == '\n')
		mxs_auart_putc('\r');
}

static int mxs_auart_tstc(void)
{
	struct mxs_uartapp_regs *regs = get_uartapp_registers();
	/* Checks if receive FIFO is empty */
	return !(readl(&regs->hw_uartapp_stat) & UARTAPP_STAT_RXFE_MASK);
}

static int mxs_auart_getc(void)
{
	struct mxs_uartapp_regs *regs = get_uartapp_registers();
	/* Wait until a character is available to read */
	while (!mxs_auart_tstc())
		;
	/* Read the character from the data register */
	return readl(&regs->hw_uartapp_data) & 0xFF;
}

static struct serial_device mxs_auart_drv = {
	.name = "mxs_auart_serial",
	.start = mxs_auart_init,
	.stop = NULL,
	.setbrg = mxs_auart_setbrg,
	.putc = mxs_auart_putc,
	.puts = default_serial_puts,
	.getc = mxs_auart_getc,
	.tstc = mxs_auart_tstc,
};

void mxs_auart_initialize(void)
{
	serial_register(&mxs_auart_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &mxs_auart_drv;
}
