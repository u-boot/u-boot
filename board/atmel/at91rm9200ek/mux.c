#include <config.h>
#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <dataflash.h>

int AT91F_GetMuxStatus(void)
{
	/* Set in PIO mode */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_PER);
	/* Configure in output */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_OER);

	if(readl(AT91C_BASE_PIOB->PIO_ODSR) & CONFIG_SYS_DATAFLASH_MMC_PIO)
		return 1;

	return 0;
}

void AT91F_SelectMMC(void)
{
	/* Set in PIO mode */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_PER);
	/* Configure in output */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_OER);
	/* Set Output */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_SODR);
}

void AT91F_SelectSPI(void)
{
	/* Set in PIO mode */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_PER);
	/* Configure in output */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_OER);
	/* Clear Output */
	writel(CONFIG_SYS_DATAFLASH_MMC_PIO, AT91C_BASE_PIOB->PIO_CODR);
}
