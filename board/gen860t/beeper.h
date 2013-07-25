/*
 * (C) Copyright 2002
 * Keith Outwater, keith_outwater@mvis.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

void init_beeper(void);
void set_beeper_frequency(uint frequency);
void beeper_on(void);
void beeper_off(void);
void set_beeper_volume(int steps);
int do_beeper(char *sequence);
