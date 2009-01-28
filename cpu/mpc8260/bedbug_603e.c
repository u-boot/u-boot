/*
 * Bedbug Functions specific to the MPC603e core
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <bedbug/type.h>
#include <bedbug/bedbug.h>
#include <bedbug/regs.h>
#include <bedbug/ppc.h>

#if defined(CONFIG_CMD_BEDBUG) \
	&& (defined(CONFIG_MPC824X) || defined(CONFIG_MPC8260))

#define MAX_BREAK_POINTS 1

extern CPU_DEBUG_CTX bug_ctx;

void bedbug603e_init __P((void));
void bedbug603e_do_break __P((cmd_tbl_t*,int,int,char*[]));
void bedbug603e_break_isr __P((struct pt_regs*));
int  bedbug603e_find_empty __P((void));
int  bedbug603e_set __P((int,unsigned long));
int  bedbug603e_clear __P((int));


/* ======================================================================
 * Initialize the global bug_ctx structure for the processor.  Clear all
 * of the breakpoints.
 * ====================================================================== */

void bedbug603e_init( void )
{
  int	i;
  /* -------------------------------------------------- */

  bug_ctx.hw_debug_enabled = 0;
  bug_ctx.stopped = 0;
  bug_ctx.current_bp = 0;
  bug_ctx.regs = NULL;

  bug_ctx.do_break   = bedbug603e_do_break;
  bug_ctx.break_isr  = bedbug603e_break_isr;
  bug_ctx.find_empty = bedbug603e_find_empty;
  bug_ctx.set        = bedbug603e_set;
  bug_ctx.clear      = bedbug603e_clear;

  for( i = 1; i <= MAX_BREAK_POINTS; ++i )
    (*bug_ctx.clear)( i );

  puts ("BEDBUG:ready\n");
  return;
} /* bedbug_init_breakpoints */



/* ======================================================================
 * Set/clear/show the hardware breakpoint for the 603e.  The "off"
 * string will disable a specific breakpoint.  The "show" string will
 * display the current breakpoints.  Otherwise an address will set a
 * breakpoint at that address.  Setting a breakpoint uses the CPU-specific
 * set routine which will assign a breakpoint number.
 * ====================================================================== */

void bedbug603e_do_break (cmd_tbl_t *cmdtp, int flag, int argc,
			 char *argv[])
{
  long		addr;           /* Address to break at  */
  int		which_bp;       /* Breakpoint number    */
  /* -------------------------------------------------- */

  if (argc < 2)
  {
    cmd_usage(cmdtp);
    return;
  }

  /* Turn off a breakpoint */

  if( strcmp( argv[ 1 ], "off" ) == 0 )
  {
    if( bug_ctx.hw_debug_enabled == 0 )
    {
      puts ( "No breakpoints enabled\n" );
      return;
    }

    which_bp = simple_strtoul( argv[ 2 ], NULL, 10 );

    if( bug_ctx.clear )
      (*bug_ctx.clear)( which_bp );

    printf( "Breakpoint %d removed\n", which_bp );
    return;
  }

  /* Show a list of breakpoints */

  if( strcmp( argv[ 1 ], "show" ) == 0 )
  {
    for( which_bp = 1; which_bp <= MAX_BREAK_POINTS; ++which_bp )
    {

      addr = GET_IABR();

      printf( "Breakpoint [%d]: ", which_bp );
      if( (addr & 0x00000002) == 0 )
	puts ( "NOT SET\n" );
      else
	disppc( (unsigned char *)(addr & 0xFFFFFFFC), 0, 1, bedbug_puts, F_RADHEX );
    }
    return;
  }

  /* Set a breakpoint at the address */

  if(!(( isdigit( argv[ 1 ][ 0 ] )) ||
	(( argv[ 1 ][ 0 ] >= 'a' ) && ( argv[ 1 ][ 0 ] <= 'f' )) ||
	(( argv[ 1 ][ 0 ] >= 'A' ) && ( argv[ 1 ][ 0 ] <= 'F' ))))
  {
    cmd_usage(cmdtp);
    return;
  }

  addr = simple_strtoul( argv[ 1 ], NULL, 16 );

  if(( bug_ctx.set ) && ( which_bp = (*bug_ctx.set)( 0, addr )) > 0 )
  {
    printf( "Breakpoint [%d]: ", which_bp );
    disppc( (unsigned char *)addr, 0, 1, bedbug_puts, F_RADHEX );
  }

  return;
} /* bedbug603e_do_break */



/* ======================================================================
 * Handle a breakpoint.  Enter a mini main loop.  Stay in the loop until
 * the stopped flag in the debug context is cleared.
 * ====================================================================== */

void bedbug603e_break_isr( struct pt_regs *regs )
{
  unsigned long	addr;           /* Address stopped at   */
  /* -------------------------------------------------- */

  bug_ctx.current_bp = 1;
  addr = GET_IABR() & 0xFFFFFFFC;

  bedbug_main_loop( addr, regs );
  return;
} /* bedbug603e_break_isr */



/* ======================================================================
 * See if the hardware breakpoint is available.
 * ====================================================================== */

int bedbug603e_find_empty( void )
{
  /* -------------------------------------------------- */

  if( (GET_IABR() && 0x00000002) == 0 )
    return 1;

  return 0;
} /* bedbug603e_find_empty */



/* ======================================================================
 * Set a breakpoint.  If 'which_bp' is zero then find an unused breakpoint
 * number, otherwise reassign the given breakpoint.  If hardware debugging
 * is not enabled, then turn it on via the MSR and DBCR0.  Set the break
 * address in the IABR register.
 * ====================================================================== */

int bedbug603e_set( int which_bp, unsigned long addr )
{
  /* -------------------------------------------------- */

  if(( addr & 0x00000003 ) != 0 )
  {
    puts ( "Breakpoints must be on a 32 bit boundary\n" );
    return 0;
  }

  /* Only look if which_bp == 0, else use which_bp */
  if(( bug_ctx.find_empty ) && ( !which_bp ) &&
     ( which_bp = (*bug_ctx.find_empty)()) == 0 )
  {
    puts ( "All breakpoints in use\n" );
    return 0;
  }

  if( which_bp < 1 || which_bp > MAX_BREAK_POINTS )
  {
    printf( "Invalid break point # %d\n", which_bp );
    return 0;
  }

  if( ! bug_ctx.hw_debug_enabled )
  {
    bug_ctx.hw_debug_enabled = 1;
  }

  SET_IABR( addr | 0x00000002 );

  return which_bp;
} /* bedbug603e_set */



/* ======================================================================
 * Disable a specific breakoint by setting the IABR register to zero.
 * ====================================================================== */

int bedbug603e_clear( int which_bp )
{
  /* -------------------------------------------------- */

  if( which_bp < 1 || which_bp > MAX_BREAK_POINTS )
  {
    printf( "Invalid break point # (%d)\n", which_bp );
    return -1;
  }

  SET_IABR( 0 );

  return 0;
} /* bedbug603e_clear */


/* ====================================================================== */
#endif
