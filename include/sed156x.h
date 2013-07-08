/*
 * (C) Copyright 2004
 *
 * Pantelis Antoniou <panto@intracom.gr>
 * Intracom S.A.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
