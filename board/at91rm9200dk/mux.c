#include <config.h>
#include <common.h>
#include <asm/hardware.h>
#include <dataflash.h>

int AT91F_GetMuxStatus(void) {
#ifdef	DATAFLASH_MMC_SELECT
	AT91C_BASE_PIOB->PIO_PER = DATAFLASH_MMC_SELECT; /* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = DATAFLASH_MMC_SELECT; /* Configure in output */


	if(AT91C_BASE_PIOB->PIO_ODSR & DATAFLASH_MMC_SELECT) {
		return 1;
	} else {
		return 0;
	}
#endif
	return 0;
}

void AT91F_SelectMMC(void) {
#ifdef	DATAFLASH_MMC_SELECT
	AT91C_BASE_PIOB->PIO_PER = DATAFLASH_MMC_SELECT;	/* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = DATAFLASH_MMC_SELECT;	/* Configure in output */
	/* Set Output */
	AT91C_BASE_PIOB->PIO_SODR = DATAFLASH_MMC_SELECT; 
#endif 
}

void AT91F_SelectSPI(void) {
#ifdef	DATAFLASH_MMC_SELECT
	AT91C_BASE_PIOB->PIO_PER = DATAFLASH_MMC_SELECT;	/* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = DATAFLASH_MMC_SELECT;	/* Configure in output */
	/* Clear Output */
	AT91C_BASE_PIOB->PIO_CODR = DATAFLASH_MMC_SELECT; 
#endif 
}


