/*
 * needed for cpu/mpc512x/start.S
 *
 * These should be auto-generated
 */
#define LPCS0AW			0x0024
#define SRAMBAR			0x00C4
#define SWCRR			0x0904
#define LPC_OFFSET		0x10000
#define CS0_CONFIG		0x00000
#define CS_CTRL			0x00020
#define CS_CTRL_ME		0x01000000	/* CS Master Enable bit */

#define EXC_OFF_SYS_RESET	0x0100
#define	_START_OFFSET		EXC_OFF_SYS_RESET
