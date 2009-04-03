#ifndef __LINUX_PS2MULT_H
#define __LINUX_PS2MULT_H

#define kbd_request_region()		ps2mult_init()
#define kbd_request_irq(handler)	ps2mult_request_irq(handler)

#define kbd_read_input()		ps2mult_read_input()
#define kbd_read_status()		ps2mult_read_status()
#define kbd_write_output(val)		ps2mult_write_output(val)
#define kbd_write_command(val)		ps2mult_write_command(val)

#define aux_request_irq(hand, dev_id)	0
#define aux_free_irq(dev_id)

#define PS2MULT_KB_SELECTOR		0xA0
#define PS2MULT_MS_SELECTOR		0xA1
#define PS2MULT_ESCAPE			0x7D
#define PS2MULT_BSYNC			0x7E
#define PS2MULT_SESSION_START		0x55
#define PS2MULT_SESSION_END		0x56

#define	PS2BUF_SIZE			512	/* power of 2, please */

#ifndef CONFIG_PS2MULT_DELAY
#define CONFIG_PS2MULT_DELAY	(CONFIG_SYS_HZ/2)	/* Initial delay	*/
#endif

  /* PS/2 controller interface (include/asm/keyboard.h)
   */
extern int ps2mult_init (void);
extern int ps2mult_request_irq(void (*handler)(void *));
extern u_char ps2mult_read_input(void);
extern u_char ps2mult_read_status(void);
extern void ps2mult_write_output(u_char val);
extern void ps2mult_write_command(u_char val);

extern void ps2mult_early_init (void);
extern void ps2mult_callback (int in_cnt);

  /* Simple serial interface
   */
extern int ps2ser_init(void);
extern void ps2ser_putc(int chr);
extern int ps2ser_getc(void);
extern int ps2ser_check(void);


  /* Serial related stuff
   */
struct serial_state {
	int	baud_base;
	int	irq;
	u8	*iomem_base;
};

#endif /* __LINUX_PS2MULT_H */
