/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *
 * This file is derived from arch/powerpc/cpu/mpc85xx/cpu.c and
 * arch/powerpc/cpu/mpc86xx/cpu.c. Basically this file contains
 * cpu specific common code for 85xx/86xx processors.
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
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/mp.h>
#include <asm/fsl_serdes.h>
#include <phy.h>
#include <hwconfig.h>

#define FSL_MAX_NUM_USB_CTRLS	2

#if defined(CONFIG_MP) && (defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx))
static int ft_del_cpuhandle(void *blob, int cpuhandle)
{
	int off, ret = -FDT_ERR_NOTFOUND;

	/* if we find a match, we'll delete at it which point the offsets are
	 * invalid so we start over from the beginning
	 */
	off = fdt_node_offset_by_prop_value(blob, -1, "cpu-handle",
						&cpuhandle, 4);
	while (off != -FDT_ERR_NOTFOUND) {
		fdt_delprop(blob, off, "cpu-handle");
		ret = 1;
		off = fdt_node_offset_by_prop_value(blob, -1, "cpu-handle",
				&cpuhandle, 4);
	}

	return ret;
}

void ft_fixup_num_cores(void *blob) {
	int off, num_cores, del_cores;

	del_cores = 0;
	num_cores = cpu_numcores();

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		u32 *reg = (u32 *)fdt_getprop(blob, off, "reg", 0);

		if ((*reg > num_cores-1) || (is_core_disabled(*reg))) {
			int ph = fdt_get_phandle(blob, off);

			/* Delete the cpu node once there are no cpu handles */
			if (-FDT_ERR_NOTFOUND == ft_del_cpuhandle(blob, ph)) {
				fdt_del_node(blob, off);
				del_cores++;
			}
			/* either we deleted some cpu handles or the cpu node
			 * so we reset the offset back to the start since we
			 * can't trust the offsets anymore
			 */
			off = -1;
		}
		off = fdt_node_offset_by_prop_value(blob, off,
				"device_type", "cpu", 4);
	}
	debug ("%x core system found\n", num_cores);
	debug ("deleted %d extra core entry entries from device tree\n",
								del_cores);
}
#endif /* defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx) */

#ifdef CONFIG_HAS_FSL_DR_USB
static void fdt_fixup_usb_mode_phy_type(void *blob, const char *mode,
				const char *phy_type)
{
	const char *compat = "fsl-usb2-dr";
	const char *prop_mode = "dr_mode";
	const char *prop_type = "phy_type";
	static int start_offset = -1;
	int node_offset;
	int err;

	node_offset = fdt_node_offset_by_compatible(blob,
			start_offset, compat);
	if (node_offset < 0) {
		printf("WARNING: could not find compatible node %s: %s.\n",
			compat, fdt_strerror(node_offset));
		return;
	}

	if (mode) {
		err = fdt_setprop(blob, node_offset, prop_mode, mode,
				  strlen(mode) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_mode, compat, fdt_strerror(err));
	}

	if (phy_type) {
		err = fdt_setprop(blob, node_offset, prop_type, phy_type,
				  strlen(phy_type) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_type, compat, fdt_strerror(err));
	}

	start_offset = node_offset;
}

void fdt_fixup_dr_usb(void *blob, bd_t *bd)
{
	const char *modes[] = { "host", "peripheral", "otg" };
	const char *phys[] = { "ulpi", "umti" };
	const char *mode = NULL;
	const char *phy_type = NULL;
	char usb1_defined = 0;
	char str[5];
	int i, j;

	for (i = 1; i <= FSL_MAX_NUM_USB_CTRLS; i++) {
		int mode_idx = -1, phy_idx = -1;
		sprintf(str, "%s%d", "usb", i);
		if (hwconfig(str)) {
			for (j = 0; j < sizeof(modes); j++) {
				if (hwconfig_subarg_cmp(str, "dr_mode",
						modes[j])) {
					mode_idx = j;
					break;
				}
			}
			for (j = 0; j < sizeof(phys); j++) {
				if (hwconfig_subarg_cmp(str, "phy_type",
						phys[j])) {
					phy_idx = j;
					break;
				}
			}
			if (mode_idx >= 0)
				fdt_fixup_usb_mode_phy_type(blob,
					modes[mode_idx], NULL);
			if (phy_idx >= 0)
				fdt_fixup_usb_mode_phy_type(blob,
					NULL, phys[phy_idx]);
			if (!strcmp(str, "usb1"))
				usb1_defined = 1;
			if (mode_idx < 0 && phy_idx < 0)
				printf("WARNING: invalid phy or mode\n");
		}
	}
	if (!usb1_defined) {
		mode = getenv("usb_dr_mode");
		phy_type = getenv("usb_phy_type");
		if (!mode && !phy_type)
			return;
		fdt_fixup_usb_mode_phy_type(blob, mode, phy_type);
	}
}
#endif /* CONFIG_HAS_FSL_DR_USB */

/*
 * update crypto node properties to a specified revision of the SEC
 * called with sec_rev == 0 if not on an E processor
 */
#if CONFIG_SYS_FSL_SEC_COMPAT == 2 /* SEC 2.x/3.x */
void fdt_fixup_crypto_node(void *blob, int sec_rev)
{
	const struct sec_rev_prop {
		u32 sec_rev;
		u32 num_channels;
		u32 channel_fifo_len;
		u32 exec_units_mask;
		u32 descriptor_types_mask;
	} sec_rev_prop_list [] = {
		{ 0x0200, 4, 24, 0x07e, 0x01010ebf }, /* SEC 2.0 */
		{ 0x0201, 4, 24, 0x0fe, 0x012b0ebf }, /* SEC 2.1 */
		{ 0x0202, 1, 24, 0x04c, 0x0122003f }, /* SEC 2.2 */
		{ 0x0204, 4, 24, 0x07e, 0x012b0ebf }, /* SEC 2.4 */
		{ 0x0300, 4, 24, 0x9fe, 0x03ab0ebf }, /* SEC 3.0 */
		{ 0x0301, 4, 24, 0xbfe, 0x03ab0ebf }, /* SEC 3.1 */
		{ 0x0303, 4, 24, 0x97c, 0x03a30abf }, /* SEC 3.3 */
	};
	char compat_strlist[ARRAY_SIZE(sec_rev_prop_list) *
			    sizeof("fsl,secX.Y")];
	int crypto_node, sec_idx, err;
	char *p;
	u32 val;

	/* locate crypto node based on lowest common compatible */
	crypto_node = fdt_node_offset_by_compatible(blob, -1, "fsl,sec2.0");
	if (crypto_node == -FDT_ERR_NOTFOUND)
		return;

	/* delete it if not on an E-processor */
	if (crypto_node > 0 && !sec_rev) {
		fdt_del_node(blob, crypto_node);
		return;
	}

	/* else we got called for possible uprev */
	for (sec_idx = 0; sec_idx < ARRAY_SIZE(sec_rev_prop_list); sec_idx++)
		if (sec_rev_prop_list[sec_idx].sec_rev == sec_rev)
			break;

	if (sec_idx == ARRAY_SIZE(sec_rev_prop_list)) {
		puts("warning: unknown SEC revision number\n");
		return;
	}

	val = cpu_to_fdt32(sec_rev_prop_list[sec_idx].num_channels);
	err = fdt_setprop(blob, crypto_node, "fsl,num-channels", &val, 4);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	val = cpu_to_fdt32(sec_rev_prop_list[sec_idx].descriptor_types_mask);
	err = fdt_setprop(blob, crypto_node, "fsl,descriptor-types-mask", &val, 4);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	val = cpu_to_fdt32(sec_rev_prop_list[sec_idx].exec_units_mask);
	err = fdt_setprop(blob, crypto_node, "fsl,exec-units-mask", &val, 4);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	val = cpu_to_fdt32(sec_rev_prop_list[sec_idx].channel_fifo_len);
	err = fdt_setprop(blob, crypto_node, "fsl,channel-fifo-len", &val, 4);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));

	val = 0;
	while (sec_idx >= 0) {
		p = compat_strlist + val;
		val += sprintf(p, "fsl,sec%d.%d",
			(sec_rev_prop_list[sec_idx].sec_rev & 0xff00) >> 8,
			sec_rev_prop_list[sec_idx].sec_rev & 0x00ff) + 1;
		sec_idx--;
	}
	err = fdt_setprop(blob, crypto_node, "compatible", &compat_strlist, val);
	if (err < 0)
		printf("WARNING: could not set crypto property: %s\n",
		       fdt_strerror(err));
}
#elif CONFIG_SYS_FSL_SEC_COMPAT >= 4  /* SEC4 */
void fdt_fixup_crypto_node(void *blob, int sec_rev)
{
	if (!sec_rev)
		fdt_del_node_and_alias(blob, "crypto");
}
#endif

int fdt_fixup_phy_connection(void *blob, int offset, phy_interface_t phyc)
{
	return fdt_setprop_string(blob, offset, "phy-connection-type",
					 phy_string_for_interface(phyc));
}

#ifdef CONFIG_SYS_SRIO
void ft_srio_setup(void *blob)
{
#ifdef CONFIG_SRIO1
	if (!is_serdes_configured(SRIO1)) {
		fdt_del_node_and_alias(blob, "rio0");
	}
#else
	fdt_del_node_and_alias(blob, "rio0");
#endif
#ifdef CONFIG_SRIO2
	if (!is_serdes_configured(SRIO2)) {
		fdt_del_node_and_alias(blob, "rio1");
	}
#else
	fdt_del_node_and_alias(blob, "rio1");
#endif
}
#endif
