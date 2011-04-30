/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _FSL_PORTALS_H_
#define _FSL_PORTALS_H_

/* entries must be in order and contiguous */
enum fsl_dpaa_dev {
	FSL_HW_PORTAL_SEC,
#ifdef CONFIG_SYS_DPAA_FMAN
	FSL_HW_PORTAL_FMAN1,
#if (CONFIG_SYS_NUM_FMAN == 2)
	FSL_HW_PORTAL_FMAN2,
#endif
#endif
#ifdef CONFIG_SYS_DPAA_PME
	FSL_HW_PORTAL_PME,
#endif
};

struct qportal_info {
	u16	dliodn;	/* DQRR LIODN */
	u16	fliodn;	/* frame data LIODN */
	u16	liodn_offset;
	u8	sdest;
};

#define SET_QP_INFO(dqrr, fdata, off, dest) \
	{ .dliodn = dqrr, .fliodn = fdata, .liodn_offset = off, .sdest = dest }

extern int get_dpaa_liodn(enum fsl_dpaa_dev dpaa_dev,
			  u32 *liodns, int liodn_offset);
extern void setup_portals(void);
extern void fdt_fixup_qportals(void *blob);
extern void fdt_fixup_bportals(void *blob);

extern struct qportal_info qp_info[];
extern void fdt_portal(void *blob, const char *compat, const char *container,
			u64 addr, u32 size);

#endif
