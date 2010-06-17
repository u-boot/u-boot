/*
 * TNETV107X: Watchdog timer implementation (for reset)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

#define MAX_DIV		0xFFFE0001

struct wdt_regs {
	u32 kick_lock;
#define KICK_LOCK_1	0x5555
#define KICK_LOCK_2	0xaaaa
	u32 kick;

	u32 change_lock;
#define CHANGE_LOCK_1	0x6666
#define CHANGE_LOCK_2	0xbbbb
	u32 change;

	u32 disable_lock;
#define DISABLE_LOCK_1	0x7777
#define DISABLE_LOCK_2	0xcccc
#define DISABLE_LOCK_3	0xdddd
	u32 disable;

	u32 prescale_lock;
#define PRESCALE_LOCK_1	0x5a5a
#define PRESCALE_LOCK_2	0xa5a5
	u32 prescale;
};

static struct wdt_regs* regs = (struct wdt_regs *)TNETV107X_WDT0_ARM_BASE;

#define wdt_reg_read(reg)	__raw_readl(&regs->reg)
#define wdt_reg_write(reg, val)	__raw_writel((val), &regs->reg)

static int write_prescale_reg(unsigned long prescale_value)
{
	wdt_reg_write(prescale_lock, PRESCALE_LOCK_1);
	if ((wdt_reg_read(prescale_lock) & 0x3) != 0x1)
		return -1;

	wdt_reg_write(prescale_lock, PRESCALE_LOCK_2);
	if ((wdt_reg_read(prescale_lock) & 0x3) != 0x3)
		return -1;

	wdt_reg_write(prescale, prescale_value);

	return 0;
}

static int write_change_reg(unsigned long initial_timer_value)
{
	wdt_reg_write(change_lock, CHANGE_LOCK_1);
	if ((wdt_reg_read(change_lock) & 0x3) != 0x1)
		return -1;

	wdt_reg_write(change_lock, CHANGE_LOCK_2);
	if ((wdt_reg_read(change_lock) & 0x3) != 0x3)
		return -1;

	wdt_reg_write(change, initial_timer_value);

	return 0;
}

static int wdt_control(unsigned long disable_value)
{
	wdt_reg_write(disable_lock, DISABLE_LOCK_1);
	if ((wdt_reg_read(disable_lock) & 0x3) != 0x1)
		return -1;

	wdt_reg_write(disable_lock, DISABLE_LOCK_2);
	if ((wdt_reg_read(disable_lock) & 0x3) != 0x2)
		return -1;

	wdt_reg_write(disable_lock, DISABLE_LOCK_3);
	if ((wdt_reg_read(disable_lock) & 0x3) != 0x3)
		return -1;

	wdt_reg_write(disable, disable_value);
	return 0;
}

static int wdt_set_period(unsigned long msec)
{
	unsigned long change_value, count_value;
	unsigned long prescale_value = 1;
	unsigned long refclk_khz, maxdiv;
	int ret;

	refclk_khz = clk_get_rate(TNETV107X_LPSC_WDT_ARM);
	maxdiv = (MAX_DIV / refclk_khz);

	if ((!msec) || (msec > maxdiv))
		return -1;

	count_value = refclk_khz * msec;
	if (count_value > 0xffff) {
		change_value = count_value / 0xffff + 1;
		prescale_value = count_value / change_value;
	} else {
		change_value = count_value;
	}

	ret = write_prescale_reg(prescale_value - 1);
	if (ret)
		return ret;

	ret = write_change_reg(change_value);
	if (ret)
		return ret;

	return 0;
}

unsigned long last_wdt = -1;

int wdt_start(unsigned long msecs)
{
	int ret;
	ret = wdt_control(0);
	if (ret)
		return ret;
	ret = wdt_set_period(msecs);
	if (ret)
		return ret;
	ret = wdt_control(1);
	if (ret)
		return ret;
	ret = wdt_kick();
	last_wdt = msecs;
	return ret;
}

int wdt_stop(void)
{
	last_wdt = -1;
	return wdt_control(0);
}

int wdt_kick(void)
{
	wdt_reg_write(kick_lock, KICK_LOCK_1);
	if ((wdt_reg_read(kick_lock) & 0x3) != 0x1)
		return -1;

	wdt_reg_write(kick_lock, KICK_LOCK_2);
	if ((wdt_reg_read(kick_lock) & 0x3) != 0x3)
		return -1;

	wdt_reg_write(kick, 1);
	return 0;
}

void reset_cpu(ulong addr)
{
	clk_enable(TNETV107X_LPSC_WDT_ARM);
	wdt_start(1);
	wdt_kick();
}
