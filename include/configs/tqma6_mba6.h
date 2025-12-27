/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2017 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 *
 * Configuration settings for the TQ-Systems TQMa6<Q,D,DL,S> module on
 * MBa6 starter kit
 */

#ifndef __CONFIG_TQMA6_MBA6_H
#define __CONFIG_TQMA6_MBA6_H

#define CFG_FEC_MXC_PHYADDR		0x03

#define CFG_MXC_UART_BASE		UART2_BASE
#define CONSOLE_DEV		"ttymxc1"

#endif /* __CONFIG_TQMA6_MBA6_H */
