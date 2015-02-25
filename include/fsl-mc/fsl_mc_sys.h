/*
 * Freescale Layerscape Management Complex (MC) Environment-specific code
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _FSL_MC_SYS_H
#define _FSL_MC_SYS_H

#include <asm/io.h>

struct mc_command;

/*
 * struct mc_portal_wrapper - MC command portal wrapper object
 */
struct fsl_mc_io {
	struct mc_command __iomem *mmio_regs;
};

int mc_send_command(struct fsl_mc_io *mc_io,
		    struct mc_command *cmd);

#endif /* _FSL_MC_SYS_H */
