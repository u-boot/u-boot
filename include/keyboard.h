#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#ifdef CONFIG_DM_KEYBOARD
#include <input.h>
#include <stdio_dev.h>

/**
 * struct keyboard_priv - information about a keyboard, for the uclass
 *
 * @sdev:	stdio device
 * @input:	input configuration (the driver may use this if desired)
 */
struct keyboard_priv {
	struct stdio_dev sdev;

	/*
	 * This is set up by the uclass but will only be used if the driver
	 * sets input.dev to its device pointer (it is initially NULL).
	 */
	struct input_config input;
};

/**
 * struct keyboard_ops - keyboard device operations
 */
struct keyboard_ops {
	/**
	 * start() - enable the keyboard ready for use
	 *
	 * @dev:	Device to enable
	 * @return 0 if OK, -ve on error
	 */
	int (*start)(struct udevice *dev);

	/**
	 * stop() - disable the keyboard when no-longer needed
	 *
	 * @dev:	Device to disable
	 * @return 0 if OK, -ve on error
	 */
	int (*stop)(struct udevice *dev);

	/**
	 * tstc() - check if a key is available
	 *
	 * @dev:	Device to check
	 * @return 0 if no key is available, 1 if a key is available, -ve on
	 *	   error
	 */
	int (*tstc)(struct udevice *dev);

	/**
	 * getc() - get a key
	 *
	 * TODO(sjg@chromium.org): At present this method may wait if it calls
	 * input_getc().
	 *
	 * @dev:	Device to read from
	 * @return -EAGAIN if no key is available, otherwise key value read
	 *	   (as ASCII).
	 */
	int (*getc)(struct udevice *dev);

	/**
	 * update_leds() - update keyboard LEDs
	 *
	 * This is called when the LEDs have changed and need to be updated.
	 * For example, if 'caps lock' is pressed then this method will be
	 * called with the new LED value.
	 *
	 * @dev:	Device to update
	 * @leds:	New LED mask (see INPUT_LED_... in input.h)
	 */
	int (*update_leds)(struct udevice *dev, int leds);
};

#define keyboard_get_ops(dev)	((struct keyboard_ops *)(dev)->driver->ops)

#else

#ifdef CONFIG_PS2MULT
#include <ps2mult.h>
#endif

#if !defined(kbd_request_region) || \
    !defined(kbd_request_irq) || \
    !defined(kbd_read_input) || \
    !defined(kbd_read_status) || \
    !defined(kbd_write_output) || \
    !defined(kbd_write_command)
#error PS/2 low level routines not defined
#endif

extern int kbd_init (void);
extern void handle_scancode(unsigned char scancode);
extern int kbd_init_hw(void);
extern void pckbd_leds(unsigned char leds);
#endif /* !CONFIG_DM_KEYBOARD */

#if defined(CONFIG_MPC5xxx) || defined(CONFIG_ARCH_MPC8540) || \
		defined(CONFIG_ARCH_MPC8541) || defined(CONFIG_ARCH_MPC8555)
int ps2ser_check(void);
#endif

#endif /* __KEYBOARD_H */
