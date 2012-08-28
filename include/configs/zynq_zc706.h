/*
 * (C) Copyright 2012 Xilinx
 *
 * Configuration settings for the Xilinx Zynq ZC706 board.
 * See zynq_zc702.h as most of the board headers for both
 * are similar
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

#ifndef __CONFIG_ZYNQ_ZC706_H
#define __CONFIG_ZYNQ_ZC706_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ZC706 /* Board */

/* ZC702 board headers */
#include <configs/zynq_zc702.h>

#undef CONFIG_ZC702

/* ZC706 QSPI settings */
#define CONFIG_SPI_FLASH_SPANSION

#endif /* __CONFIG_ZYNQ_ZC706_H */
