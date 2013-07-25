/*
 *
 * (c) 2009 Emcraft Systems, Ilya Yanok <yanok@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef QONG_FPGA_H
#define QONG_FPGA_H

#define QONG_FPGA_CTRL_BASE		CONFIG_FPGA_BASE
#define QONG_FPGA_CTRL_VERSION		(QONG_FPGA_CTRL_BASE + 0x00000000)
#define QONG_FPGA_PERIPH_SIZE		(1 << 24)

#define	QONG_FPGA_TCK_PIN		26
#define	QONG_FPGA_TMS_PIN		25
#define	QONG_FPGA_TDI_PIN		8
#define	QONG_FPGA_TDO_PIN		7
#define	QONG_FPGA_RST_PIN		48
#define	QONG_FPGA_IRQ_PIN		40

int qong_fpga_init(void);
#endif /* QONG_FPGA_H */
