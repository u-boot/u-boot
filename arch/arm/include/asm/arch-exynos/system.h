/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
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

#endif	/* _EXYNOS4_SYSTEM_H */
