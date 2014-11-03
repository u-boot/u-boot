/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CL_COMMON_
#define _CL_COMMON_

#include <asm/errno.h>

void cl_print_pcb_info(void);

#ifdef CONFIG_CMD_USB
int cl_usb_hub_init(int gpio, const char *label);
void cl_usb_hub_deinit(int gpio);
#else /* !CONFIG_CMD_USB */
static inline int cl_usb_hub_init(int gpio, const char *label)
{
	return -ENOSYS;
}
static inline void cl_usb_hub_deinit(int gpio) {}
#endif /* CONFIG_CMD_USB */

#ifdef CONFIG_SPLASH_SCREEN
int cl_splash_screen_prepare(int nand_offset);
#else /* !CONFIG_SPLASH_SCREEN */
static inline int cl_splash_screen_prepare(int nand_offset)
{
	return -ENOSYS;
}
#endif /* CONFIG_SPLASH_SCREEN */

#endif /* _CL_COMMON_ */
