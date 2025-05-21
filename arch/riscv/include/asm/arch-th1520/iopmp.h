/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */
#ifndef _ASM_ARCH_TH1520_IOPMP_H_
#define _ASM_ARCH_TH1520_IOPMP_H_

#define TH1520_IOPMP_EMMC		(void *)0xfffc0280c0
#define TH1520_IOPMP_SDIO0		(void *)0xfffc0290c0
#define TH1520_IOPMP_SDIO1		(void *)0xfffc02a0c0
#define TH1520_IOPMP_USB0		(void *)0xfffc02e0c0
#define TH1520_IOPMP_AO			(void *)0xffffc210c0
#define TH1520_IOPMP_AUD		(void *)0xffffc220c0
#define TH1520_IOPMP_CHIP_DBG		(void *)0xffffc370c0
#define TH1520_IOPMP_EIP120I		(void *)0xffff2200c0
#define TH1520_IOPMP_EIP120II		(void *)0xffff2300c0
#define TH1520_IOPMP_EIP120III		(void *)0xffff2400c0
#define TH1520_IOPMP_ISP0		(void *)0xfff40800c0
#define TH1520_IOPMP_ISP1		(void *)0xfff40810c0
#define TH1520_IOPMP_DW200		(void *)0xfff40820c0
#define TH1520_IOPMP_VIPRE		(void *)0xfff40830c0
#define TH1520_IOPMP_VENC		(void *)0xfffcc600c0
#define TH1520_IOPMP_VDEC		(void *)0xfffcc610c0
#define TH1520_IOPMP_G2D		(void *)0xfffcc620c0
#define TH1520_IOPMP_FCE		(void *)0xfffcc630c0
#define TH1520_IOPMP_NPU		(void *)0xffff01c0c0
#define TH1520_IOPMP_DPU0		(void *)0xffff5200c0
#define TH1520_IOPMP_DPU1		(void *)0xffff5210c0
#define TH1520_IOPMP_GPU		(void *)0xffff5220c0
#define TH1520_IOPMP_GMAC1		(void *)0xfffc0010c0
#define TH1520_IOPMP_GMAC2		(void *)0xfffc0020c0
#define TH1520_IOPMP_DMAC		(void *)0xffffc200c0
#define TH1520_IOPMP_TEE_DMAC		(void *)0xffff2500c0
#define TH1520_IOPMP_DSP0		(void *)0xffff0580c0
#define TH1520_IOPMP_DSP1		(void *)0xffff0590c0
#define TH1520_IOPMP_AUDIO		(void *)0xffffc220c0
#define TH1520_IOPMP_AUDIO0		(void *)0xffcb02e0c0
#define TH1520_IOPMP_AUDIO1		(void *)0xffcb02f0c0

#define TH1520_IOPMP_DEFAULT_ATTR	0xffffffff

#endif // _ASM_ARCH_TH1520_IOPMP_H_
