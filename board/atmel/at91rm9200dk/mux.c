#include <config.h>
#include <common.h>
#include <asm/hardware.h>
#include <dataflash.h>

int AT91F_GetMuxStatus(void) {
	AT91C_BASE_PIOB->PIO_PER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Configure in output */


	if(AT91C_BASE_PIOB->PIO_ODSR & CONFIG_SYS_DATAFLASH_MMC_PIO)
		return 1;

	return 0;
}

void AT91F_SelectMMC(void) {
	AT91C_BASE_PIOB->PIO_PER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Configure in output */
	/* Set Output */
	AT91C_BASE_PIOB->PIO_SODR = CONFIG_SYS_DATAFLASH_MMC_PIO;
}

void AT91F_SelectSPI(void) {
	AT91C_BASE_PIOB->PIO_PER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Set in PIO mode */
	AT91C_BASE_PIOB->PIO_OER = CONFIG_SYS_DATAFLASH_MMC_PIO; /* Configure in output */
	/* Clear Output */
	AT91C_BASE_PIOB->PIO_CODR = CONFIG_SYS_DATAFLASH_MMC_PIO;
}
