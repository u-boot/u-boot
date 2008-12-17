/*
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

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>

#define WDT_BASE	WTCNT

#define WDT_WD		(1 << 6)
#define WDT_RST_P	(0)
#define WDT_RST_M	(1 << 5)
#define WDT_ENABLE	(1 << 7)

#if defined(CONFIG_WATCHDOG)
static unsigned char csr_read(void)
{
	return inb(WDT_BASE + 0x04);
}

static void cnt_write(unsigned char value)
{
	outl((unsigned short)value | 0x5A00, WDT_BASE + 0x00);
}

static void csr_write(unsigned char value)
{
	outl((unsigned short)value | 0xA500, WDT_BASE + 0x04);
}

void watchdog_reset(void)
{
	outl(0x55000000, WDT_BASE + 0x08);
}

int watchdog_init(void)
{
	/* Set overflow time*/
	cnt_write(0);
	/* Power on reset */
	csr_write(WDT_WD|WDT_RST_P|WDT_ENABLE);

	return 0;
}

int watchdog_disable(void)
{
	csr_write(csr_read() & ~WDT_ENABLE);
	return 0;
}
#endif

void reset_cpu(unsigned long ignored)
{
	while (1)
		;
}
