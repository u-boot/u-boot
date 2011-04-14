/*
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
 *
 */

extern int mergerbox_init_fpga(void);

extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_status_fn(int cookie);
extern int fpga_config_fn(int assert, int flush, int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_wr_fn(void *buf, size_t len, int flush, int cookie);
extern int fpga_null_fn(int cookie);
