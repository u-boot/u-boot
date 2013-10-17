#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <post.h>

struct serial_device {
	/* enough bytes to match alignment of following func pointer */
	char	name[16];

	int	(*start)(void);
	int	(*stop)(void);
	void	(*setbrg)(void);
	int	(*getc)(void);
	int	(*tstc)(void);
	void	(*putc)(const char c);
	void	(*puts)(const char *s);
#if CONFIG_POST & CONFIG_SYS_POST_UART
	void	(*loop)(int);
#endif
	struct serial_device	*next;
};

void default_serial_puts(const char *s);

extern struct serial_device serial_smc_device;
extern struct serial_device serial_scc_device;
extern struct serial_device *default_serial_console(void);

#if	defined(CONFIG_405GP) || \
	defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
	defined(CONFIG_405EX) || defined(CONFIG_440) || \
	defined(CONFIG_MB86R0x) || defined(CONFIG_MPC5xxx) || \
	defined(CONFIG_MPC83xx) || defined(CONFIG_MPC85xx) || \
	defined(CONFIG_MPC86xx) || defined(CONFIG_SYS_SC520) || \
	defined(CONFIG_TEGRA) || defined(CONFIG_SYS_COREBOOT) || \
	defined(CONFIG_MICROBLAZE)
extern struct serial_device serial0_device;
extern struct serial_device serial1_device;
#endif

extern struct serial_device eserial1_device;
extern struct serial_device eserial2_device;

extern void serial_register(struct serial_device *);
extern void serial_initialize(void);
extern void serial_stdio_init(void);
extern int serial_assign(const char *name);
extern void serial_reinit_all(void);

/* For usbtty */
#ifdef CONFIG_USB_TTY

extern int usbtty_getc(void);
extern void usbtty_putc(const char c);
extern void usbtty_puts(const char *str);
extern int usbtty_tstc(void);

#else

/* stubs */
#define usbtty_getc() 0
#define usbtty_putc(a)
#define usbtty_puts(a)
#define usbtty_tstc() 0

#endif /* CONFIG_USB_TTY */

#if defined(CONFIG_MPC512X)
extern struct stdio_dev *open_port(int num, int baudrate);
extern int close_port(int num);
extern int write_port(struct stdio_dev *port, char *buf);
extern int read_port(struct stdio_dev *port, char *buf, int size);
#endif

#endif
