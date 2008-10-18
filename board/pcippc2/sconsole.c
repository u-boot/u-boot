/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>

#include "sconsole.h"

DECLARE_GLOBAL_DATA_PTR;

void	(*sconsole_putc) (char) = 0;
void	(*sconsole_puts) (const char *) = 0;
int	(*sconsole_getc) (void) = 0;
int	(*sconsole_tstc) (void) = 0;
void	(*sconsole_setbrg) (void) = 0;

int serial_init (void)
{
	sconsole_buffer_t *sb = SCONSOLE_BUFFER;

	sb->pos  = 0;
	sb->size = 0;
	sb->baud = gd->baudrate;
	sb->max_size = CONFIG_SYS_SCONSOLE_SIZE - sizeof (sconsole_buffer_t);

	return (0);
}

void serial_putc (char c)
{
	if (sconsole_putc) {
		(*sconsole_putc) (c);
	} else {
		sconsole_buffer_t *sb = SCONSOLE_BUFFER;

		if (c) {
			sb->data[sb->pos++] = c;
			if (sb->pos == sb->max_size) {
				sb->pos = 0;
			}
			if (sb->size < sb->max_size) {
				sb->size++;
			}
		}
	}
}

void serial_puts (const char *s)
{
	if (sconsole_puts) {
		(*sconsole_puts) (s);
	} else {
		sconsole_buffer_t *sb = SCONSOLE_BUFFER;

		while (*s) {
			sb->data[sb->pos++] = *s++;
			if (sb->pos == sb->max_size) {
				sb->pos = 0;
			}
			if (sb->size < sb->max_size) {
				sb->size++;
			}
		}
	}
}

int serial_getc (void)
{
	if (sconsole_getc) {
		return (*sconsole_getc) ();
	} else {
		return 0;
	}
}

int serial_tstc (void)
{
	if (sconsole_tstc) {
		return (*sconsole_tstc) ();
	} else {
		return 0;
	}
}

void serial_setbrg (void)
{
	if (sconsole_setbrg) {
		(*sconsole_setbrg) ();
	} else {
		sconsole_buffer_t *sb = SCONSOLE_BUFFER;

		sb->baud = gd->baudrate;
	}
}

int sconsole_get_baudrate (void)
{
	sconsole_buffer_t *sb = SCONSOLE_BUFFER;

	return sb->baud;
}

void sconsole_flush (void)
{
	if (sconsole_putc) {
		sconsole_buffer_t *sb = SCONSOLE_BUFFER;
		unsigned int end = sb->pos < sb->size
				? sb->pos + sb->max_size - sb->size
				: sb->pos - sb->size;

		while (sb->size) {
			(*sconsole_putc) (sb->data[end++]);
			if (end == sb->max_size) {
				end = 0;
			}
			sb->size--;
		}
	}
}
