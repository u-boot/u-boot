/*
 * (C) Copyright 2005
 * John Otken, jotken@softadvances.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <ppc4xx.h>
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
	char *s = getenv("serial#");

	printf("Board: Luan - AMCC PPC440SP Evaluation Board");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return  0;
}

/*
 * Override the default functions in cpu/ppc4xx/44x_spd_ddr2.c with
 * board specific values.
 */
u32 ddr_clktr(u32 default_val) {
	return (SDRAM_CLKTR_CLKP_180_DEG_ADV);
}

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init( struct pci_controller *hose )
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 *	The luan board is always configured as the host & requires the
	 *	PCI arbiter to be enabled.
	 *--------------------------------------------------------------------------*/
	mfsdr(SDR0_SDSTP1, strap);
	if( (strap & SDR0_SDSTP1_PAE_MASK) == 0 ) {
		printf("PCI: SDR0_STRP1[%08lX] - PCI Arbiter disabled.\n",strap);

		return  0;
	}

	return  1;
}
#endif /* defined(CONFIG_PCI) */


/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r( PCIL0_PIM0SA, 0 ); /* disable */
	out32r( PCIL0_PIM1SA, 0 ); /* disable */
	out32r( PCIL0_PIM2SA, 0 ); /* disable */
	out32r( PCIL0_EROMBA, 0 ); /* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r( PCIL0_PIM0LAL, CONFIG_SYS_SDRAM_BASE );
	out32r( PCIL0_PIM0LAH, 0 );
	out32r( PCIL0_PIM0SA, ~(gd->ram_size - 1) | 1 );

	out32r( PCIL0_BAR0, 0 );

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r( PCIL0_SBSYSVID, CONFIG_SYS_PCI_SUBSYS_VENDORID );
	out16r( PCIL0_SBSYSID, CONFIG_SYS_PCI_SUBSYS_DEVICEID );

	out16r( PCIL0_CMD, in16r(PCIL0_CMD) | PCI_COMMAND_MEMORY );
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */


/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	return  1;
}
#endif				/* defined(CONFIG_PCI) */


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
int do_l2cache( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
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
		cmd_usage(cmdtp);
		return 1;
	}

	return  0;
}


U_BOOT_CMD(
	l2cache,   2,   1,     do_l2cache,
	"enable or disable L2 cache",
	"[on, off]\n"
	"    - enable or disable L2 cache"
);
