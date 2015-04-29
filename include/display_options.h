/*
 * Copyright (c) 2015 Google, Inc
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DISPLAY_OPTIONS_H
#define __DISPLAY_OPTIONS_H

/**
 * print_size() - Print a size with a suffix
 *
 * Print sizes as "xxx KiB", "xxx.y KiB", "xxx MiB", "xxx.y MiB",
 * xxx GiB, xxx.y GiB, etc as needed; allow for optional trailing string
 * (like "\n")
 *
 * @size:	Size to print
 * @suffix	String to print after the size
 */
void print_size(uint64_t size, const char *suffix);

/**
 * print_freq() - Print a frequency with a suffix
 *
 * Print frequencies as "x.xx GHz", "xxx KHz", etc as needed; allow for
 * optional trailing string (like "\n")
 *
 * @freq:	Frequency to print in Hz
 * @suffix	String to print after the frequency
 */
void print_freq(uint64_t freq, const char *suffix);

/**
 * print_buffer() - Print data buffer in hex and ascii form
 *
 * Data reads are buffered so that each memory address is only read once.
 * This is useful when displaying the contents of volatile registers.
 *
 * @addr:	Starting address to display at start of line
 * @data:	pointer to data buffer
 * @width:	data value width.  May be 1, 2, or 4.
 * @count:	number of values to display
 * @linelen:	Number of values to print per line; specify 0 for default length
 */
int print_buffer(ulong addr, const void *data, uint width, uint count,
		 uint linelen);

/**
 * display_options() - display the version string / build tag
 *
 * This displays the U-Boot version string. If a build tag is available this
 * is displayed also.
 */
int display_options(void);

#endif
