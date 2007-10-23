/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/cache.h>
#include <ppc4xx.h>

#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>

DECLARE_GLOBAL_DATA_PTR;

static void do_fixup(void *fdt, const char *node, const char *prop,
		     const void *val, int len, int create)
{
#if defined(DEBUG)
	int i;
	debug("Updating property '%s/%s' = ", node, prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("(%d)\n", *(u32 *)val);
#endif
	int rc = fdt_find_and_setprop(fdt, node, prop, val, len, create);
	if (rc)
		printf("Unable to update property %s:%s, err=%s\n",
		       node, prop, fdt_strerror(rc));
}

static void do_fixup_macaddr(void *fdt, int offset, const void *val, int i)
{
	int rc;

	debug("Updating node EMAC%d\n", i);

	rc = fdt_setprop(fdt, offset, "mac-address", val, 6);
	if (rc)
		printf("Unable to update property %s, err=%s\n",
		       "mac-address", fdt_strerror(rc));
	rc = fdt_setprop(fdt, offset, "local-mac-address", val, 6);
	if (rc)
		printf("Unable to update property %s, err=%s\n",
		       "local-mac-address", fdt_strerror(rc));
}

static void do_fixup_u32(void *fdt, const char *node, const char *prop,
			 u32 val, int create)
{
	val = cpu_to_fdt32(val);
	do_fixup(fdt, node, prop, &val, sizeof(val), create);
}

static void do_fixup_uart(void *fdt, int offset, int i, bd_t *bd)
{
	int rc;
	u32 val;
	PPC4xx_SYS_INFO sys_info;

	get_sys_info(&sys_info);

	debug("Updating node UART%d: clock-frequency=%d\n", i, gd->uart_clk);

	val = cpu_to_fdt32(gd->uart_clk);
	rc = fdt_setprop(fdt, offset, "clock-frequency", &val, 4);
	if (rc)
		printf("Unable to update node UART, err=%s\n", fdt_strerror(rc));

	val = cpu_to_fdt32(bd->bi_baudrate);
	rc = fdt_setprop(fdt, offset, "current-speed", &val, 4);
	if (rc)
		printf("Unable to update node UART, err=%s\n", fdt_strerror(rc));
}

void ft_cpu_setup(void *blob, bd_t *bd)
{
	char * cpu_path = "/cpus/" OF_CPU;
	sys_info_t sys_info;
	int offset;
	int i;
	int tmp[2];

	get_sys_info (&sys_info);

	do_fixup_u32(blob, cpu_path, "timebase-frequency", bd->bi_intfreq, 1);
	do_fixup_u32(blob, cpu_path, "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_u32(blob, "/plb", "clock-frequency", sys_info.freqPLB, 1);
	do_fixup_u32(blob, "/plb/opb", "clock-frequency", sys_info.freqOPB, 1);
	do_fixup_u32(blob, "/plb/opb/ebc", "clock-frequency", sys_info.freqEBC, 1);

	/* update, or add and update /memory node */
	offset = fdt_find_node_by_path(blob, "/memory");
	if (offset < 0) {
		offset = fdt_add_subnode(blob, 0, "memory");
		if (offset < 0)
			debug("failed to add /memory node: %s\n",
			      fdt_strerror(offset));
	}
	if (offset >= 0) {
		fdt_setprop(blob, offset, "device_type",
			    "memory", sizeof("memory"));
		tmp[0] = cpu_to_fdt32(bd->bi_memstart);
		tmp[1] = cpu_to_fdt32(bd->bi_memsize);
		fdt_setprop(blob, offset, "reg", tmp, sizeof(tmp));
		debug("Updating /memory node to %d:%d\n",
		      bd->bi_memstart, bd->bi_memsize);
	}

	/*
	 * Setup all baudrates for the UARTs
	 */
	offset = 0;
	for (i = 0; i < 4; i++) {
		offset = fdt_find_node_by_type(blob, offset, "serial");
		if (offset < 0)
			break;

		do_fixup_uart(blob, offset, i, bd);
	}

	/*
	 * Setup all MAC addresses in fdt
	 */
	offset = 0;
	for (i = 0; i < 4; i++) {
		offset = fdt_find_node_by_type(blob, offset, "network");
		if (offset < 0)
			break;

		switch (i) {
		case 0:
			do_fixup_macaddr(blob, offset, bd->bi_enetaddr, 0);
			break;
#ifdef CONFIG_HAS_ETH1
		case 1:
			do_fixup_macaddr(blob, offset, bd->bi_enet1addr, 1);
			break;
#endif
#ifdef CONFIG_HAS_ETH2
		case 2:
			do_fixup_macaddr(blob, offset, bd->bi_enet2addr, 2);
			break;
#endif
#ifdef CONFIG_HAS_ETH3
		case 3:
			do_fixup_macaddr(blob, offset, bd->bi_enet3addr, 3);
			break;
#endif
		}
	}
}
#endif /* CONFIG_OF_LIBFDT */
