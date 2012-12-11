/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TEGRA30_FLOW_H_
#define _TEGRA30_FLOW_H_

struct flow_ctlr {
	u32 halt_cpu_events;
	u32 halt_cop_events;
	u32 cpu_csr;
	u32 cop_csr;
	u32 xrq_events;
	u32 halt_cpu1_events;
	u32 cpu1_csr;
	u32 halt_cpu2_events;
	u32 cpu2_csr;
	u32 halt_cpu3_events;
	u32 cpu3_csr;
	u32 cluster_control;
};

#endif	/* _TEGRA30_FLOW_H_ */
