/*
 * Driver for ATMEL DataFlash support
 * Author : Hamid Ikdoumi (Atmel)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 * This driver desperately needs rework:
 *
 * - use structure SoC access
 * - get rid of including asm/arch/at91_spi.h
 * - remove asm/arch/at91_spi.h
 * - get rid of all CONFIG_ATMEL_LEGACY defines and uses
 *
 * 02-Aug-2010 Reinhard Meyer <uboot@emk-elektronik.de>
 */

#include <common.h>
#ifndef CONFIG_ATMEL_LEGACY
# define CONFIG_ATMEL_LEGACY
#endif
#include <spi.h>
#include <malloc.h>

#include <asm/io.h>

#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

#include "atmel_spi.h"

#include <asm/arch/gpio.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_spi.h>

#include <dataflash.h>

#define AT91_SPI_PCS0_DATAFLASH_CARD	0xE	/* Chip Select 0: NPCS0%1110 */
#define AT91_SPI_PCS1_DATAFLASH_CARD	0xD	/* Chip Select 1: NPCS1%1101 */
#define AT91_SPI_PCS2_DATAFLASH_CARD	0xB	/* Chip Select 2: NPCS2%1011 */
#define AT91_SPI_PCS3_DATAFLASH_CARD	0x7	/* Chip Select 3: NPCS3%0111 */

void AT91F_SpiInit(void)
{
	/* Reset the SPI */
	writel(AT91_SPI_SWRST, ATMEL_BASE_SPI0 + AT91_SPI_CR);

	/* Configure SPI in Master Mode with No CS selected !!! */
	writel(AT91_SPI_MSTR | AT91_SPI_MODFDIS | AT91_SPI_PCS,
	       ATMEL_BASE_SPI0 + AT91_SPI_MR);

	/* Configure CS0 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((get_mck_clk_rate() / AT91_SPI_CLK) << 8),
	       ATMEL_BASE_SPI0 + AT91_SPI_CSR(0));

#ifdef CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS1
	/* Configure CS1 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((get_mck_clk_rate() / AT91_SPI_CLK) << 8),
	       ATMEL_BASE_SPI0 + AT91_SPI_CSR(1));
#endif
#ifdef CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS2
	/* Configure CS2 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((get_mck_clk_rate() / AT91_SPI_CLK) << 8),
	       ATMEL_BASE_SPI0 + AT91_SPI_CSR(2));
#endif
#ifdef CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3
	/* Configure CS3 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((get_mck_clk_rate() / AT91_SPI_CLK) << 8),
	       ATMEL_BASE_SPI0 + AT91_SPI_CSR(3));
#endif

	/* SPI_Enable */
	writel(AT91_SPI_SPIEN, ATMEL_BASE_SPI0 + AT91_SPI_CR);

	while (!(readl(ATMEL_BASE_SPI0 + AT91_SPI_SR) & AT91_SPI_SPIENS))
		;

	/*
	 * Add tempo to get SPI in a safe state.
	 * Should not be needed for new silicon (Rev B)
	 */
	udelay(500000);
	readl(ATMEL_BASE_SPI0 + AT91_SPI_SR);
	readl(ATMEL_BASE_SPI0 + AT91_SPI_RDR);

}

void AT91F_SpiEnable(int cs)
{
	unsigned long mode;

	switch (cs) {
	case 0:	/* Configure SPI CS0 for Serial DataFlash AT45DBxx */
		mode = readl(ATMEL_BASE_SPI0 + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS0_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       ATMEL_BASE_SPI0 + AT91_SPI_MR);
		break;
	case 1:	/* Configure SPI CS1 for Serial DataFlash AT45DBxx */
		mode = readl(ATMEL_BASE_SPI0 + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS1_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       ATMEL_BASE_SPI0 + AT91_SPI_MR);
		break;
	case 2:	/* Configure SPI CS2 for Serial DataFlash AT45DBxx */
		mode = readl(ATMEL_BASE_SPI0 + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS2_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       ATMEL_BASE_SPI0 + AT91_SPI_MR);
		break;
	case 3:
		mode = readl(ATMEL_BASE_SPI0 + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS3_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       ATMEL_BASE_SPI0 + AT91_SPI_MR);
		break;
	}

	/* SPI_Enable */
	writel(AT91_SPI_SPIEN, ATMEL_BASE_SPI0 + AT91_SPI_CR);
}

unsigned int AT91F_SpiWrite1(AT91PS_DataflashDesc pDesc);

unsigned int AT91F_SpiWrite(AT91PS_DataflashDesc pDesc)
{
	unsigned int timeout;
	unsigned int timebase;

	pDesc->state = BUSY;

	writel(AT91_SPI_TXTDIS + AT91_SPI_RXTDIS,
		ATMEL_BASE_SPI0 + AT91_SPI_PTCR);

	/* Initialize the Transmit and Receive Pointer */
	writel((unsigned int)pDesc->rx_cmd_pt,
		ATMEL_BASE_SPI0 + AT91_SPI_RPR);
	writel((unsigned int)pDesc->tx_cmd_pt,
		ATMEL_BASE_SPI0 + AT91_SPI_TPR);

	/* Intialize the Transmit and Receive Counters */
	writel(pDesc->rx_cmd_size, ATMEL_BASE_SPI0 + AT91_SPI_RCR);
	writel(pDesc->tx_cmd_size, ATMEL_BASE_SPI0 + AT91_SPI_TCR);

	if (pDesc->tx_data_size != 0) {
		/* Initialize the Next Transmit and Next Receive Pointer */
		writel((unsigned int)pDesc->rx_data_pt,
			ATMEL_BASE_SPI0 + AT91_SPI_RNPR);
		writel((unsigned int)pDesc->tx_data_pt,
			ATMEL_BASE_SPI0 + AT91_SPI_TNPR);

		/* Intialize the Next Transmit and Next Receive Counters */
		writel(pDesc->rx_data_size,
			ATMEL_BASE_SPI0 + AT91_SPI_RNCR);
		writel(pDesc->tx_data_size,
			ATMEL_BASE_SPI0 + AT91_SPI_TNCR);
	}

	/* arm simple, non interrupt dependent timer */
	timebase = get_timer(0);
	timeout = 0;

	writel(AT91_SPI_TXTEN + AT91_SPI_RXTEN,
		ATMEL_BASE_SPI0 + AT91_SPI_PTCR);
	while (!(readl(ATMEL_BASE_SPI0 + AT91_SPI_SR) & AT91_SPI_RXBUFF) &&
		((timeout = get_timer(timebase)) < CONFIG_SYS_SPI_WRITE_TOUT))
		;
	writel(AT91_SPI_TXTDIS + AT91_SPI_RXTDIS,
		ATMEL_BASE_SPI0 + AT91_SPI_PTCR);
	pDesc->state = IDLE;

	if (timeout >= CONFIG_SYS_SPI_WRITE_TOUT) {
		printf("Error Timeout\n\r");
		return DATAFLASH_ERROR;
	}

	return DATAFLASH_OK;
}
