/*
 * (C) Copyright 2004
 *
 * Pantelis Antoniou <panto@intracom.gr>
 * Intracom S.A.
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

/* Video support for Epson SED156x chipset(s) */

#ifndef SED156X_H
#define SED156X_H

void sed156x_init(void);
void sed156x_clear(void);
void sed156x_output_at(int x, int y, const char *str, int size);
void sed156x_reverse_at(int x, int y, int size);
void sed156x_sync(void);
void sed156x_scroll(int dx, int dy);

/* export display */
extern const int sed156x_text_width;
extern const int sed156x_text_height;

#endif	/* SED156X_H */
