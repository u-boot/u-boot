/*
 * ppmc7xx.c
 * ---------
 *
 * Main board-specific routines for Wind River PPMC 7xx/74xx board.
 *
 * By Richard Danter (richard.danter@windriver.com)
 * Copyright (C) 2005 Wind River Systems
 */

#include <common.h>
#include <command.h>
#include <netdev.h>


/* Define some MPC107 (memory controller) registers */
#define MPC107_EUMB_GCR         0xfce41020
#define MPC107_EUMB_IACKR       0xfce600a0


/* Function prototypes */
extern void _start(void);


/*
 * initdram()
 *
 * This function normally initialises the (S)DRAM of the system. For this board
 * the SDRAM was already initialised by board_asm_init (see init.S) so we just
 * return the size of RAM.
 */
phys_size_t initdram( int board_type )
{
    return CONFIG_SYS_SDRAM_SIZE;
}


/*
 * after_reloc()
 *
 * This is called after U-Boot has been copied from Flash/ROM to RAM. It gives
 * us an opportunity to do some additional setup before the rest of the system
 * is initialised. We don't need to do anything, so we just call board_init_r()
 * which should never return.
 */
void after_reloc( ulong dest_addr, gd_t* gd )
{
	/* Jump to the main U-Boot board init code */
	board_init_r( gd, dest_addr );
}


/*
 * checkboard()
 *
 * We could do some board level checks here, such as working out what version
 * it is, but for this board we simply display it's name (on the console).
 */
int checkboard( void )
{
    puts( "Board: Wind River PPMC 7xx/74xx\n" );
    return 0;
}


/*
 * misc_init_r
 *
 * Used for other setup which needs to be done late in the bring-up phase.
 */
int misc_init_r( void )
{
	/* Reset the EPIC and clear pending interrupts */
	out32r(MPC107_EUMB_GCR, 0xa0000000);
	while( in32r( MPC107_EUMB_GCR ) & 0x80000000 );
	out32r( MPC107_EUMB_GCR, 0x20000000 );
	while( in32r( MPC107_EUMB_IACKR ) != 0xff );

	/* Enable the I-Cache */
	icache_enable();

	return 0;
}


/*
 * do_reset()
 *
 * Shell command to reset the board.
 */
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf( "Resetting...\n" );

	/* Disabe and invalidate cache */
	icache_disable();
	dcache_disable();

	/* Jump to cold reset point (in RAM) */
	_start();

	/* Should never get here */
	while(1)
		;

	return 1;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
