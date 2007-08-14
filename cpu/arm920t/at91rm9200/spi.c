/* Driver for ATMEL DataFlash support
 * Author : Hamid Ikdoumi (Atmel)
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
 *
 */

#include <config.h>
#include <common.h>
#include <asm/hardware.h>

#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>

#define AT91C_SPI_CLK	10000000	/* Max Value = 10MHz to be compliant to
					the Continuous Array Read function */

/* AC Characteristics */
/* DLYBS = tCSS = 250ns min and DLYBCT = tCSH = 250ns */
#define DATAFLASH_TCSS	(0xC << 16)
#define DATAFLASH_TCHS	(0x1 << 24)

#define AT91C_TIMEOUT_WRDY		200000
#define AT91C_SPI_PCS0_SERIAL_DATAFLASH	0xE	/* Chip Select 0: NPCS0%1110 */
#define AT91C_SPI_PCS3_DATAFLASH_CARD	0x7	/* Chip Select 3: NPCS3%0111 */

/*-------------------------------------------------------------------*/
/*	SPI DataFlash Init					     */
/*-------------------------------------------------------------------*/
void AT91F_SpiInit(void)
{
	/* Configure PIOs */
	AT91C_BASE_PIOA->PIO_ASR =
		AT91C_PA3_NPCS0 | AT91C_PA4_NPCS1 | AT91C_PA1_MOSI |
		AT91C_PA5_NPCS2 | AT91C_PA6_NPCS3 | AT91C_PA0_MISO |
		AT91C_PA2_SPCK;
	AT91C_BASE_PIOA->PIO_PDR =
		AT91C_PA3_NPCS0 | AT91C_PA4_NPCS1 | AT91C_PA1_MOSI |
		AT91C_PA5_NPCS2 | AT91C_PA6_NPCS3 | AT91C_PA0_MISO |
		AT91C_PA2_SPCK;
	/* Enable CLock */
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI;

	/* Reset the SPI */
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SWRST;

	/* Configure SPI in Master Mode with No CS selected !!! */
	AT91C_BASE_SPI->SPI_MR =
		AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | AT91C_SPI_PCS;

	/* Configure CS0 and CS3 */
	*(AT91C_SPI_CSR + 0) =
		AT91C_SPI_CPOL | (AT91C_SPI_DLYBS & DATAFLASH_TCSS) |
		(AT91C_SPI_DLYBCT & DATAFLASH_TCHS) |
		((AT91C_MASTER_CLOCK / (2*AT91C_SPI_CLK)) << 8);

	*(AT91C_SPI_CSR + 3) =
		AT91C_SPI_CPOL | (AT91C_SPI_DLYBS & DATAFLASH_TCSS) |
		(AT91C_SPI_DLYBCT & DATAFLASH_TCHS) |
		((AT91C_MASTER_CLOCK / (2*AT91C_SPI_CLK)) << 8);
}

void AT91F_SpiEnable(int cs)
{
	switch(cs) {
	case 0:	/* Configure SPI CS0 for Serial DataFlash AT45DBxx */
		AT91C_BASE_SPI->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI->SPI_MR |=
			((AT91C_SPI_PCS0_SERIAL_DATAFLASH<<16) &
				AT91C_SPI_PCS);
		break;
	case 3:	/* Configure SPI CS3 for Serial DataFlash Card */
		/* Set up PIO SDC_TYPE to switch on DataFlash Card */
		/* and not MMC/SDCard */
		AT91C_BASE_PIOB->PIO_PER =
			AT91C_PIO_PB7;	/* Set in PIO mode */
		AT91C_BASE_PIOB->PIO_OER =
			AT91C_PIO_PB7;	/* Configure in output */
		/* Clear Output */
		AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB7;
		/* Configure PCS */
		AT91C_BASE_SPI->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI->SPI_MR |=
			((AT91C_SPI_PCS3_DATAFLASH_CARD<<16) & AT91C_SPI_PCS);
		break;
	}

	/* SPI_Enable */
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN; }

/*---------------------------------------------------------------------------*/
/* \fn    AT91F_SpiWrite						     */
/* \brief Set the PDC registers for a transfert				     */
/*---------------------------------------------------------------------------*/
unsigned int AT91F_SpiWrite ( AT91PS_DataflashDesc pDesc )
{
	unsigned int timeout;

	pDesc->state = BUSY;

	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;

	/* Initialize the Transmit and Receive Pointer */
	AT91C_BASE_SPI->SPI_RPR = (unsigned int)pDesc->rx_cmd_pt ;
	AT91C_BASE_SPI->SPI_TPR = (unsigned int)pDesc->tx_cmd_pt ;

	/* Intialize the Transmit and Receive Counters */
	AT91C_BASE_SPI->SPI_RCR = pDesc->rx_cmd_size;
	AT91C_BASE_SPI->SPI_TCR = pDesc->tx_cmd_size;

	if ( pDesc->tx_data_size != 0 ) {
		/* Initialize the Next Transmit and Next Receive Pointer */
		AT91C_BASE_SPI->SPI_RNPR = (unsigned int)pDesc->rx_data_pt ;
		AT91C_BASE_SPI->SPI_TNPR = (unsigned int)pDesc->tx_data_pt ;

		/* Intialize the Next Transmit and Next Receive Counters */
		AT91C_BASE_SPI->SPI_RNCR = pDesc->rx_data_size ;
		AT91C_BASE_SPI->SPI_TNCR = pDesc->tx_data_size ;
	}

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked();
	timeout = 0;

	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTEN + AT91C_PDC_RXTEN;
	while(!(AT91C_BASE_SPI->SPI_SR & AT91C_SPI_RXBUFF) &&
		((timeout = get_timer_masked() ) < CFG_SPI_WRITE_TOUT));
	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;
	pDesc->state = IDLE;

	if (timeout >= CFG_SPI_WRITE_TOUT){
		printf("Error Timeout\n\r");
		return DATAFLASH_ERROR;
	}

	return DATAFLASH_OK;
}

#endif

