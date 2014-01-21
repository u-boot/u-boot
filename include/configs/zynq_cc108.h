/*
 * (C) Copyright 2013 Xilinx
 *
 * Configuration settings for the Xilinx Zynq CC108 boards
 * See zynq-common.h for Zynq common configs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_ZYNQ_CC108_H
#define __CONFIG_ZYNQ_CC108_H

#define CONFIG_SYS_SDRAM_SIZE (512 * 1024 * 1024)

/*
 * UART is connected to EMIO, making it very likely that this board uses a
 * boot.bin which includes a bitstream, whose file size  exceeds the default
 * env offset. Hence the env offset is moved to the last MB of the QSPI.
 */
#define CONFIG_ENV_OFFSET	0xF00000

#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	7

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_SDHCI1
#define CONFIG_ZYNQ_QSPI
#define CONFIG_ZYNQ_BOOT_FREEBSD

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_CC108_H */
