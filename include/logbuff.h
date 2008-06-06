/*
 * (C) Copyright 2002-2007
 * Detlev Zundel, dzu@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
