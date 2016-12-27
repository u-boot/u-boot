/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _TIME_H
#define _TIME_H

unsigned long get_timer(unsigned long base);

/*
 * Return the current value of a monotonically increasing microsecond timer.
 * Granularity may be larger than 1us if hardware does not support this.
 */
unsigned long timer_get_us(void);

#endif /* _TIME_H */
