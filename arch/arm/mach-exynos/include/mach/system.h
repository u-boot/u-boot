/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_SYSTEM_H_
#define __ASM_ARM_ARCH_SYSTEM_H_

#ifndef __ASSEMBLY__
struct exynos4_sysreg {
	unsigned char	res1[0x210];
	unsigned int	display_ctrl;
	unsigned int	display_ctrl2;
	unsigned int	camera_control;
	unsigned int	audio_endian;
	unsigned int	jtag_con;
};

struct exynos5_sysreg {
	unsigned char	res1[0x214];
	unsigned int	disp1blk_cfg;
	unsigned int	disp2blk_cfg;
	unsigned int	hdcp_e_fuse;
	unsigned int	gsclblk_cfg0;
	unsigned int	gsclblk_cfg1;
	unsigned int	reserved;
	unsigned int	ispblk_cfg;
	unsigned int	usb20phy_cfg;
	unsigned char	res2[0x29c];
	unsigned int	mipi_dphy;
	unsigned int	dptx_dphy;
	unsigned int	phyclk_sel;
};
#endif

#define USB20_PHY_CFG_HOST_LINK_EN	(1 << 0)

/* Move 0xd3 value to CPSR register to enable SVC mode */
#define svc32_mode_en() __asm__ __volatile__				\
			("@ I&F disable, Mode: 0x13 - SVC\n\t"		\
			 "msr     cpsr_c, %0\n\t" : : "r"(0x13|0xC0))

/* Set program counter with the given value */
#define set_pc(x) __asm__ __volatile__ ("mov     pc, %0\n\t" : : "r"(x))

/* Branch to the given location */
#define branch_bx(x) __asm__ __volatile__ ("bx	%0\n\t" : : "r"(x))

/* Read Main Id register */
#define mrc_midr(x) __asm__ __volatile__	\
			("mrc     p15, 0, %0, c0, c0, 0\n\t" : "=r"(x) : )

/* Read Multiprocessor Affinity Register */
#define mrc_mpafr(x) __asm__ __volatile__	\
			("mrc     p15, 0, %0, c0, c0, 5\n\t" : "=r"(x) : )

/* Read System Control Register */
#define mrc_sctlr(x) __asm__ __volatile__	\
			("mrc     p15, 0, %0, c1, c0, 0\n\t" : "=r"(x) : )

/* Read Auxiliary Control Register */
#define mrc_auxr(x) __asm__ __volatile__	\
			("mrc     p15, 0, %0, c1, c0, 1\n\t" : "=r"(x) : )

/* Read L2 Control register */
#define mrc_l2_ctlr(x) __asm__ __volatile__	\
			("mrc     p15, 1, %0, c9, c0, 2\n\t" : "=r"(x) : )

/* Read L2 Auxilliary Control register */
#define mrc_l2_aux_ctlr(x) __asm__ __volatile__	\
			("mrc     p15, 1, %0, c15, c0, 0\n\t" : "=r"(x) : )

/* Write System Control Register */
#define mcr_sctlr(x) __asm__ __volatile__	\
			("mcr     p15, 0, %0, c1, c0, 0\n\t" : : "r"(x))

/* Write Auxiliary Control Register */
#define mcr_auxr(x) __asm__ __volatile__	\
			("mcr     p15, 0, %0, c1, c0, 1\n\t" : : "r"(x))

/* Invalidate all instruction caches to PoU */
#define mcr_icache(x) __asm__ __volatile__	\
			("mcr     p15, 0, %0, c7, c5, 0\n\t" : : "r"(x))

/* Invalidate unified TLB */
#define mcr_tlb(x) __asm__ __volatile__	\
			("mcr     p15, 0, %0, c8, c7, 0\n\t" : : "r"(x))

/* Write L2 Control register */
#define mcr_l2_ctlr(x) __asm__ __volatile__	\
			("mcr     p15, 1, %0, c9, c0, 2\n\t" : : "r"(x))

/* Write L2 Auxilliary Control register */
#define mcr_l2_aux_ctlr(x) __asm__ __volatile__	\
			("mcr     p15, 1, %0, c15, c0, 0\n\t" : : "r"(x))

void set_usbhost_mode(unsigned int mode);
void set_system_display_ctrl(void);

#endif	/* _EXYNOS4_SYSTEM_H */
