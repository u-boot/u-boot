/*
 * Copyright (C) 2013 - 2015 Markus Niebel <Markus.Niebel@tq-group.com>
 *
 * Configuration settings for the TQ Systems TQMa6<Q,S> module.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TQMA6_MBA6_H
#define __CONFIG_TQMA6_MBA6_H

#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define CONFIG_DEFAULT_FDT_FILE		"imx6dl-mba6x.dtb"
#elif defined(CONFIG_MX6Q) || defined(CONFIG_MX6D)
#define CONFIG_DEFAULT_FDT_FILE		"imx6q-mba6x.dtb"
#endif

#define CONFIG_DTT_SENSORS		{ 0, 1 }

#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"

#define CONFIG_FEC_MXC_PHYADDR		0x03
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_KSZ9031

#define CONFIG_MXC_UART_BASE		UART2_BASE
#define CONFIG_CONSOLE_DEV		"ttymxc1"

#endif /* __CONFIG_TQMA6_MBA6_H */
