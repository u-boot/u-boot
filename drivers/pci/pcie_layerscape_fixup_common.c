// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2020 NXP
 *
 * PCIe DT fixup for NXP Layerscape SoCs
 * Author: Wasim Khan <wasim.khan@nxp.com>
 *
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include "pcie_layerscape_fixup_common.h"

void ft_pci_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_PCIE_LAYERSCAPE_GEN4)
	uint svr;

	svr = SVR_SOC_VER(get_svr());

	if (svr == SVR_LX2160A && IS_SVR_REV(get_svr(), 1, 0))
		ft_pci_setup_ls_gen4(blob, bd);
	else
#endif /* CONFIG_PCIE_LAYERSCAPE_GEN4 */
		ft_pci_setup_ls(blob, bd);
}

#if defined(CONFIG_FSL_LAYERSCAPE)
int lx2_board_fix_fdt(void *fdt)
{
	char *reg_name, *old_str, *new_str;
	const char *reg_names;
	int names_len, old_str_len, new_str_len, remaining_str_len;
	struct str_map {
		char *old_str;
		char *new_str;
	} reg_names_map[] = {
		{ "csr_axi_slave", "regs" },
		{ "config_axi_slave", "config" }
	};
	int off = -1, i;

	off = fdt_node_offset_by_compatible(fdt, -1, "fsl,lx2160a-pcie");
	while (off != -FDT_ERR_NOTFOUND) {
		fdt_setprop(fdt, off, "compatible", "fsl,ls2088a-pcie",
			    strlen("fsl,ls2088a-pcie") + 1);

		reg_names = fdt_getprop(fdt, off, "reg-names", &names_len);
		if (!reg_names)
			continue;
		reg_name = (char *)reg_names;
		remaining_str_len = names_len - (reg_name - reg_names);
		i = 0;
		while ((i < ARRAY_SIZE(reg_names_map)) && remaining_str_len) {
			old_str = reg_names_map[i].old_str;
			new_str = reg_names_map[i].new_str;
			old_str_len = strlen(old_str);
			new_str_len = strlen(new_str);
			if (memcmp(reg_name, old_str, old_str_len) == 0) {
				/* first only leave required bytes for new_str
				 * and copy rest of the string after it
				 */
				memcpy(reg_name + new_str_len,
				       reg_name + old_str_len,
				       remaining_str_len - old_str_len);

				/* Now copy new_str */
				memcpy(reg_name, new_str, new_str_len);
				names_len -= old_str_len;
				names_len += new_str_len;
				i++;
			}

			reg_name = memchr(reg_name, '\0', remaining_str_len);
			if (!reg_name)
				break;
			reg_name += 1;

			remaining_str_len = names_len - (reg_name - reg_names);
		}
		fdt_setprop(fdt, off, "reg-names", reg_names, names_len);
		fdt_delprop(fdt, off, "apio-wins");
		fdt_delprop(fdt, off, "ppio-wins");
		off = fdt_node_offset_by_compatible(fdt, off,
						    "fsl,lx2160a-pcie");
	}
	return 0;
}

int pcie_board_fix_fdt(void *fdt)
{
	uint svr;

	svr = SVR_SOC_VER(get_svr());

	if (svr == SVR_LX2160A && IS_SVR_REV(get_svr(), 2, 0))
		return lx2_board_fix_fdt(fdt);

	return 0;
}

#ifdef CONFIG_ARCH_LX2160A
/* returns the next available streamid for pcie, -errno if failed */
int pcie_next_streamid(int currentid, int idx)
{
	if (currentid > FSL_PEX_STREAM_ID_END)
		return -EINVAL;

	return currentid | ((idx + 1) << 11);
}
#else
/* returns the next available streamid for pcie, -errno if failed */
int pcie_next_streamid(int currentid, int idx)
{
	static int next_stream_id = FSL_PEX_STREAM_ID_START;

	if (next_stream_id > FSL_PEX_STREAM_ID_END)
		return -EINVAL;

	return next_stream_id++;
}
#endif
#endif /* CONFIG_FSL_LAYERSCAPE */
