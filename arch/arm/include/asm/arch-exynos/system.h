/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

void set_usbhost_mode(unsigned int mode);
void set_system_display_ctrl(void);
int exynos_lcd_early_init(const void *blob);

/* Initialize the Parade dP<->LVDS bridge if present */
int parade_init(const void *blob);

#endif	/* _EXYNOS4_SYSTEM_H */
