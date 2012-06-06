/*
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#ifndef __MACH_MX5_IOMUX_H__
#define __MACH_MX5_IOMUX_H__

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>

typedef unsigned int iomux_pin_name_t;

/* various IOMUX output functions */
typedef enum iomux_config {
	IOMUX_CONFIG_ALT0,	/*!< used as alternate function 0 */
	IOMUX_CONFIG_ALT1,	/*!< used as alternate function 1 */
	IOMUX_CONFIG_ALT2,	/*!< used as alternate function 2 */
	IOMUX_CONFIG_ALT3,	/*!< used as alternate function 3 */
	IOMUX_CONFIG_ALT4,	/*!< used as alternate function 4 */
	IOMUX_CONFIG_ALT5,	/*!< used as alternate function 5 */
	IOMUX_CONFIG_ALT6,	/*!< used as alternate function 6 */
	IOMUX_CONFIG_ALT7,	/*!< used as alternate function 7 */
	IOMUX_CONFIG_GPIO,	/*!< added to help user use GPIO mode */
	IOMUX_CONFIG_SION = 0x1 << 4,	/*!< used as LOOPBACK:MUX SION bit */
} iomux_pin_cfg_t;

/* various IOMUX pad functions */
typedef enum iomux_pad_config {
	PAD_CTL_SRE_SLOW = 0x0 << 0,	/* Slow slew rate */
	PAD_CTL_SRE_FAST = 0x1 << 0,	/* Fast slew rate */
	PAD_CTL_DRV_LOW = 0x0 << 1,	/* Low drive strength */
	PAD_CTL_DRV_MEDIUM = 0x1 << 1,	/* Medium drive strength */
	PAD_CTL_DRV_HIGH = 0x2 << 1,	/* High drive strength */
	PAD_CTL_DRV_MAX = 0x3 << 1,	/* Max drive strength */
	PAD_CTL_ODE_OPENDRAIN_NONE = 0x0 << 3,	/* Opendrain disable */
	PAD_CTL_ODE_OPENDRAIN_ENABLE = 0x1 << 3,/* Opendrain enable */
	PAD_CTL_100K_PD = 0x0 << 4,	/* 100Kohm pulldown */
	PAD_CTL_47K_PU = 0x1 << 4,	/* 47Kohm pullup */
	PAD_CTL_100K_PU = 0x2 << 4,	/* 100Kohm pullup */
	PAD_CTL_22K_PU = 0x3 << 4,	/* 22Kohm pullup */
	PAD_CTL_PUE_KEEPER = 0x0 << 6,	/* enable pulldown */
	PAD_CTL_PUE_PULL = 0x1 << 6,	/* enable pullup */
	PAD_CTL_PKE_NONE = 0x0 << 7,	/* Disable pullup/pulldown */
	PAD_CTL_PKE_ENABLE = 0x1 << 7,	/* Enable pullup/pulldown */
	PAD_CTL_HYS_NONE = 0x0 << 8,	/* Hysteresis disabled */
	PAD_CTL_HYS_ENABLE = 0x1 << 8,	/* Hysteresis enabled */
	PAD_CTL_DDR_INPUT_CMOS = 0x0 << 9,/* DDR input CMOS */
	PAD_CTL_DDR_INPUT_DDR = 0x1 << 9,/* DDR input DDR */
	PAD_CTL_DRV_VOT_LOW = 0x0 << 13, /* Low voltage mode */
	PAD_CTL_DRV_VOT_HIGH = 0x1 << 13,/* High voltage mode */
} iomux_pad_config_t;

/* various IOMUX input functions */
typedef enum iomux_input_config {
	INPUT_CTL_PATH0 = 0x0,
	INPUT_CTL_PATH1,
	INPUT_CTL_PATH2,
	INPUT_CTL_PATH3,
	INPUT_CTL_PATH4,
	INPUT_CTL_PATH5,
	INPUT_CTL_PATH6,
	INPUT_CTL_PATH7,
} iomux_input_config_t;

void mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t config);
void mxc_free_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t config);
void mxc_iomux_set_pad(iomux_pin_name_t pin, u32 config);
unsigned int mxc_iomux_get_pad(iomux_pin_name_t pin);
void mxc_iomux_set_input(iomux_input_select_t input, u32 config);

#endif				/*  __MACH_MX5_IOMUX_H__ */
