#ifndef __KEYBOARD_H
#define __KEYBOARD_H

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

#endif /* __KEYBOARD_H */
