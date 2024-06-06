/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/**
 * DOC: Overview
 *
 * Status LED is a Low-Level way to handle LEDs to signal state of the
 * bootloader, for example boot progress, file transfer fail, activity
 * of some sort like tftp transfer, mtd write/erase.
 *
 * The original usage these API were to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 *
 * Status LED require support Low Level and the board to implement
 * the specific functions to correctly works.
 */

#ifndef _STATUS_LED_H_
#define	_STATUS_LED_H_

/**
 * DOC: CONFIG Description
 *
 * Enable `CONFIG_LED_STATUS` to support the Status LED under
 * > Device Drivers > LED Support.
 *
 * The Status LED can be defined in various ways, but most of the time,
 * specific functions will need to be defined in the board file.
 * If this is the case, enable `CONFIG_LED_STATUS_BOARD_SPECIFIC`.
 *
 * If the LEDs are GPIO-driven, you can use the GPIO wrapper driver
 * instead of defining specific board functions.
 * If this is the case, enable `CONFIG_LED_STATUS_GPIO`.
 * (Note that `CONFIG_LED_STATUS_BOARD_SPECIFIC` is also required.)
 *
 * The Status LED allows defining up to six different LEDs, from 0 to 5,
 * with the following configurations:
 * `CONFIG_STATUS_LED`, `CONFIG_STATUS_LED1`, ..., `CONFIG_STATUS_LED5`.
 *
 * For each LED, the following options are required:
 *  * `CONFIG_STATUS_LED_BIT<n>`
 *  * `CONFIG_STATUS_LED_STATE<n>`
 *  * `CONFIG_STATUS_LED_FREQ<n>`
 *
 * Where `<n>` is an integer from 1 through 5. (Note that LED 0 doesn't have the
 * integer suffix.)
 *
 * `CONFIG_STATUS_LED_BIT` is passed into the `__led_*` functions to identify
 * which LED is being acted on. The value is opaque, meaning it depends on how
 * the low-level API handles this value. It can be an ID to reference the LED
 * internally, or in the case of the GPIO wrapper, it's the GPIO number of the LED.
 * Mapping the value to a physical LED is the responsibility of the `__led_*` function.
 *
 * `CONFIG_STATUS_LED_STATE` sets the initial state of the LED. It should be set
 * to one of these values: `CONFIG_LED_STATUS_OFF` or `CONFIG_LED_STATUS_ON`.
 *
 * `CONFIG_STATUS_LED_FREQ` determines the LED blink frequency.
 * Values range from 2 to 10.
 */
/**
 * DOC: LED Status Config
 *
 * The Status LED uses two special configurations for common operations:
 *
 *   * CONFIG_STATUS_LED_BOOT is the LED that lights up when the board is booting.
 *   * CONFIG_STATUS_LED_ACTIVITY is the LED that lights and blinks during activity
 *     (e.g., file transfer, flash write).
 *
 * The values set in these configurations refer to the LED ID previously set.
 *
 * - ``CONFIG_STATUS_LED_RED=0`` will refer to the option ``CONFIG_STATUS_LED_BIT``.
 * - ``CONFIG_STATUS_LED_RED=1`` will refer to the option ``CONFIG_STATUS_LED_BIT1``.
 * - ``CONFIG_STATUS_LED_RED=2`` will refer to the option ``CONFIG_STATUS_LED_BIT2``.
 * - ...
 */
/**
 * DOC: LED Colour Config
 *
 * The Status LED exposes specific configurations for LEDs of different colors.
 *
 * The values set in these configurations refer to the LED ID previously set.
 *
 * - ``CONFIG_STATUS_LED_RED=0`` will refer to the option ``CONFIG_STATUS_LED_BIT``.
 * - ``CONFIG_STATUS_LED_RED=1`` will refer to the option ``CONFIG_STATUS_LED_BIT1``.
 * - ``CONFIG_STATUS_LED_RED=2`` will refer to the option ``CONFIG_STATUS_LED_BIT2``.
 * - ...
 *
 * Supported colors are:
 *   * red
 *   * green
 *   * yellow
 *   * blue
 *   * white
 */

#ifdef CONFIG_LED_STATUS

#define LED_STATUS_PERIOD	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ)
#ifdef CONFIG_LED_STATUS1
#define LED_STATUS_PERIOD1	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ1)
#endif /* CONFIG_LED_STATUS1 */
#ifdef CONFIG_LED_STATUS2
#define LED_STATUS_PERIOD2	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ2)
#endif /* CONFIG_LED_STATUS2 */
#ifdef CONFIG_LED_STATUS3
#define LED_STATUS_PERIOD3	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ3)
#endif /* CONFIG_LED_STATUS3 */
#ifdef CONFIG_LED_STATUS4
#define LED_STATUS_PERIOD4	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ4)
#endif /* CONFIG_LED_STATUS4 */
#ifdef CONFIG_LED_STATUS5
#define LED_STATUS_PERIOD5	(CONFIG_SYS_HZ / CONFIG_LED_STATUS_FREQ5)
#endif /* CONFIG_LED_STATUS5 */

/**
 * void status_led_init - Init the Status LED with all the required structs.
 */
void status_led_init(void);
/**
 * void status_led_tick - Blink each LED that is currently set in blinking
 *   mode
 * @timestamp: currently unused
 */
void status_led_tick(unsigned long timestamp);
/**
 * void status_led_set - Set the LED ID passed as first arg to the mode set
 * @led: reference to the Status LED ID
 * @state: state to set the LED to
 *
 * Modes for state arE:
 *   * CONFIG_LED_STATUS_OFF (LED off)
 *   * CONFIG_LED_STATUS_ON (LED on)
 *   * CONFIG_LED_STATUS_BLINK (LED initially on, expected to blink)
 */
void status_led_set(int led, int state);
/**
 * void status_led_toggle - toggle the LED ID
 * @led: reference to the Status LED ID
 *
 * Toggle the LED ID passed as first arg. If it's ON became OFF, if it's
 * OFF became ON.
 */
void status_led_toggle(int led);
/**
 * void status_led_activity_start - start a LED activity
 * @led: reference to the Status LED ID
 *
 * Set the Status LED ON and start the Cyclic to make the LED blink at
 * the configured freq.
 */
void status_led_activity_start(int led);
/**
 * void status_led_activity_stop - stop a LED activity
 * @led: reference to the Status LED ID
 *
 * Stop and free the Cyclic and turn the LED OFF.
 */
void status_led_activity_stop(int led);

/*****  MVS v1  **********************************************************/
#if (defined(CONFIG_MVS) && CONFIG_MVS < 2)
# define STATUS_LED_PAR		im_ioport.iop_pdpar
# define STATUS_LED_DIR		im_ioport.iop_pddir
# undef  STATUS_LED_ODR
# define STATUS_LED_DAT		im_ioport.iop_pddat

# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1	*/

/*****  Someone else defines these  *************************************/
#elif defined(STATUS_LED_PAR)
  /*
   * ADVICE: Define in your board configuration file rather than
   * filling this file up with lots of custom board stuff.
   */

#elif defined(CONFIG_LED_STATUS_BOARD_SPECIFIC)
/* led_id_t is unsigned long mask */
typedef unsigned long led_id_t;

/**
 * DOC: Required API
 *
 * The Status LED requires the following API functions to operate correctly
 * and compile:
 *
 * - ``__led_toggle``: Low-level function to toggle the LED at the specified
 *   mask.
 * - ``__led_init``: Initializes the Status LED, sets up required tables, and
 *   configures registers.
 * - ``__led_set``: Low-level function to set the state of the LED at the
 *   specified mask.
 * - ``__led_blink``: Low-level function to set the LED to blink at the
 *   specified frequency.
 *
 * The Status LED also provides optional functions to control colored LEDs.
 * Each supported LED color has corresponding ``_on`` and ``_off`` functions.
 *
 * There is also support for ``coloured_LED_init`` for LEDs that provide
 * multiple colors. (Currently, this is only used by ARM.)
 *
 * Each function is weakly defined and should be implemented in the
 * board-specific source file. (This does not apply to the GPIO LED wrapper.)
 */
/**
 * void __led_toggle - toggle LED at @mask
 * @mask: opaque value to reference the LED
 *
 * Low-Level function to toggle the LED at mask.
 */
extern void __led_toggle (led_id_t mask);
/**
 * void __led_init - Init the LED at @mask
 * @mask: opaque value to reference the LED
 * @state: starting state of the LED
 *
 * Init the Status LED, init required tables, setup regs...
 */
extern void __led_init (led_id_t mask, int state);
/**
 * void __led_set - Set the LED at @mask
 * @mask: opaque value to reference the LED
 * @state: state to set the LED to
 *
 * Init the Status LED, init required tables, setup regs...
 */
extern void __led_set (led_id_t mask, int state);
/**
 * void __led_blink - Set the LED at @mask to HW blink
 * @mask: opaque value to reference the LED
 * @freq: freq to make the LED blink at
 *
 * Low-Level function to set the LED at HW blink by the
 * passed freq.
 */
void __led_blink(led_id_t mask, int freq);
#else
# error Status LED configuration missing
#endif
/************************************************************************/

#ifndef CONFIG_LED_STATUS_BOARD_SPECIFIC
# include <asm/status_led.h>
#endif

#endif	/* CONFIG_LED_STATUS	*/

/**
 * DOC: Coloured LED API
 *
 * Status LED expose optional functions to control coloured LED.
 * Each LED color supported expose _on and _off function.
 *
 * There is also support for coloured_LED_init for LED that provide
 * multiple colours. (currently only used by ARM)
 *
 * Each function is weakly defined and should be defined in the board
 * specific source. (doesn't apply for GPIO LED wrapper)
 */
#ifndef	__ASSEMBLY__
/**
 * void coloured_LED_init - Init multi colour LED
 */
void coloured_LED_init(void);
/**
 * void red_led_on - Turn LED Red on
 */
void red_led_on(void);
/**
 * void red_led_off - Turn LED Red off
 */
void red_led_off(void);
/**
 * void green_led_on - Turn LED Green on
 */
void green_led_on(void);
/**
 * void green_led_off - Turn LED Green off
 */
void green_led_off(void);
/**
 * void yellow_led_on - Turn LED Yellow on
 */
void yellow_led_on(void);
/**
 * void yellow_led_off - Turn LED Yellow off
 */
void yellow_led_off(void);
/**
 * void blue_led_on - Turn LED Blue on
 */
void blue_led_on(void);
/**
 * void blue_led_off - Turn LED Blue off
 */
void blue_led_off(void);
/**
 * void white_led_on - Turn LED White on
 */
void white_led_on(void);
/**
 * void white_led_off - Turn LED White off
 */
void white_led_off(void);
#else
	.extern LED_init
	.extern red_led_on
	.extern red_led_off
	.extern yellow_led_on
	.extern yellow_led_off
	.extern green_led_on
	.extern green_led_off
	.extern blue_led_on
	.extern blue_led_off
	.extern white_led_on
	.extern white_led_off
#endif

#endif	/* _STATUS_LED_H_	*/
