/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#ifndef __CIRCBUF_H__
#define __CIRCBUF_H__

typedef struct circbuf {
	unsigned int size;	/* current number of bytes held */
	unsigned int totalsize; /* number of bytes allocated */

	char *top;		/* pointer to current buffer start */
	char *tail;		/* pointer to space for next element */

	char *data;		/* all data */
	char *end;		/* end of data buffer */
} circbuf_t;

int buf_init (circbuf_t * buf, unsigned int size);
int buf_free (circbuf_t * buf);
int buf_pop (circbuf_t * buf, char *dest, unsigned int len);
int buf_push (circbuf_t * buf, const char *src, unsigned int len);

#endif
