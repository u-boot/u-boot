/*
 * Semi-blindly hacked together.
 */

#ifndef __DRAGONFIRE_H__
#define __DRAGONFIRE_H__

/* UART base addresses */
#define DRAGONFIRE_UART0_BASE 0xE0000000
#define DRAGONFIRE_UART1_BASE 0xE0001000

#ifdef PELE_TTC
	/* For PELE the base address has changed*/
	#define DRAGONFIRE_TTC0_BASE 0x90001000
	#define DRAGONFIRE_TTC1_BASE 0x90002000

#else

	/* Timers forf DFE */
	#define DRAGONFIRE_TTC0_BASE 0xFE001000
	#define DRAGONFIRE_TTC1_BASE 0xFE002000
#endif

/* GEMs */
#define DRAGONFIRE_GEM0_BASE 0xE000B000
#define DRAGONFIRE_GEM1_BASE 0xE000C000

#endif /* __DRAGONFIRE_H__ */
