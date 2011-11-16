/*
 * Freescale i.MX28 MX28 specific functions
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __MX28_H__
#define __MX28_H__

int mx28_reset_block(struct mx28_register *reg);
int mx28_wait_mask_set(struct mx28_register *reg, uint32_t mask, int timeout);
int mx28_wait_mask_clr(struct mx28_register *reg, uint32_t mask, int timeout);

int mxsmmc_initialize(bd_t *bis, int id, int (*wp)(int));

#endif	/* __MX28_H__ */
