/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_SYSTEM_MANAGER_H_
#define	_SYSTEM_MANAGER_H_

#ifndef __ASSEMBLY__

void sysmgr_pinmux_init(void);

/* declaration for handoff table type */
extern unsigned long sys_mgr_init_table[CONFIG_HPS_PINMUX_NUM];

#endif


#define CONFIG_SYSMGR_PINMUXGRP_OFFSET	(0x400)

#define SYSMGR_SDMMC_CTRL_SET(smplsel, drvsel)	\
	((((drvsel) << 0) & 0x7) | (((smplsel) << 3) & 0x38))

struct socfpga_system_manager {
	u32	siliconid1;
	u32	siliconid2;
	u32	_pad_0x8_0xf[2];
	u32	wddbg;
	u32	bootinfo;
	u32	hpsinfo;
	u32	parityinj;
	u32	fpgaintfgrp_gbl;
	u32	fpgaintfgrp_indiv;
	u32	fpgaintfgrp_module;
	u32	_pad_0x2c_0x2f;
	u32	scanmgrgrp_ctrl;
	u32	_pad_0x34_0x3f[3];
	u32	frzctrl_vioctrl;
	u32	_pad_0x44_0x4f[3];
	u32	frzctrl_hioctrl;
	u32	frzctrl_src;
	u32	frzctrl_hwctrl;
	u32	_pad_0x5c_0x5f;
	u32	emacgrp_ctrl;
	u32	emacgrp_l3master;
	u32	_pad_0x68_0x6f[2];
	u32	dmagrp_ctrl;
	u32	dmagrp_persecurity;
	u32	_pad_0x78_0x7f[2];
	u32	iswgrp_handoff[8];
	u32	_pad_0xa0_0xbf[8];
	u32	romcodegrp_ctrl;
	u32	romcodegrp_cpu1startaddr;
	u32	romcodegrp_initswstate;
	u32	romcodegrp_initswlastld;
	u32	romcodegrp_bootromswstate;
	u32	__pad_0xd4_0xdf[3];
	u32	romcodegrp_warmramgrp_enable;
	u32	romcodegrp_warmramgrp_datastart;
	u32	romcodegrp_warmramgrp_length;
	u32	romcodegrp_warmramgrp_execution;
	u32	romcodegrp_warmramgrp_crc;
	u32	__pad_0xf4_0xff[3];
	u32	romhwgrp_ctrl;
	u32	_pad_0x104_0x107;
	u32	sdmmcgrp_ctrl;
	u32	sdmmcgrp_l3master;
	u32	nandgrp_bootstrap;
	u32	nandgrp_l3master;
	u32	usbgrp_l3master;
	u32	_pad_0x11c_0x13f[9];
	u32	eccgrp_l2;
	u32	eccgrp_ocram;
	u32	eccgrp_usb0;
	u32	eccgrp_usb1;
	u32	eccgrp_emac0;
	u32	eccgrp_emac1;
	u32	eccgrp_dma;
	u32	eccgrp_can0;
	u32	eccgrp_can1;
	u32	eccgrp_nand;
	u32	eccgrp_qspi;
	u32	eccgrp_sdmmc;
};

#endif /* _SYSTEM_MANAGER_H_ */
