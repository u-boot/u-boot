/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ZOOM2_SERIAL_H
#define ZOOM2_SERIAL_H

#include <linux/stringify.h>

extern int zoom2_debug_board_connected (void);

#define SERIAL_TL16CP754C_BASE	0x10000000	/* Zoom2 Serial chip address */

#define QUAD_BASE_0	SERIAL_TL16CP754C_BASE
#define QUAD_BASE_1	(SERIAL_TL16CP754C_BASE + 0x100)
#define QUAD_BASE_2	(SERIAL_TL16CP754C_BASE + 0x200)
#define QUAD_BASE_3	(SERIAL_TL16CP754C_BASE + 0x300)

#define QUAD_INIT(n)				\
int quad_init_##n(void)				\
{						\
	return quad_init_dev(QUAD_BASE_##n);	\
}						\
void quad_setbrg_##n(void)			\
{						\
	quad_setbrg_dev(QUAD_BASE_##n);		\
}						\
void quad_putc_##n(const char c)		\
{						\
	quad_putc_dev(QUAD_BASE_##n, c);	\
}						\
void quad_puts_##n(const char *s)		\
{						\
	quad_puts_dev(QUAD_BASE_##n, s);	\
}						\
int quad_getc_##n(void)				\
{						\
	return quad_getc_dev(QUAD_BASE_##n);	\
}						\
int quad_tstc_##n(void)				\
{						\
	return quad_tstc_dev(QUAD_BASE_##n);	\
}						\
struct serial_device zoom2_serial_device##n =	\
{						\
	.name	= __stringify(n),		\
	.start	= quad_init_##n,		\
	.stop	= NULL,				\
	.setbrg	= quad_setbrg_##n,		\
	.getc	= quad_getc_##n,		\
	.tstc	= quad_tstc_##n,		\
	.putc	= quad_putc_##n,		\
	.puts	= quad_puts_##n,		\
};

#endif /* ZOOM2_SERIAL_H */
