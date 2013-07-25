/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Replacement (=dummy) for GNU/Linux division-by zero handler */
void __div0 (void)
{
	extern void hang (void);

	hang();
}
