/*
 *(C) Copyright 2005-2008 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "nm.h"

DECLARE_GLOBAL_DATA_PTR;

#define DEFAULT_ETH_ADDR  "ethaddr"

typedef struct {u8	id;	char *name;} generation_info;

generation_info generations[6] = {
	{HW_GENERATION_HCU3,	"HCU3"},
	{HW_GENERATION_HCU4,	"HCU4"},
	{HW_GENERATION_HCU5,	"HCU5"},
	{HW_GENERATION_MCU,	"MCU"},
	{HW_GENERATION_MCU20,	"MCU20"},
	{HW_GENERATION_MCU25,	"MCU25"},
};

void nm_show_print(int generation, int index, int hw_capabilities)
{
	int j;
	char *generationName=0;

	/* reset ANSI terminal color mode */
	printf("\x1B""[0m""Netstal Maschinen AG: ");
	for (j=0; j < (sizeof(generations)/sizeof(generations[0])); j++) {
		if (generations[j].id == generation) {
			generationName = generations[j].name;
			break;
		}
	}
	printf("%s: index %d HW 0x%x\n", generationName, index, hw_capabilities);
	for (j = 0;j < 6; j++) {
		hcu_led_set(1 << j);
		udelay(200 * 1000);
	}
}

void set_params_for_sw_install(int install_requested, char *board_name )
{
	if (install_requested) {
		char string[128];

		printf("\n\n%s SW-Installation: %d patching boot parameters\n",
		       board_name, install_requested);
		setenv("bootdelay", "0");
		setenv("loadaddr", "0x01000000");
		setenv("serverip", "172.25.1.1");
		setenv("bootcmd", "run install");
		sprintf(string, "tftp ${loadaddr} admin/sw_on_hd; "
			"tftp ${loadaddr} installer/%s_sw_inst; "
			"run boot_sw_inst", board_name);
		setenv("install", string);
		sprintf(string, "setenv bootargs emac(0,0)c:%s/%s_sw_inst "
			"e=${ipaddr} h=${serverip} f=0x1000; "
			"bootvx ${loadaddr}%c",
			board_name, board_name, 0);
		setenv("boot_sw_inst", string);
	}
}

void common_misc_init_r(void)
{
	IPaddr_t ipaddr;
	char *ipstring;
	uchar ethaddr[6];

	if (!eth_getenv_enetaddr(DEFAULT_ETH_ADDR, ethaddr)) {
		/* Must be in sync with CONFIG_ETHADDR */
		u32 serial = get_serial_number();
		ethaddr[0] = 0x00;
		ethaddr[1] = 0x60;
		ethaddr[2] = 0x13;
		ethaddr[3] = (serial >> 16) & 0xff;
		ethaddr[4] = (serial >>  8) & 0xff;
		ethaddr[5] = hcu_get_slot();
		eth_setenv_enetaddr(DEFAULT_ETH_ADDR, ethaddr);
	}

	/* IP-Adress update */
	ipstring = getenv("ipaddr");
	if (ipstring == 0)
		ipaddr = string_to_ip("172.25.1.99");
	else
		ipaddr = string_to_ip(ipstring);
	if ((ipaddr & 0xff) != (32 + hcu_get_slot())) {
		char tmp[22];

		ipaddr = (ipaddr & 0xffffff00) + 32 + hcu_get_slot();
		ip_to_string (ipaddr, tmp);
		printf("%s: enforce %s\n",  __FUNCTION__, tmp);
		setenv("ipaddr", tmp);
		saveenv();
	}
}
