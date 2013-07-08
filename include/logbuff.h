/*
 * (C) Copyright 2002-2007
 * Detlev Zundel, dzu@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _LOGBUFF_H
#define _LOGBUFF_H

#ifdef CONFIG_LOGBUFFER

#define LOGBUFF_MAGIC	0xc0de4ced	/* Forced by code, eh!	*/
#define LOGBUFF_LEN	(16384)	/* Must be 16k right now */
#define LOGBUFF_MASK	(LOGBUFF_LEN-1)
#define LOGBUFF_OVERHEAD (4096) /* Logbuffer overhead for extra info */
#define LOGBUFF_RESERVE (LOGBUFF_LEN+LOGBUFF_OVERHEAD)

/* The mapping used here has to be the same as in setup_ext_logbuff ()
   in linux/kernel/printk */

typedef struct {
	union {
		struct {
			unsigned long	tag;
			unsigned long	start;
			unsigned long	con;
			unsigned long	end;
			unsigned long	chars;
		} v2;
		struct {
			unsigned long	dummy;
			unsigned long	tag;
			unsigned long	start;
			unsigned long	size;
			unsigned long	chars;
		} v1;
	};
	unsigned char	buf[0];
} logbuff_t;

int drv_logbuff_init (void);
void logbuff_init_ptrs (void);
void logbuff_log(char *msg);
void logbuff_reset (void);
unsigned long logbuffer_base (void);

#endif /* CONFIG_LOGBUFFER */

#endif /* _LOGBUFF_H */
