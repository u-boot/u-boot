/*
 * Blackfin MUSB HCD (Host Controller Driver) for u-boot
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>

#include <usb.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/usb.h>

#include "musb_core.h"

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	.regs       = (struct musb_regs *)USB_FADDR,
	.timeout    = 0x3FFFFFF,
	.musb_speed = 0,
};

/*
 * This function read or write data to endpoint fifo
 * Blackfin use DMA polling method to avoid buffer alignment issues
 *
 * ep		- Endpoint number
 * length	- Number of bytes to write to FIFO
 * fifo_data	- Pointer to data buffer to be read/write
 * is_write	- Flag for read or write
 */
void rw_fifo(u8 ep, u32 length, void *fifo_data, int is_write)
{
	struct bfin_musb_dma_regs *regs;
	u32 val = (u32)fifo_data;

	blackfin_dcache_flush_invalidate_range(fifo_data, fifo_data + length);

	regs = (void *)USB_DMA_INTERRUPT;
	regs += ep;

	/* Setup DMA address register */
	bfin_write16(&regs->addr_low, val);
	SSYNC();

	bfin_write16(&regs->addr_high, val >> 16);
	SSYNC();

	/* Setup DMA count register */
	bfin_write16(&regs->count_low, length);
	bfin_write16(&regs->count_high, 0);
	SSYNC();

	/* Enable the DMA */
	val = (ep << 4) | DMA_ENA | INT_ENA;
	if (is_write)
		val |= DIRECTION;
	bfin_write16(&regs->control, val);
	SSYNC();

	/* Wait for compelete */
	while (!(bfin_read_USB_DMA_INTERRUPT() & (1 << ep)))
		continue;

	/* acknowledge dma interrupt */
	bfin_write_USB_DMA_INTERRUPT(1 << ep);
	SSYNC();

	/* Reset DMA */
	bfin_write16(&regs->control, 0);
	SSYNC();
}

void write_fifo(u8 ep, u32 length, void *fifo_data)
{
	rw_fifo(ep, length, fifo_data, 1);
}

void read_fifo(u8 ep, u32 length, void *fifo_data)
{
	rw_fifo(ep, length, fifo_data, 0);
}


/*
 * CPU and board-specific MUSB initializations.  Aliased function
 * signals caller to move on.
 */
static void __def_musb_init(void)
{
}
void board_musb_init(void) __attribute__((weak, alias("__def_musb_init")));

int musb_platform_init(void)
{
	/* board specific initialization */
	board_musb_init();

	if (ANOMALY_05000346) {
		bfin_write_USB_APHY_CALIB(ANOMALY_05000346_value);
		SSYNC();
	}

	if (ANOMALY_05000347) {
		bfin_write_USB_APHY_CNTRL(0x0);
		SSYNC();
	}

	/* Configure PLL oscillator register */
	bfin_write_USB_PLLOSC_CTRL(0x30a8);
	SSYNC();

	bfin_write_USB_SRP_CLKDIV((get_sclk()/1000) / 32 - 1);
	SSYNC();

	bfin_write_USB_EP_NI0_RXMAXP(64);
	SSYNC();

	bfin_write_USB_EP_NI0_TXMAXP(64);
	SSYNC();

	/* Route INTRUSB/INTR_RX/INTR_TX to USB_INT0*/
	bfin_write_USB_GLOBINTR(0x7);
	SSYNC();

	bfin_write_USB_GLOBAL_CTL(GLOBAL_ENA | EP1_TX_ENA | EP2_TX_ENA |
				EP3_TX_ENA | EP4_TX_ENA | EP5_TX_ENA |
				EP6_TX_ENA | EP7_TX_ENA | EP1_RX_ENA |
				EP2_RX_ENA | EP3_RX_ENA | EP4_RX_ENA |
				EP5_RX_ENA | EP6_RX_ENA | EP7_RX_ENA);
	SSYNC();

	return 0;
}

/*
 * This function performs Blackfin platform specific deinitialization for usb.
*/
void musb_platform_deinit(void)
{
}
