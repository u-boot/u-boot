/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <errno.h>
#include <stdio_dev.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/armv7.h>
#include <asm/psci.h>

static int fdt_psci(void *fdt)
{
#ifdef CONFIG_ARMV7_PSCI
	int nodeoff;
	int tmp;

	nodeoff = fdt_path_offset(fdt, "/cpus");
	if (nodeoff < 0) {
		printf("couldn't find /cpus\n");
		return nodeoff;
	}

	/* add 'enable-method = "psci"' to each cpu node */
	for (tmp = fdt_first_subnode(fdt, nodeoff);
	     tmp >= 0;
	     tmp = fdt_next_subnode(fdt, tmp)) {
		const struct fdt_property *prop;
		int len;

		prop = fdt_get_property(fdt, tmp, "device_type", &len);
		if (!prop)
			continue;
		if (len < 4)
			continue;
		if (strcmp(prop->data, "cpu"))
			continue;

		fdt_setprop_string(fdt, tmp, "enable-method", "psci");
	}

	nodeoff = fdt_path_offset(fdt, "/psci");
	if (nodeoff < 0) {
		nodeoff = fdt_path_offset(fdt, "/");
		if (nodeoff < 0)
			return nodeoff;

		nodeoff = fdt_add_subnode(fdt, nodeoff, "psci");
		if (nodeoff < 0)
			return nodeoff;
	}

	tmp = fdt_setprop_string(fdt, nodeoff, "compatible", "arm,psci");
	if (tmp)
		return tmp;
	tmp = fdt_setprop_string(fdt, nodeoff, "method", "smc");
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_suspend", ARM_PSCI_FN_CPU_SUSPEND);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_off", ARM_PSCI_FN_CPU_OFF);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "cpu_on", ARM_PSCI_FN_CPU_ON);
	if (tmp)
		return tmp;
	tmp = fdt_setprop_u32(fdt, nodeoff, "migrate", ARM_PSCI_FN_MIGRATE);
	if (tmp)
		return tmp;
#endif
	return 0;
}

int armv7_apply_memory_carveout(u64 *start, u64 *size)
{
#ifdef CONFIG_ARMV7_SECURE_RESERVE_SIZE
	if (*start + *size < CONFIG_ARMV7_SECURE_BASE ||
	    *start >= (u64)CONFIG_ARMV7_SECURE_BASE +
		      CONFIG_ARMV7_SECURE_RESERVE_SIZE)
		return 0;

	/* carveout must be at the beginning or the end of the bank */
	if (*start == CONFIG_ARMV7_SECURE_BASE ||
	    *start + *size == (u64)CONFIG_ARMV7_SECURE_BASE +
			      CONFIG_ARMV7_SECURE_RESERVE_SIZE) {
		if (*size < CONFIG_ARMV7_SECURE_RESERVE_SIZE) {
			debug("Secure monitor larger than RAM bank!?\n");
			return -EINVAL;
		}
		*size -= CONFIG_ARMV7_SECURE_RESERVE_SIZE;
		if (*start == CONFIG_ARMV7_SECURE_BASE)
			*start += CONFIG_ARMV7_SECURE_RESERVE_SIZE;
		return 0;
	}
	debug("Secure monitor not located at beginning or end of RAM bank\n");
	return -EINVAL;
#else /* !CONFIG_ARMV7_SECURE_RESERVE_SIZE */
	return 0;
#endif
}

int psci_update_dt(void *fdt)
{
#ifdef CONFIG_ARMV7_NONSEC
	if (!armv7_boot_nonsec())
		return 0;
#endif
#ifndef CONFIG_ARMV7_SECURE_BASE
	/* secure code lives in RAM, keep it alive */
	fdt_add_mem_rsv(fdt, (unsigned long)__secure_start,
			__secure_end - __secure_start);
#endif

	return fdt_psci(fdt);
}
