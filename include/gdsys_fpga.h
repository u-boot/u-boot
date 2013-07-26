/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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

int fpga_set_reg(u32 fpga, u16 *reg, off_t regoff, u16 data);
int fpga_get_reg(u32 fpga, u16 *reg, off_t regoff, u16 *data);

extern struct ihs_fpga *fpga_ptr[];

#define FPGA_SET_REG(ix, fld, val) \
	fpga_set_reg((ix), \
		     &fpga_ptr[ix]->fld, \
		     offsetof(struct ihs_fpga, fld), \
		     val)

#define FPGA_GET_REG(ix, fld, val) \
	fpga_get_reg((ix), \
		     &fpga_ptr[ix]->fld, \
		     offsetof(struct ihs_fpga, fld), \
		     val)

struct ihs_gpio {
	u16 read;
	u16 clear;
	u16 set;
};

struct ihs_i2c {
	u16 write_mailbox;
	u16 write_mailbox_ext;
	u16 read_mailbox;
	u16 read_mailbox_ext;
};

struct ihs_osd {
	u16 version;
	u16 features;
	u16 control;
	u16 xy_size;
	u16 xy_scale;
	u16 x_pos;
	u16 y_pos;
};

#ifdef CONFIG_NEO
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[8187];	/* 0x0008 */
	u16 reflection_high;	/* 0x3ffe */
};
#endif

#ifdef CONFIG_IO
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[5];	/* 0x0008 */
	u16 quad_serdes_reset;	/* 0x0012 */
	u16 reserved_1[8181];	/* 0x0014 */
	u16 reflection_high;	/* 0x3ffe */
};
#endif

#ifdef CONFIG_IO64

struct ihs_fpga_channel {
	u16 status_int;
	u16 config_int;
	u16 switch_connect_config;
	u16 tx_destination;
};

struct ihs_fpga_hicb {
	u16 status_int;
	u16 config_int;
};

struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[5];	/* 0x0008 */
	u16 quad_serdes_reset;	/* 0x0012 */
	u16 reserved_1[502];	/* 0x0014 */
	struct ihs_fpga_channel ch[32];		/* 0x0400 */
	struct ihs_fpga_channel hicb_ch[32];	/* 0x0500 */
	u16 reserved_2[7487];	/* 0x0580 */
	u16 reflection_high;	/* 0x3ffe */
};
#endif

#ifdef CONFIG_IOCON
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[6];	/* 0x0008 */
	struct ihs_gpio gpio;	/* 0x0014 */
	u16 mpc3w_control;	/* 0x001a */
	u16 reserved_1[19];	/* 0x001c */
	u16 videocontrol;	/* 0x0042 */
	u16 reserved_2[14];	/* 0x0044 */
	u16 mc_int;		/* 0x0060 */
	u16 mc_int_en;		/* 0x0062 */
	u16 mc_status;		/* 0x0064 */
	u16 mc_control;		/* 0x0066 */
	u16 mc_tx_data;		/* 0x0068 */
	u16 mc_tx_address;	/* 0x006a */
	u16 mc_tx_cmd;		/* 0x006c */
	u16 mc_res;		/* 0x006e */
	u16 mc_rx_cmd_status;	/* 0x0070 */
	u16 mc_rx_data;		/* 0x0072 */
	u16 reserved_3[69];	/* 0x0074 */
	u16 reflection_high;	/* 0x00fe */
	struct ihs_osd osd;	/* 0x0100 */
	u16 reserved_4[889];	/* 0x010e */
	u16 videomem[31736];	/* 0x0800 */
};
#endif

#ifdef CONFIG_DLVISION_10G
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[10];	/* 0x0008 */
	u16 extended_interrupt; /* 0x001c */
	u16 reserved_1[9];	/* 0x001e */
	struct ihs_i2c i2c;	/* 0x0030 */
	u16 reserved_2[16];	/* 0x0038 */
	u16 mpc3w_control;	/* 0x0058 */
	u16 reserved_3[34];	/* 0x005a */
	u16 videocontrol;	/* 0x009e */
	u16 reserved_4[176];	/* 0x00a0 */
	struct ihs_osd osd;	/* 0x0200 */
	u16 reserved_5[761];	/* 0x020e */
	u16 videomem[31736];	/* 0x0800 */
};
#endif

#endif
