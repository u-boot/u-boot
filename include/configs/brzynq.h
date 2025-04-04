/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Config file for BR Zynq board
 *
 * Copyright (C) 2024
 * B&R Industrial Automation GmbH - http://www.br-automation.com/
 */

#ifndef __CONFIG_BRZYNQ_H__
#define __CONFIG_BRZYNQ_H__

/* Increase PHY_ANEG_TIMEOUT since the FPGA needs some setup time */
#if IS_ENABLED(CONFIG_SPL_FPGA)
#define PHY_ANEG_TIMEOUT  8000
#endif

/* Use top mapped SRAM */
#define CFG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CFG_SYS_INIT_RAM_SIZE	0x2000

#endif /* __CONFIG_BRZYNQ_H__ */
