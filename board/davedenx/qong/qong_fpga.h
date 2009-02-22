/*
 *
 * (c) 2009 Emcraft Systems, Ilya Yanok <yanok@emcraft.com>
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

#ifndef QONG_FPGA_H
#define QONG_FPGA_H

#ifdef CONFIG_QONG_FPGA
#define QONG_FPGA_CTRL_BASE		CONFIG_FPGA_BASE
#define QONG_FPGA_CTRL_VERSION		(QONG_FPGA_CTRL_BASE + 0x00000000)
#define QONG_FPGA_PERIPH_SIZE		(1 << 24)

#define	QONG_FPGA_TCK_PIN		26
#define	QONG_FPGA_TMS_PIN		25
#define	QONG_FPGA_TDI_PIN		8
#define	QONG_FPGA_TDO_PIN		7
#define	QONG_FPGA_RST_PIN		16
#define	QONG_FPGA_IRQ_PIN		8
#endif

#endif /* QONG_FPGA_H */
