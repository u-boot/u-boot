/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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

#include <common.h>
#include <mpc824x.h>
#include <pci.h>
#include <i2c.h>

int checkboard (void)
{
	puts (	"Board: OXC8240\n" );
	return 0;
}

long int initdram (int board_type)
{
#ifndef CFG_RAMBOOT
	long size;
	long new_bank0_end;
	long mear1;
	long emear1;

	size = get_ram_size(CFG_SDRAM_BASE, CFG_MAX_RAM_SIZE);

	new_bank0_end = size - 1;
	mear1 = mpc824x_mpc107_getreg(MEAR1);
	emear1 = mpc824x_mpc107_getreg(EMEAR1);
	mear1 = (mear1  & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT);
	emear1 = (emear1 & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_EADDR_SHIFT);
	mpc824x_mpc107_setreg(MEAR1, mear1);
	mpc824x_mpc107_setreg(EMEAR1, emear1);

	return (size);
#else
	/* if U-Boot starts from RAM, then suppose we have 16Mb of RAM */
	return (16 << 20);
#endif
}

/*
 * Initialize PCI Devices, report devices found.
 */
#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_oxc_config_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x14, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET0_IOADDR,
				       PCI_ENET0_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0x15, PCI_ANY_ID,
	  pci_cfgfunc_config_device, { PCI_ENET1_IOADDR,
				       PCI_ENET1_MEMADDR,
				       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER }},
	{ }
};
#endif

static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table: pci_oxc_config_table,
#endif
};

void pci_init_board (void)
{
	pci_mpc824x_init(&hose);
}

int board_early_init_f (void)
{
	*(volatile unsigned char *)(CFG_CPLD_RESET) = 0x89;
	return 0;
}

#ifdef CONFIG_WATCHDOG
void oxc_wdt_reset(void)
{
	*(volatile unsigned char *)(CFG_CPLD_WATCHDOG) = 0xff;
}

void watchdog_reset(void)
{
	int re_enable = disable_interrupts();

	oxc_wdt_reset();
	if (re_enable)
		enable_interrupts();
}
#endif

static int oxc_get_expander(unsigned char addr, unsigned char * val)
{
	return i2c_read(addr, 0, 0, val, 1);
}

static int oxc_set_expander(unsigned char addr, unsigned char val)
{
	return i2c_write(addr, 0, 0, &val, 1);
}

static int expander0alive = 0;

#ifdef CONFIG_SHOW_ACTIVITY
static int ledtoggle = 0;
static int ledstatus = 1;

void oxc_toggle_activeled(void)
{
	ledtoggle++;
}

void board_show_activity (ulong timestamp)
{
	if ((timestamp % (CFG_HZ / 10)) == 0)
		oxc_toggle_activeled ();
}

void show_activity(int arg)
{
	static unsigned char led = 0;
	unsigned char val;

	if (!expander0alive) return;

	if ((ledtoggle > (2 * arg)) && ledstatus) {
		led ^= 0x80;
		oxc_get_expander(CFG_I2C_EXPANDER0_ADDR, &val);
		udelay(200);
		oxc_set_expander(CFG_I2C_EXPANDER0_ADDR, (val & 0x7F) | led);
		ledtoggle = 0;
	}
}
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int arg)
{
	unsigned char val;

	if (!expander0alive) return;

	if (arg > 0 && ledstatus) {
		ledstatus = 0;
		oxc_get_expander(CFG_I2C_EXPANDER0_ADDR, &val);
		udelay(200);
		oxc_set_expander(CFG_I2C_EXPANDER0_ADDR, val | 0x80);
	} else if (arg < 0) {
		oxc_get_expander(CFG_I2C_EXPANDER0_ADDR, &val);
		udelay(200);
		oxc_set_expander(CFG_I2C_EXPANDER0_ADDR, val & 0x7F);
		ledstatus = 1;
	}
}
#endif

int misc_init_r (void)
{
	/* check whether the i2c expander #0 is accessible */
	if (!oxc_set_expander(CFG_I2C_EXPANDER0_ADDR, 0x7F)) {
		udelay(200);
		expander0alive = 1;
	}

#ifdef CFG_OXC_GENERATE_IP
	{
		DECLARE_GLOBAL_DATA_PTR;

		char str[32];
		unsigned long ip = CFG_OXC_IPMASK;
		bd_t *bd = gd->bd;

		if (expander0alive) {
			unsigned char val;

			if (!oxc_get_expander(CFG_I2C_EXPANDER0_ADDR, &val)) {
				ip = (ip & 0xffffff00) | ((val & 0x7c) >> 2);
			}
		}

		if ((ip & 0xff) < 3) {
			/* if fail, set x.x.x.254 */
			ip = (ip & 0xffffff00) | 0xfe;
		}

		bd->bi_ip_addr = ip;
		sprintf(str, "%ld.%ld.%ld.%ld",
			(bd->bi_ip_addr & 0xff000000) >> 24,
			(bd->bi_ip_addr & 0x00ff0000) >> 16,
			(bd->bi_ip_addr & 0x0000ff00) >> 8,
			(bd->bi_ip_addr & 0x000000ff));
		setenv("ipaddr", str);
		printf("ip:    %s\n", str);
	}
#endif
	return (0);
}
