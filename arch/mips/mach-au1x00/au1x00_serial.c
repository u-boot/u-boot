// SPDX-License-Identifier: GPL-2.0+
/*
 * AU1X00 UART support
 *
 * Hardcoded to UART 0 for now
 * Speed and options also hardcoded to 115200 8N1
 *
 *  Copyright (c) 2003	Thomas.Lange@corelatus.se
 */

#include <config.h>
#include <common.h>
#include <mach/au1x00.h>
#include <serial.h>
#include <linux/compiler.h>

/******************************************************************************
*
* serial_init - initialize a channel
*
* This routine initializes the number of data bits, parity
* and set the selected baud rate. Interrupts are disabled.
* Set the modem control signals if the option is selected.
*
* RETURNS: N/A
*/

static int au1x00_serial_init(void)
{
	volatile u32 *uart_fifoctl = (volatile u32*)(UART0_ADDR+UART_FCR);
	volatile u32 *uart_enable = (volatile u32*)(UART0_ADDR+UART_ENABLE);

	/* Enable clocks first */
	*uart_enable = UART_EN_CE;

	/* Then release reset */
	/* Must release reset before setting other regs */
	*uart_enable = UART_EN_CE|UART_EN_E;

	/* Activate fifos, reset tx and rx */
	/* Set tx trigger level to 12 */
	*uart_fifoctl = UART_FCR_ENABLE_FIFO|UART_FCR_CLEAR_RCVR|
		UART_FCR_CLEAR_XMIT|UART_FCR_T_TRIGGER_12;

	serial_setbrg();

	return 0;
}


static void au1x00_serial_setbrg(void)
{
	volatile u32 *uart_clk = (volatile u32*)(UART0_ADDR+UART_CLK);
	volatile u32 *uart_lcr = (volatile u32*)(UART0_ADDR+UART_LCR);
	volatile u32 *sys_powerctrl = (u32 *)SYS_POWERCTRL;
	int sd;
	int divisorx2;

	/* sd is system clock divisor			*/
	/* see section 10.4.5 in au1550 datasheet	*/
	sd = (*sys_powerctrl & 0x03) + 2;

	/* calulate 2x baudrate and round */
	divisorx2 = ((CONFIG_SYS_MIPS_TIMER_FREQ/(sd * 16 * CONFIG_BAUDRATE)));

	if (divisorx2 & 0x01)
		divisorx2 = divisorx2 + 1;

	*uart_clk = divisorx2 / 2;

	/* Set parity, stop bits and word length to 8N1 */
	*uart_lcr = UART_LCR_WLEN8;
}

static void au1x00_serial_putc(const char c)
{
	volatile u32 *uart_lsr = (volatile u32*)(UART0_ADDR+UART_LSR);
	volatile u32 *uart_tx = (volatile u32*)(UART0_ADDR+UART_TX);

	if (c == '\n')
		au1x00_serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while((*uart_lsr&UART_LSR_THRE)==0);

	*uart_tx = (u32)c;
}

static int au1x00_serial_getc(void)
{
	volatile u32 *uart_rx = (volatile u32*)(UART0_ADDR+UART_RX);
	char c;

	while (!serial_tstc());

	c = (*uart_rx&0xFF);
	return c;
}

static int au1x00_serial_tstc(void)
{
	volatile u32 *uart_lsr = (volatile u32*)(UART0_ADDR+UART_LSR);

	if(*uart_lsr&UART_LSR_DR){
		/* Data in rfifo */
		return(1);
	}
	return 0;
}

static struct serial_device au1x00_serial_drv = {
	.name	= "au1x00_serial",
	.start	= au1x00_serial_init,
	.stop	= NULL,
	.setbrg	= au1x00_serial_setbrg,
	.putc	= au1x00_serial_putc,
	.puts	= default_serial_puts,
	.getc	= au1x00_serial_getc,
	.tstc	= au1x00_serial_tstc,
};

void au1x00_serial_initialize(void)
{
	serial_register(&au1x00_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &au1x00_serial_drv;
}
