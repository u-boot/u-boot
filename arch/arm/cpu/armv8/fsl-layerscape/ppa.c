/*
 * Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <errno.h>
#include <asm/system.h>
#include <asm/types.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_FSL_LSCH3
#include <asm/arch/immap_lsch3.h>
#elif defined(CONFIG_FSL_LSCH2)
#include <asm/arch/immap_lsch2.h>
#endif
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#include <asm/armv8/sec_firmware.h>
#endif
#ifdef CONFIG_CHAIN_OF_TRUST
#include <fsl_validate.h>
#endif

int ppa_init(void)
{
	const void *ppa_fit_addr;
	u32 *boot_loc_ptr_l, *boot_loc_ptr_h;
	int ret;

#ifdef CONFIG_CHAIN_OF_TRUST
	uintptr_t ppa_esbc_hdr = CONFIG_SYS_LS_PPA_ESBC_ADDR;
	uintptr_t ppa_img_addr = 0;
#endif

#ifdef CONFIG_SYS_LS_PPA_FW_IN_XIP
	ppa_fit_addr = (void *)CONFIG_SYS_LS_PPA_FW_ADDR;
#else
#error "No CONFIG_SYS_LS_PPA_FW_IN_xxx defined"
#endif

#ifdef CONFIG_CHAIN_OF_TRUST
	ppa_img_addr = (uintptr_t)ppa_fit_addr;
	if (fsl_check_boot_mode_secure() != 0) {
		ret = fsl_secboot_validate(ppa_esbc_hdr,
					   CONFIG_PPA_KEY_HASH,
					   &ppa_img_addr);
		if (ret != 0)
			printf("PPA validation failed\n");
		else
			printf("PPA validation Successful\n");
	}
#endif

#ifdef CONFIG_FSL_LSCH3
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	boot_loc_ptr_l = &gur->bootlocptrl;
	boot_loc_ptr_h = &gur->bootlocptrh;
#elif defined(CONFIG_FSL_LSCH2)
	struct ccsr_scfg __iomem *scfg = (void *)(CONFIG_SYS_FSL_SCFG_ADDR);
	boot_loc_ptr_l = &scfg->scratchrw[1];
	boot_loc_ptr_h = &scfg->scratchrw[0];
#endif

	debug("fsl-ppa: boot_loc_ptr_l = 0x%p, boot_loc_ptr_h =0x%p\n",
	      boot_loc_ptr_l, boot_loc_ptr_h);
	ret = sec_firmware_init(ppa_fit_addr, boot_loc_ptr_l, boot_loc_ptr_h);

	return ret;
}
