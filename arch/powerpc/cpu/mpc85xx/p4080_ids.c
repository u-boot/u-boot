/*
 * Copyright 2010 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

struct qportal_info qp_info[CONFIG_SYS_QMAN_NUM_PORTALS] = {
	/* dqrr liodn, frame data liodn, liodn off, sdest */
	SET_QP_INFO( 1,  2,  1, 0),
	SET_QP_INFO( 3,  4,  2, 1),
	SET_QP_INFO( 5,  6,  3, 2),
	SET_QP_INFO( 7,  8,  4, 3),
	SET_QP_INFO( 9, 10,  5, 4),
	SET_QP_INFO(11, 12,  6, 5),
	SET_QP_INFO(13, 14,  7, 6),
	SET_QP_INFO(15, 16,  8, 7),
	SET_QP_INFO(17, 18,  9, 0), /* for now sdest to 0 */
	SET_QP_INFO(19, 20, 10, 0), /* for now sdest to 0 */
};

struct liodn_id_table liodn_tbl[] = {
	SET_USB_LIODN(1, "fsl-usb2-mph", 127),
	SET_USB_LIODN(2, "fsl-usb2-dr", 157),

	SET_SDHC_LIODN(1, 156),

	SET_PCI_LIODN(1, 193),
	SET_PCI_LIODN(2, 194),
	SET_PCI_LIODN(3, 195),

	SET_DMA_LIODN(1, 196),
	SET_DMA_LIODN(2, 197),

	SET_GUTS_LIODN("fsl,rapidio-delta", 198, rio1liodnr, 0),
	SET_GUTS_LIODN(NULL, 199, rio2liodnr, 0),
	SET_GUTS_LIODN(NULL, 200, rmuliodnr, 0),

	SET_QMAN_LIODN(31),
	SET_BMAN_LIODN(32),
	SET_PME_LIODN(128),
};

#ifdef CONFIG_SYS_DPAA_FMAN
struct liodn_id_table fman1_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(1, 0, 11),
	SET_FMAN_RX_1G_LIODN(1, 1, 12),
	SET_FMAN_RX_1G_LIODN(1, 2, 13),
	SET_FMAN_RX_1G_LIODN(1, 3, 14),
	SET_FMAN_RX_10G_LIODN(1, 0, 15),
};

#if (CONFIG_SYS_NUM_FMAN == 2)
struct liodn_id_table fman2_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(2, 0, 16),
	SET_FMAN_RX_1G_LIODN(2, 1, 17),
	SET_FMAN_RX_1G_LIODN(2, 2, 18),
	SET_FMAN_RX_1G_LIODN(2, 3, 19),
	SET_FMAN_RX_10G_LIODN(2, 0, 20),
};
#endif
#endif

struct liodn_id_table sec_liodn_tbl[] = {
	SET_SEC_JQ_LIODN_ENTRY(0, 146, 154),
	SET_SEC_JQ_LIODN_ENTRY(1, 147, 155),
	SET_SEC_JQ_LIODN_ENTRY(2, 178, 186),
	SET_SEC_JQ_LIODN_ENTRY(3, 179, 187),
	SET_SEC_RTIC_LIODN_ENTRY(a, 144),
	SET_SEC_RTIC_LIODN_ENTRY(b, 145),
	SET_SEC_RTIC_LIODN_ENTRY(c, 176),
	SET_SEC_RTIC_LIODN_ENTRY(d, 177),
	SET_SEC_DECO_LIODN_ENTRY(0, 129, 161),
	SET_SEC_DECO_LIODN_ENTRY(1, 130, 162),
	SET_SEC_DECO_LIODN_ENTRY(2, 131, 163),
	SET_SEC_DECO_LIODN_ENTRY(3, 132, 164),
	SET_SEC_DECO_LIODN_ENTRY(4, 133, 165),
};

struct liodn_id_table liodn_bases[] = {
	[FSL_HW_PORTAL_SEC]  = SET_LIODN_BASE_2(96, 106),
#ifdef CONFIG_SYS_DPAA_FMAN
	[FSL_HW_PORTAL_FMAN1] = SET_LIODN_BASE_1(32),
#if (CONFIG_SYS_NUM_FMAN == 2)
	[FSL_HW_PORTAL_FMAN2] = SET_LIODN_BASE_1(64),
#endif
#endif
#ifdef CONFIG_SYS_DPAA_PME
	[FSL_HW_PORTAL_PME]   = SET_LIODN_BASE_2(116, 133),
#endif
};

int liodn_tbl_sz = ARRAY_SIZE(liodn_tbl);
int fman1_liodn_tbl_sz = ARRAY_SIZE(fman1_liodn_tbl);
int fman2_liodn_tbl_sz = ARRAY_SIZE(fman2_liodn_tbl);
int sec_liodn_tbl_sz = ARRAY_SIZE(sec_liodn_tbl);
