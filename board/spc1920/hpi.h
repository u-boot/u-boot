/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, DENX Software Engineering, mk@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

int hpi_init(void);

#ifdef CONFIG_SPC1920_HPI_TEST
int hpi_test(void);
#endif
