/*
 * U-boot - main board file
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <netdev.h>
#include <config.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/sdh.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF548 EZ-Kit board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

int board_early_init_f(void)
{
	/* Port H: PH8 - PH13 == A4 - A9
	 * address lines of the parallel asynchronous memory interface
	 */

	/************************************************
	* configure GPIO 				*
	* set port H function enable register		*
	*  configure PH8-PH13 as peripheral (not GPIO) 	*
	*************************************************/
	bfin_write_PORTH_FER(0x3F03);

	/************************************************
	* set port H MUX to configure PH8-PH13		*
	*  1st Function (MUX = 00) (bits 16-27 == 0)	*
	*  Set to address signals A4-A9 		*
	*************************************************/
	bfin_write_PORTH_MUX(0);

	/************************************************
	* set port H direction register			*
	*  enable PH8-PH13 as outputs			*
	*************************************************/
	bfin_write_PORTH_DIR_SET(0x3F00);

	/* Port I: PI0 - PH14 == A10 - A24
	 * address lines of the parallel asynchronous memory interface
	 */

	/************************************************
	* set port I function enable register		*
	*  configure PI0-PI14 as peripheral (not GPIO) 	*
	*************************************************/
	bfin_write_PORTI_FER(0x7fff);

	/**************************************************
	* set PORT I MUX to configure PI14-PI0 as	  *
	* 1st Function (MUX=00) - address signals A10-A24 *
	***************************************************/
	bfin_write_PORTI_MUX(0);

	/****************************************
	* set PORT I direction register		*
	*  enable PI0 - PI14 as outputs		*
	*****************************************/
	bfin_write_PORTI_DIR_SET(0x7fff);

	return 0;
}

#ifdef CONFIG_SMC911X
int board_eth_init(bd_t *bis)
{
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif

#ifdef CONFIG_BFIN_SDH
int board_mmc_init(bd_t *bis)
{
	return bfin_mmc_init(bis);
}
#endif

#ifdef CONFIG_USB_BLACKFIN
void board_musb_init(void)
{
	/*
	 * Rev 1.0 BF549 EZ-KITs require PE7 to be high for both device
	 * and OTG host modes, while rev 1.1 and greater require PE7 to
	 * be low for device mode and high for host mode.  We set it high
	 * here because we are in host mode.
	 */
	bfin_write_PORTE_FER(bfin_read_PORTE_FER() & ~PE7);
	bfin_write_PORTE_DIR_SET(PE7);
	bfin_write_PORTE_SET(PE7);
	SSYNC();
}
#endif
