/* SPDX-License-Identifier: GPL-2.0+ */
#if defined(CONFIG_SPL_BUILD)
	b	reset
#else
#if defined(CONFIG_BOOT0_MSM8916_PSCI_WORKAROUND)
#include "msm8916_boot0.h"
#else
	b	reset
#endif
#endif
