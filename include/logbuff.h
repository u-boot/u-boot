/*
 * (C) Copyright 2002
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

#define LOGBUFF_LEN	(16384)	/* Must be 16k right now */
#define LOGBUFF_MASK	(LOGBUFF_LEN-1)
#define LOGBUFF_OVERHEAD (4096) /* Logbuffer overhead for extra info */
#define LOGBUFF_RESERVE (LOGBUFF_LEN+LOGBUFF_OVERHEAD)

#define LOGBUFF_INITIALIZED	(1<<31)

int drv_logbuff_init (void);
void logbuff_init_ptrs (void);
void logbuff_log(char *msg);
void logbuff_reset (void);

#endif /* CONFIG_LOGBUFFER */

#endif /* _LOGBUFF_H */
