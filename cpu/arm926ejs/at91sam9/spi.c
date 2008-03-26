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

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_spi.h>

#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>

#define AT91_SPI_PCS0_DATAFLASH_CARD	0xE	/* Chip Select 0: NPCS0%1110 */
#define AT91_SPI_PCS1_DATAFLASH_CARD	0xD	/* Chip Select 0: NPCS0%1101 */
#define AT91_SPI_PCS3_DATAFLASH_CARD	0x7	/* Chip Select 3: NPCS3%0111 */

void AT91F_SpiInit(void)
{
	/* Reset the SPI */
	writel(AT91_SPI_SWRST, AT91_BASE_SPI + AT91_SPI_CR);

	/* Configure SPI in Master Mode with No CS selected !!! */
	writel(AT91_SPI_MSTR | AT91_SPI_MODFDIS | AT91_SPI_PCS,
	       AT91_BASE_SPI + AT91_SPI_MR);

	/* Configure CS0 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((AT91_MASTER_CLOCK / AT91_SPI_CLK) << 8),
	       AT91_BASE_SPI + AT91_SPI_CSR(0));

#ifdef CFG_DATAFLASH_LOGIC_ADDR_CS1
	/* Configure CS1 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((AT91_MASTER_CLOCK / AT91_SPI_CLK) << 8),
	       AT91_BASE_SPI + AT91_SPI_CSR(1));
#endif

#ifdef CFG_DATAFLASH_LOGIC_ADDR_CS3
	/* Configure CS3 */
	writel(AT91_SPI_NCPHA |
	       (AT91_SPI_DLYBS & DATAFLASH_TCSS) |
	       (AT91_SPI_DLYBCT & DATAFLASH_TCHS) |
	       ((AT91_MASTER_CLOCK / AT91_SPI_CLK) << 8),
	       AT91_BASE_SPI + AT91_SPI_CSR(3));
#endif

	/* SPI_Enable */
	writel(AT91_SPI_SPIEN, AT91_BASE_SPI + AT91_SPI_CR);

	while (!(readl(AT91_BASE_SPI + AT91_SPI_SR) & AT91_SPI_SPIENS));

	/*
	 * Add tempo to get SPI in a safe state.
	 * Should not be needed for new silicon (Rev B)
	 */
	udelay(500000);
	readl(AT91_BASE_SPI + AT91_SPI_SR);
	readl(AT91_BASE_SPI + AT91_SPI_RDR);

}

void AT91F_SpiEnable(int cs)
{
	unsigned long mode;
	switch (cs) {
	case 0:	/* Configure SPI CS0 for Serial DataFlash AT45DBxx */
		mode = readl(AT91_BASE_SPI + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS0_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       AT91_BASE_SPI + AT91_SPI_MR);
		break;
	case 1:	/* Configure SPI CS1 for Serial DataFlash AT45DBxx */
		mode = readl(AT91_BASE_SPI + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS1_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       AT91_BASE_SPI + AT91_SPI_MR);
		break;
	case 3:
		mode = readl(AT91_BASE_SPI + AT91_SPI_MR);
		mode &= 0xFFF0FFFF;
		writel(mode | ((AT91_SPI_PCS3_DATAFLASH_CARD<<16) & AT91_SPI_PCS),
		       AT91_BASE_SPI + AT91_SPI_MR);
		break;
	}

	/* SPI_Enable */
	writel(AT91_SPI_SPIEN, AT91_BASE_SPI + AT91_SPI_CR);
}

unsigned int AT91F_SpiWrite1(AT91PS_DataflashDesc pDesc);

unsigned int AT91F_SpiWrite(AT91PS_DataflashDesc pDesc)
{
	unsigned int timeout;


	pDesc->state = BUSY;

	writel(AT91_SPI_TXTDIS + AT91_SPI_RXTDIS, AT91_BASE_SPI + AT91_SPI_PTCR);


	/* Initialize the Transmit and Receive Pointer */
	writel((unsigned int)pDesc->rx_cmd_pt, AT91_BASE_SPI + AT91_SPI_RPR);
	writel((unsigned int)pDesc->tx_cmd_pt, AT91_BASE_SPI + AT91_SPI_TPR);

	/* Intialize the Transmit and Receive Counters */
	writel(pDesc->rx_cmd_size, AT91_BASE_SPI + AT91_SPI_RCR);
	writel(pDesc->tx_cmd_size, AT91_BASE_SPI + AT91_SPI_TCR);

	if (pDesc->tx_data_size != 0) {
		/* Initialize the Next Transmit and Next Receive Pointer */
		writel((unsigned int)pDesc->rx_data_pt, AT91_BASE_SPI + AT91_SPI_RNPR);
		writel((unsigned int)pDesc->tx_data_pt, AT91_BASE_SPI + AT91_SPI_TNPR);

		/* Intialize the Next Transmit and Next Receive Counters */
		writel(pDesc->rx_data_size, AT91_BASE_SPI + AT91_SPI_RNCR);
		writel(pDesc->tx_data_size, AT91_BASE_SPI + AT91_SPI_TNCR);
	}

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked();
	timeout = 0;

	writel(AT91_SPI_TXTEN + AT91_SPI_RXTEN, AT91_BASE_SPI + AT91_SPI_PTCR);
	while (!(readl(AT91_BASE_SPI + AT91_SPI_SR) & AT91_SPI_RXBUFF) &&
		((timeout = get_timer_masked()) < CFG_SPI_WRITE_TOUT));
	writel(AT91_SPI_TXTDIS + AT91_SPI_RXTDIS, AT91_BASE_SPI + AT91_SPI_PTCR);
	pDesc->state = IDLE;

	if (timeout >= CFG_SPI_WRITE_TOUT) {
		printf("Error Timeout\n\r");
		return DATAFLASH_ERROR;
	}

	return DATAFLASH_OK;
}
#endif
