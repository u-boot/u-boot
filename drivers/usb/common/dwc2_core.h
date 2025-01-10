/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 *
 */

#ifndef __DWC2_CORE_H_
#define __DWC2_CORE_H_

struct dwc2_global_regs {
	u32 gotgctl;	/* 0x000 */
	u32 gotgint;
	u32 gahbcfg;
	u32 gusbcfg;
	u32 grstctl;	/* 0x010 */
	u32 gintsts;
	u32 gintmsk;
	u32 grxstsr;
	u32 grxstsp;	/* 0x020 */
	u32 grxfsiz;
	u32 gnptxfsiz;
	u32 gnptxsts;
	u32 gi2cctl;	/* 0x030 */
	u32 gpvndctl;
	u32 ggpio;
	u32 guid;
	u32 gsnpsid;	/* 0x040 */
	u32 ghwcfg1;
	u32 ghwcfg2;
	u32 ghwcfg3;
	u32 ghwcfg4;	/* 0x050 */
	u32 glpmcfg;
	u32 gpwrdn;
	u32 gdfifocfg;
	u32 gadpctl;	/* 0x060 */
	u32 grefclk;
	u32 gintmsk2;
	u32 gintsts2;
	u8  _pad_from_0x70_to_0x100[0x100 - 0x70];
	u32 hptxfsiz;	/* 0x100 */
	u32 dptxfsizn[15];
	u8  _pad_from_0x140_to_0x400[0x400 - 0x140];
};

struct dwc2_hc_regs {
	u32 hcchar;	/* 0x500 + 0x20 * ch */
	u32 hcsplt;
	u32 hcint;
	u32 hcintmsk;
	u32 hctsiz;
	u32 hcdma;
	u32 reserved;
	u32 hcdmab;
};

struct dwc2_host_regs {
	u32 hcfg;	/* 0x400 */
	u32 hfir;
	u32 hfnum;
	u32 _pad_0x40c;
	u32 hptxsts;	/* 0x410 */
	u32 haint;
	u32 haintmsk;
	u32 hflbaddr;
	u8  _pad_from_0x420_to_0x440[0x440 - 0x420];
	u32 hprt0;	/* 0x440 */
	u8  _pad_from_0x444_to_0x500[0x500 - 0x444];
	struct dwc2_hc_regs hc[16];	/* 0x500 */
	u8  _pad_from_0x700_to_0x800[0x800 - 0x700];
};

/* Device Logical IN Endpoint-Specific Registers */
struct dwc2_dev_in_endp {
	u32 diepctl;	/* 0x900 + 0x20 * ep */
	u32 reserved0;
	u32 diepint;
	u32 reserved1;
	u32 dieptsiz;
	u32 diepdma;
	u32 reserved2;
	u32 diepdmab;
};

/* Device Logical OUT Endpoint-Specific Registers */
struct dwc2_dev_out_endp {
	u32 doepctl;	/* 0xB00 + 0x20 * ep */
	u32 reserved0;
	u32 doepint;
	u32 reserved1;
	u32 doeptsiz;
	u32 doepdma;
	u32 reserved2;
	u32 doepdmab;
};

struct dwc2_device_regs {
	u32 dcfg;	/* 0x800 */
	u32 dctl;
	u32 dsts;
	u32 _pad_0x80c;
	u32 diepmsk;	/* 0x810 */
	u32 doepmsk;
	u32 daint;
	u32 daintmsk;
	u32 dtknqr1;	/* 0x820 */
	u32 dtknqr2;
	u32 dvbusdis;
	u32 dvbuspulse;
	u32 dtknqr3;	/* 0x830 */
	u32 dtknqr4;
	u8  _pad_from_0x838_to_0x900[0x900 - 0x838];
	struct dwc2_dev_in_endp  in_endp[16];	/* 0x900 */
	struct dwc2_dev_out_endp out_endp[16];	/* 0xB00 */
};

struct dwc2_core_regs {
	struct dwc2_global_regs  global_regs;	/* 0x000 */
	struct dwc2_host_regs    host_regs;	/* 0x400 */
	struct dwc2_device_regs  device_regs;	/* 0x800 */
	u8  _pad_from_0xd00_to_0xe00[0xe00 - 0xd00];
	u32 pcgcctl;				/* 0xe00 */
	u8  _pad_from_0xe04_to_0x1000[0x1000 - 0xe04];
	u8  ep_fifo[16][0x1000];		/* 0x1000 */
};

#endif /* __DWC2_CORE_H_ */
