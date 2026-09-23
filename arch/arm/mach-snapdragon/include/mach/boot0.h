/* SPDX-License-Identifier: GPL-2.0+ */
#if defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_BOOT0_SDM845_WORKAROUND)
#include "sdm845_spl_boot0.h"
#else
	b	reset
#endif
#else
#if defined(CONFIG_BOOT0_MSM8916_PSCI_WORKAROUND)
#include "msm8916_boot0.h"
#else
	b	reset
#endif
#endif
