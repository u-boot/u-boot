/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __GDSYS_FPGA_H
#define __GDSYS_FPGA_H

int init_func_fpga(void);

enum {
	FPGA_STATE_DONE_FAILED = 1 << 0,
	FPGA_STATE_REFLECTION_FAILED = 1 << 1,
	FPGA_STATE_PLATFORM = 1 << 2,
};

int get_fpga_state(unsigned dev);
void print_fpga_state(unsigned dev);

typedef struct ihs_gpio {
	u16 read;
	u16 clear;
	u16 set;
} ihs_gpio_t;

typedef struct ihs_i2c {
	u16 write_mailbox;
	u16 write_mailbox_ext;
	u16 read_mailbox;
	u16 read_mailbox_ext;
} ihs_i2c_t;

typedef struct ihs_osd {
	u16 version;
	u16 features;
	u16 control;
	u16 xy_size;
	u16 xy_scale;
	u16 x_pos;
	u16 y_pos;
} ihs_osd_t;

#ifdef CONFIG_IO
typedef struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[5];	/* 0x0008 */
	u16 quad_serdes_reset;	/* 0x0012 */
	u16 reserved_1[8181];	/* 0x0014 */
	u16 reflection_high;	/* 0x3ffe */
} ihs_fpga_t;
#endif

#ifdef CONFIG_IO64
typedef struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[5];	/* 0x0008 */
	u16 quad_serdes_reset;	/* 0x0012 */
	u16 reserved_1[502];	/* 0x0014 */
	u16 ch0_status_int;	/* 0x0400 */
	u16 ch0_config_int;	/* 0x0402 */
	u16 reserved_2[126];	/* 0x0404 */
	u16 ch0_hicb_status_int;/* 0x0500 */
	u16 ch0_hicb_config_int;/* 0x0502 */
	u16 reserved_3[7549];	/* 0x0504 */
	u16 reflection_high;	/* 0x3ffe */
} ihs_fpga_t;
#endif

#ifdef CONFIG_IOCON
typedef struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[6];	/* 0x0008 */
	ihs_gpio_t gpio;	/* 0x0014 */
	u16 mpc3w_control;	/* 0x001a */
	u16 reserved_1[19];	/* 0x001c */
	u16 videocontrol;	/* 0x0042 */
	u16 reserved_2[93];	/* 0x0044 */
	u16 reflection_high;	/* 0x00fe */
	ihs_osd_t osd;		/* 0x0100 */
	u16 reserved_3[88];	/* 0x010e */
	u16 videomem;		/* 0x0800 */
} ihs_fpga_t;
#endif

#ifdef CONFIG_DLVISION_10G
typedef struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[10];	/* 0x0008 */
	u16 extended_interrupt; /* 0x001c */
	u16 reserved_1[9];	/* 0x001e */
	ihs_i2c_t i2c;		/* 0x0030 */
	u16 reserved_2[16];	/* 0x0038 */
	u16 mpc3w_control;	/* 0x0058 */
	u16 reserved_3[34];	/* 0x005a */
	u16 videocontrol;	/* 0x009e */
	u16 reserved_4[176];	/* 0x00a0 */
	ihs_osd_t osd;		/* 0x0200 */
	u16 reserved_5[761];	/* 0x020e */
	u16 videomem;		/* 0x0800 */
} ihs_fpga_t;
#endif

#endif
