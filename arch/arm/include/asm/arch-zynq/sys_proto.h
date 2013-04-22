/*
 * Copyright (c) 2013 Xilinx Inc.
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

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

extern void zynq_slcr_lock(void);
extern void zynq_slcr_unlock(void);
extern void zynq_slcr_cpu_reset(void);
extern void zynq_slcr_gem_clk_setup(u32 gem_id, u32 rclk, u32 clk);
extern void zynq_slcr_devcfg_disable(void);
extern void zynq_slcr_devcfg_enable(void);
extern u32 zynq_slcr_get_idcode(void);

/* Driver extern functions */
extern int zynq_sdhci_init(u32 regbase);

#endif /* _SYS_PROTO_H_ */
