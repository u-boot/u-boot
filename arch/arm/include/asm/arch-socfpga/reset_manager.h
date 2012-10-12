/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef	_RESET_MANAGER_H_
#define	_RESET_MANAGER_H_

void reset_cpu(ulong addr);
void reset_deassert_peripherals_handoff(void);

struct socfpga_reset_manager {
	u32	padding1;
	u32	ctrl;
	u32	padding2;
	u32	padding3;
	u32	mpu_mod_reset;
	u32	per_mod_reset;
	u32	per2_mod_reset;
	u32	brg_mod_reset;
};

#define RSTMGR_CTRL_SWWARMRSTREQ_LSB 1

#endif /* _RESET_MANAGER_H_ */
