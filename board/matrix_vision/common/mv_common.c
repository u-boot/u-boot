/*
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <environment.h>
#include <fpga.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static char* entries_to_keep[] = {
	"serial#", "ethaddr", "eth1addr", "model_info", "sensor_cnt",
	"fpgadatasize", "ddr_size", "use_dhcp", "use_static_ipaddr",
	"static_ipaddr", "static_netmask", "static_gateway",
	"syslog", "watchdog", "netboot", "evo8serialnumber" };

#define MV_MAX_ENV_ENTRY_LENGTH	64
#define MV_KEEP_ENTRIES		ARRAY_SIZE(entries_to_keep)

void mv_reset_environment(void)
{
	int i;
	char *s[MV_KEEP_ENTRIES];
	char entries[MV_KEEP_ENTRIES][MV_MAX_ENV_ENTRY_LENGTH];

	printf("\n*** RESET ENVIRONMENT ***\n");

	memset(entries, 0, MV_KEEP_ENTRIES * MV_MAX_ENV_ENTRY_LENGTH);
	for (i = 0; i < MV_KEEP_ENTRIES; i++) {
		s[i] = getenv(entries_to_keep[i]);
		if (s[i]) {
			printf("save '%s' : %s\n", entries_to_keep[i], s[i]);
			strncpy(entries[i], s[i], MV_MAX_ENV_ENTRY_LENGTH);
		}
	}

	gd->env_valid = 0;
	env_relocate();

	for (i = 0; i < MV_KEEP_ENTRIES; i++) {
		if (s[i]) {
			printf("restore '%s' : %s\n", entries_to_keep[i], s[i]);
			setenv(entries_to_keep[i], s[i]);
		}
	}

	saveenv();
}

int mv_load_fpga(void)
{
	int result;
	size_t data_size = 0;
	void *fpga_data = NULL;
	char *datastr = getenv("fpgadata");
	char *sizestr = getenv("fpgadatasize");

	if (getenv("skip_fpga")) {
		printf("found 'skip_fpga' -> FPGA _not_ loaded !\n");
		return -1;
	}
	printf("loading FPGA\n");

	if (datastr)
		fpga_data = (void *)simple_strtoul(datastr, NULL, 16);
	if (sizestr)
		data_size = (size_t)simple_strtoul(sizestr, NULL, 16);
	if (!data_size) {
		printf("fpgadatasize invalid -> FPGA _not_ loaded !\n");
		return -1;
	}

	result = fpga_load(0, fpga_data, data_size);
	if (!result)
		show_boot_progress(0);

	return result;
}

u8 *dhcp_vendorex_prep(u8 *e)
{
	char *ptr;

	/* DHCP vendor-class-identifier = 60 */
	if ((ptr = getenv("dhcp_vendor-class-identifier"))) {
		*e++ = 60;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}
	/* DHCP_CLIENT_IDENTIFIER = 61 */
	if ((ptr = getenv("dhcp_client_id"))) {
		*e++ = 61;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}

	return e;
}

u8 *dhcp_vendorex_proc(u8 *popt)
{
	return NULL;
}
