#ifndef __SERIAL_H__
#define __SERIAL_H__

#define NAMESIZE 16
#define CTLRSIZE 8

struct serial_device {
	char name[NAMESIZE];
	char ctlr[CTLRSIZE];

	int  (*init) (void);
	void (*setbrg) (void);
	int (*getc) (void);
	int (*tstc) (void);
	void (*putc) (const char c);
	void (*puts) (const char *s);

	struct serial_device *next;
};

extern struct serial_device serial_smc_device;
extern struct serial_device serial_scc_device;
extern struct serial_device * default_serial_console (void);

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_440) || \
    defined(CONFIG_405EP) || defined(CONFIG_405EZ) || defined(CONFIG_405EX) || \
    defined(CONFIG_MPC5xxx)
extern struct serial_device serial0_device;
extern struct serial_device serial1_device;
#if defined(CFG_NS16550_SERIAL)
extern struct serial_device eserial1_device;
extern struct serial_device eserial2_device;
extern struct serial_device eserial3_device;
extern struct serial_device eserial4_device;
#endif /* CFG_NS16550_SERIAL */

#endif


extern struct serial_device serial_ffuart_device;
extern struct serial_device serial_btuart_device;
extern struct serial_device serial_stuart_device;

extern void serial_initialize(void);
extern void serial_devices_init(void);
extern int serial_assign(char * name);
extern void serial_reinit_all(void);

#endif
