/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _FSL_MC_PRIVATE_H_
#define _FSL_MC_PRIVATE_H_

#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/stringify.h>

#include <fsl-mc/fsl_mc_sys.h>
#include <fsl-mc/fsl_mc_cmd.h>
#include <fsl-mc/fsl_dprc.h>
#include <fsl-mc/fsl_dpbp.h>

extern struct fsl_mc_io *dflt_mc_io;

/**
 * struct dpbp_node - DPBP strucuture
 * @uint16_t handle: DPBP object handle
 * @int dpbp_id: DPBP id
 */
struct fsl_dpbp_obj {
	uint16_t dpbp_handle;
	struct dpbp_attr dpbp_attr;
};

extern struct fsl_dpbp_obj *dflt_dpbp;

/**
 * struct fsl_dpio_obj - DPIO strucuture
 * @int dpio_id: DPIO id
 * @struct qbman_swp *sw_portal: SW portal object
 */
struct fsl_dpio_obj {
	int dpio_id;
	struct qbman_swp *sw_portal; /** SW portal object */
};

extern struct fsl_dpio_obj *dflt_dpio;

int mc_init(void);
int ldpaa_eth_init(struct dprc_obj_desc obj_desc);
#endif /* _FSL_MC_PRIVATE_H_ */
