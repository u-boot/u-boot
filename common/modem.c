/*
 * (C) Copyright 2002-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/* 'inline' - We have to do it fast */
static inline void mdm_readline(char *buf, int bufsiz)
{
	char c;
	char *p;
	int n;

	n = 0;
	p = buf;
	for(;;) {
		c = serial_getc();

		/*		dbg("(%c)", c); */

		switch(c) {
		case '\r':
			break;
		case '\n':
			*p = '\0';
			return;

		default:
			if(n++ > bufsiz) {
				*p = '\0';
				return; /* sanity check */
			}
			*p = c;
			p++;
			break;
		}
	}
}

extern void  dbg(const char *fmt, ...);
int mdm_init (void)
{
	char env_str[16];
	char *init_str;
	int i;
	extern void enable_putc(void);
	extern int hwflow_onoff(int);

	enable_putc(); /* enable serial_putc() */

#ifdef CONFIG_HWFLOW
	init_str = getenv("mdm_flow_control");
	if (init_str && (strcmp(init_str, "rts/cts") == 0))
		hwflow_onoff (1);
	else
		hwflow_onoff(-1);
#endif

	for (i = 1;;i++) {
		sprintf(env_str, "mdm_init%d", i);
		if ((init_str = getenv(env_str)) != NULL) {
			serial_puts(init_str);
			serial_puts("\n");
			for(;;) {
				mdm_readline(console_buffer, CONFIG_SYS_CBSIZE);
				dbg("ini%d: [%s]", i, console_buffer);

				if ((strcmp(console_buffer, "OK") == 0) ||
					(strcmp(console_buffer, "ERROR") == 0)) {
					dbg("ini%d: cmd done", i);
					break;
				} else /* in case we are originating call ... */
					if (strncmp(console_buffer, "CONNECT", 7) == 0) {
						dbg("ini%d: connect", i);
						return 0;
					}
			}
		} else
			break; /* no init string - stop modem init */

		udelay(100000);
	}

	udelay(100000);

	/* final stage - wait for connect */
	for(;i > 1;) { /* if 'i' > 1 - wait for connection
				  message from modem */
		mdm_readline(console_buffer, CONFIG_SYS_CBSIZE);
		dbg("ini_f: [%s]", console_buffer);
		if (strncmp(console_buffer, "CONNECT", 7) == 0) {
			dbg("ini_f: connected");
			return 0;
		}
	}

	return 0;
}
