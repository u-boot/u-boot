// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2020 NXP
 *
 * PCIe DT fixup for NXP Layerscape SoCs
 * Author: Wasim Khan <wasim.khan@nxp.com>
 *
 */

#include <common.h>
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
