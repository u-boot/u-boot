/*
 *
 * Adaptive Body Bias programming sequence for OMAP5 family
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Andrii Tseglytskyi <andrii.tseglytskyi@ti.com>
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

#include <common.h>
#include <asm/omap_common.h>
#include <asm/io.h>

/*
 * Setup LDOVBB for OMAP5.
 * On OMAP5+ some ABB settings are fused. They are handled
 * in the following way:
 *
 * 1. corresponding EFUSE register contains ABB enable bit
 *    and VSET value
 * 2. If ABB enable bit is set to 1, than ABB should be
 *    enabled, otherwise ABB should be disabled
 * 3. If ABB is enabled, than VSET value should be copied
 *    to corresponding MUX control register
 */
s8 abb_setup_ldovbb(u32 fuse, u32 ldovbb)
{
	u32 vset;

	/*
	 * ABB parameters must be properly fused
	 * otherwise ABB should be disabled
	 */
	vset = readl(fuse);
	if (!(vset & OMAP5_ABB_FUSE_ENABLE_MASK))
		return -1;

	/* prepare VSET value for LDOVBB mux register */
	vset &= OMAP5_ABB_FUSE_VSET_MASK;
	vset >>= ffs(OMAP5_ABB_FUSE_VSET_MASK) - 1;
	vset <<= ffs(OMAP5_ABB_LDOVBBMPU_VSET_OUT_MASK) - 1;
	vset |= OMAP5_ABB_LDOVBBMPU_MUX_CTRL_MASK;

	/* setup LDOVBB using fused value */
	clrsetbits_le32(ldovbb,  OMAP5_ABB_LDOVBBMPU_VSET_OUT_MASK, vset);

	return 0;
}
