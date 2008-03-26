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

#include <config.h>
#include <common.h>
#include <asm/hardware.h>

#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>

/* Max Value = 10MHz to be compliant to the Continuous Array Read function */
#define AT91C_SPI_CLK	10000000

/* AC Characteristics: DLYBS = tCSS = 250ns min and DLYBCT = tCSH = 250ns */
#define DATAFLASH_TCSS	(0xFA << 16)
#define DATAFLASH_TCHS	(0x8 << 24)

#define AT91C_TIMEOUT_WRDY		200000
#define AT91C_SPI_PCS0_DATAFLASH_CARD	0xE	/* Chip Select 0: NPCS0%1110 */
#define AT91C_SPI_PCS3_DATAFLASH_CARD	0x7	/* Chip Select 3: NPCS3%0111 */

void AT91F_SpiInit(void)
{
	/* Reset the SPI */
	AT91C_BASE_SPI0->SPI_CR = AT91C_SPI_SWRST;

	/* Configure SPI in Master Mode with No CS selected !!! */
	AT91C_BASE_SPI0->SPI_MR =
		AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | AT91C_SPI_PCS;

	/* Configure CS0 */
	AT91C_BASE_SPI0->SPI_CSR[0] =
		AT91C_SPI_CPOL |
		(AT91C_SPI_DLYBS & DATAFLASH_TCSS) |
		(AT91C_SPI_DLYBCT & DATAFLASH_TCHS) |
		((AT91C_MASTER_CLOCK / (2*AT91C_SPI_CLK)) << 8);
}

void AT91F_SpiEnable(int cs)
{
	switch (cs) {
	case 0:	/* Configure SPI CS0 for Serial DataFlash AT45DBxx */
		AT91C_BASE_SPI0->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI0->SPI_MR |=
			((AT91C_SPI_PCS0_DATAFLASH_CARD<<16) & AT91C_SPI_PCS);
		break;
	case 3:
		AT91C_BASE_SPI0->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI0->SPI_MR |=
			((AT91C_SPI_PCS3_DATAFLASH_CARD<<16) & AT91C_SPI_PCS);
		break;
	}

	/* SPI_Enable */
	AT91C_BASE_SPI0->SPI_CR = AT91C_SPI_SPIEN;
}

unsigned int AT91F_SpiWrite(AT91PS_DataflashDesc pDesc)
{
	unsigned int timeout;

	pDesc->state = BUSY;

	AT91C_BASE_SPI0->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;

	/* Initialize the Transmit and Receive Pointer */
	AT91C_BASE_SPI0->SPI_RPR = (unsigned int)pDesc->rx_cmd_pt;
	AT91C_BASE_SPI0->SPI_TPR = (unsigned int)pDesc->tx_cmd_pt;

	/* Intialize the Transmit and Receive Counters */
	AT91C_BASE_SPI0->SPI_RCR = pDesc->rx_cmd_size;
	AT91C_BASE_SPI0->SPI_TCR = pDesc->tx_cmd_size;

	if (pDesc->tx_data_size != 0) {
		/* Initialize the Next Transmit and Next Receive Pointer */
		AT91C_BASE_SPI0->SPI_RNPR = (unsigned int)pDesc->rx_data_pt;
		AT91C_BASE_SPI0->SPI_TNPR = (unsigned int)pDesc->tx_data_pt;

		/* Intialize the Next Transmit and Next Receive Counters */
		AT91C_BASE_SPI0->SPI_RNCR = pDesc->rx_data_size;
		AT91C_BASE_SPI0->SPI_TNCR = pDesc->tx_data_size;
	}

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked();
	timeout = 0;

	AT91C_BASE_SPI0->SPI_PTCR = AT91C_PDC_TXTEN + AT91C_PDC_RXTEN;
	while (!(AT91C_BASE_SPI0->SPI_SR & AT91C_SPI_RXBUFF) &&
		((timeout = get_timer_masked()) < CFG_SPI_WRITE_TOUT));
	AT91C_BASE_SPI0->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;
	pDesc->state = IDLE;

	if (timeout >= CFG_SPI_WRITE_TOUT) {
		printf("Error Timeout\n\r");
		return DATAFLASH_ERROR;
	}

	return DATAFLASH_OK;
}
#endif
