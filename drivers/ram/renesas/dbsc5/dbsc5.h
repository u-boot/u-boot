// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Renesas Electronics Corp.
 */

#ifndef __DRIVERS_RAM_RENESAS_DBSC5_DBSC5_H__
#define __DRIVERS_RAM_RENESAS_DBSC5_DBSC5_H__

/*
 * DBSC5 ... 0xe678_0000..0xe67fffff
 * - AXMM_BASE		0xe6780000	MM (DDR Hier) MM AXI Router - Region 0
 * - DBSC_A_BASE	0xe6790000	MM (DDR Hier) DBSC0A - Region 0
 * - CCI_BASE		0xe67A0000	MM (DDR Hier) FBA for MM
 * - DBSC_D_BASE	0xE67A4000	MM (DDR Hier) DBSC0D - Region 0
 * - QOS_BASE		0xe67E0000	MM (DDR Hier) M-STATQ (64kiB)
 */
#define DBSC5_AXMM_OFFSET			0x00000
#define DBSC5_DBSC_A_OFFSET			0x10000
#define DBSC5_CCI_OFFSET			0x20000
#define DBSC5_DBSC_D_OFFSET			0x24000
#define DBSC5_QOS_OFFSET			0x60000

struct renesas_dbsc5_data {
	const char		*clock_node;
	const char		*reset_node;
	const char		*otp_node;
};

#endif /* __DRIVERS_RAM_RENESAS_DBSC5_DBSC5_H__ */
