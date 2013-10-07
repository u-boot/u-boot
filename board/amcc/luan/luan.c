/*
 * (C) Copyright 2005
 * John Otken, jotken@softadvances.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/ppc4xx-isram.h>
#include <spd_sdram.h>
#include "epld.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */


/*************************************************************************
 *  int board_early_init_f()
 *
 ************************************************************************/
int board_early_init_f(void)
{
	u32 mfr;

	mtebc( PB0AP,  0x03800000 );	/* set chip selects */
	mtebc( PB0CR,  0xffc58000 );	/* ebc0_b0cr, 4MB at 0xffc00000 CS0 */
	mtebc( PB1AP,  0x03800000 );
	mtebc( PB1CR,  0xff018000 );	/* ebc0_b1cr, 1MB at 0xff000000 CS1 */
	mtebc( PB2AP,  0x03800000 );
	mtebc( PB2CR,  0xff838000 );	/* ebc0_b2cr, 2MB at 0xff800000 CS2 */

	mtdcr( UIC1SR, 0xffffffff );	/* Clear all interrupts */
	mtdcr( UIC1ER, 0x00000000 );	/* disable all interrupts */
	mtdcr( UIC1CR, 0x00000000 );	/* Set Critical / Non Critical interrupts */
	mtdcr( UIC1PR, 0x7fff83ff );	/* Set Interrupt Polarities */
	mtdcr( UIC1TR, 0x001f8000 );	/* Set Interrupt Trigger Levels */
	mtdcr( UIC1VR, 0x00000001 );	/* Set Vect base=0,INT31 Highest priority */
	mtdcr( UIC1SR, 0x00000000 );	/* clear all interrupts */
	mtdcr( UIC1SR, 0xffffffff );

	mtdcr( UIC0SR, 0xffffffff );	/* Clear all interrupts */
	mtdcr( UIC0ER, 0x00000000 );	/* disable all interrupts excepted cascade */
	mtdcr( UIC0CR, 0x00000001 );	/* Set Critical / Non Critical interrupts */
	mtdcr( UIC0PR, 0xffffffff );	/* Set Interrupt Polarities */
	mtdcr( UIC0TR, 0x01000004 );	/* Set Interrupt Trigger Levels */
	mtdcr( UIC0VR, 0x00000001 );	/* Set Vect base=0,INT31 Highest priority */
	mtdcr( UIC0SR, 0x00000000 );	/* clear all interrupts */
	mtdcr( UIC0SR, 0xffffffff );

	mfsdr(SDR0_MFR, mfr);
	mfr |= SDR0_MFR_FIXD;		/* Workaround for PCI/DMA */
	mtsdr(SDR0_MFR, mfr);

	return  0;
}


/*************************************************************************
 *  int misc_init_r()
 *
 ************************************************************************/
int misc_init_r(void)
{
	volatile epld_t *x = (epld_t *) CONFIG_SYS_EPLD_BASE;

	/* set modes of operation */
	x->ethuart |= EPLD2_ETH_MODE_10 | EPLD2_ETH_MODE_100 |
		EPLD2_ETH_MODE_1000 | EPLD2_ETH_DUPLEX_MODE;
	/* clear ETHERNET_AUTO_NEGO bit to turn on autonegotiation */
	x->ethuart &= ~EPLD2_ETH_AUTO_NEGO;

	/* put Ethernet+PHY in reset */
	x->ethuart &= ~EPLD2_RESET_ETH_N;
	udelay(10000);
	/* take Ethernet+PHY out of reset */
	x->ethuart |= EPLD2_RESET_ETH_N;

	return  0;
}


/*************************************************************************
 *  int checkboard()
 *
 ************************************************************************/
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: Luan - AMCC PPC440SP Evaluation Board");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return  0;
}

/*
 * Override the default functions in arch/powerpc/cpu/ppc4xx/44x_spd_ddr2.c with
 * board specific values.
 */
u32 ddr_clktr(u32 default_val) {
	return (SDRAM_CLKTR_CLKP_180_DEG_ADV);
}

/*************************************************************************
 *  hw_watchdog_reset
 *
 *	This routine is called to reset (keep alive) the watchdog timer
 *
 ************************************************************************/
#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
}
#endif


/*************************************************************************
 *  int on_off()
 *
 ************************************************************************/
static int on_off( const char *s )
{
	if (strcmp(s, "on") == 0) {
		return  1;
	} else if (strcmp(s, "off") == 0) {
		return  0;
	}
	return  -1;
}


/*************************************************************************
 *  void l2cache_disable()
 *
 ************************************************************************/
static void l2cache_disable(void)
{
	mtdcr( L2_CACHE_CFG, 0 );
}


/*************************************************************************
 *  void l2cache_enable()
 *
 ************************************************************************/
static void l2cache_enable(void)	/* see p258 7.4.1 Enabling L2 Cache */
{
	mtdcr( L2_CACHE_CFG, 0x80000000 );	/* enable L2_MODE L2_CFG[L2M] */

	mtdcr( L2_CACHE_ADDR, 0 );		/* set L2_ADDR with all zeros */

	mtdcr( L2_CACHE_CMD, 0x80000000 );	/* issue HCLEAR command via L2_CMD */

	while (!(mfdcr( L2_CACHE_STAT ) & 0x80000000 ))  ;; /* poll L2_SR for completion */

	mtdcr( L2_CACHE_CMD, 0x10000000 );	/* clear cache errors L2_CMD[CCP] */

	mtdcr( L2_CACHE_CMD, 0x08000000 );	/* clear tag errors L2_CMD[CTE] */

	mtdcr( L2_CACHE_SNP0, 0 );		/* snoop registers */
	mtdcr( L2_CACHE_SNP1, 0 );

	__asm__ volatile ("sync");		/* msync */

	mtdcr( L2_CACHE_CFG, 0xe0000000 );	/* inst and data use L2 */

	__asm__ volatile ("sync");
}


/*************************************************************************
 *  int l2cache_status()
 *
 ************************************************************************/
static int l2cache_status(void)
{
	return  (mfdcr( L2_CACHE_CFG ) & 0x60000000) != 0;
}


/*************************************************************************
 *  int do_l2cache()
 *
 ************************************************************************/
int do_l2cache( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[] )
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (on_off(argv[1])) {
		case 0:	l2cache_disable();
			break;
		case 1:	l2cache_enable();
			break;
		}
		/* FALL TROUGH */
	case 1:			/* get status */
		printf ("L2 Cache is %s\n",
			l2cache_status() ? "ON" : "OFF");
		return 0;
	default:
		return cmd_usage(cmdtp);
	}

	return  0;
}


U_BOOT_CMD(
	l2cache,   2,   1,     do_l2cache,
	"enable or disable L2 cache",
	"[on, off]\n"
	"    - enable or disable L2 cache"
);
