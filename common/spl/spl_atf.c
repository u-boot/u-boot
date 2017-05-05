/*
 * Reference to the ARM TF Project,
 * plat/arm/common/arm_bl2_setup.c
 * Portions copyright (c) 2013-2016, ARM Limited and Contributors. All rights
 * reserved.
 * Copyright (C) 2016 Rockchip Electronic Co.,Ltd
 * Written by Kever Yang <kever.yang@rock-chips.com>
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

#include <common.h>
#include <atf_common.h>
#include <errno.h>
#include <spl.h>

static struct bl2_to_bl31_params_mem bl31_params_mem;
static struct bl31_params *bl2_to_bl31_params;

/**
 * bl2_plat_get_bl31_params() - prepare params for bl31.
 *
 * This function assigns a pointer to the memory that the platform has kept
 * aside to pass platform specific and trusted firmware related information
 * to BL31. This memory is allocated by allocating memory to
 * bl2_to_bl31_params_mem structure which is a superset of all the
 * structure whose information is passed to BL31
 * NOTE: This function should be called only once and should be done
 * before generating params to BL31
 *
 * @return bl31 params structure pointer
 */
struct bl31_params *bl2_plat_get_bl31_params(void)
{
	struct entry_point_info *bl33_ep_info;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	memset(&bl31_params_mem, 0, sizeof(struct bl2_to_bl31_params_mem));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, ATF_PARAM_BL31, ATF_VERSION_1, 0);

	/* Fill BL31 related information */
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	/* Fill BL32 related information if it exists */
#ifdef BL32_BASE
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info, ATF_PARAM_EP,
		       ATF_VERSION_1, 0);
	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);
#endif /* BL32_BASE */

	/* Fill BL33 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl33_ep_info, ATF_PARAM_EP, ATF_VERSION_1,
		       ATF_EP_NON_SECURE);

	/* BL33 expects to receive the primary CPU MPID (through x0) */
	bl33_ep_info->args.arg0 = 0xffff & read_mpidr();
	bl33_ep_info->pc = CONFIG_SYS_TEXT_BASE;
	bl33_ep_info->spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
				     DISABLE_ALL_EXECPTIONS);

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info,
		       ATF_PARAM_IMAGE_BINARY, ATF_VERSION_1, 0);

	return bl2_to_bl31_params;
}

void raw_write_daif(unsigned int daif)
{
	__asm__ __volatile__("msr DAIF, %0\n\t" : : "r" (daif) : "memory");
}

void bl31_entry(void)
{
	struct bl31_params *bl31_params;
	void (*entry)(struct bl31_params *params, void *plat_params) = NULL;

	bl31_params = bl2_plat_get_bl31_params();
	entry = (void *)CONFIG_SPL_ATF_TEXT_BASE;

	raw_write_daif(SPSR_EXCEPTION_MASK);
	dcache_disable();

	entry(bl31_params, NULL);
}
