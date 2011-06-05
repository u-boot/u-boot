/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __KEYMILE_COMMON_H
#define __KEYMILE_COMMON_H

#define WRG_RESET	0x80
#define H_OPORTS_14	0x40
#define WRG_LED		0x02
#define WRL_BOOT	0x01

#define H_OPORTS_SCC4_ENA	0x10
#define H_OPORTS_SCC4_FD_ENA	0x04
#define H_OPORTS_FCC1_PW_DWN	0x01

#define PIGGY_PRESENT	0x80

struct km_bec_fpga {
	unsigned char	id;
	unsigned char	rev;
	unsigned char	oprth;
	unsigned char	oprtl;
	unsigned char	res1[3];
	unsigned char	bprth;
	unsigned char	bprtl;
	unsigned char	res2[6];
	unsigned char	prst;
	unsigned char	res3[0xfff0];
	unsigned char	pgy_id;
	unsigned char	pgy_rev;
	unsigned char	pgy_outputs;
	unsigned char	pgy_eth;
};

#define BFTICU_DIPSWITCH_MASK   0x0f

/*
 * BFTICU FPGA iomap
 * BFTICU is used on mgcoge and mgocge3ne
 */
struct bfticu_iomap {
	u8	xi_ena;		/* General defect enable */
	u8	pack1[3];
	u8	en_csn;
	u8	pack2;
	u8	safe_mem;
	u8	pack3;
	u8	id;
	u8	pack4;
	u8	rev;
	u8	build;
	u8	p_frc;
	u8	p_msk;
	u8	pack5[2];
	u8	xg_int;
	u8	pack6[15];
	u8	s_conf;
	u8	pack7;
	u8	dmx_conf12;
	u8	pack8;
	u8	s_clkslv;
	u8	pack9[11];
	u8	d_conf;
	u8	d_mask_ca;
	u8	d_pll_del;
	u8	pack10[16];
	u8	t_conf_ca;
	u8	t_mask_ca;
	u8	pack11[13];
	u8	m_def0;
	u8	m_def1;
	u8	m_def2;
	u8	m_def3;
	u8	m_def4;
	u8	m_def5;
	u8	m_def_trap0;
	u8	m_def_trap1;
	u8	m_def_trap2;
	u8	m_def_trap3;
	u8	m_def_trap4;
	u8	m_def_trap5;
	u8	m_mask_def0;
	u8	m_mask_def1;
	u8	m_mask_def2;
	u8	m_mask_def3;
	u8	m_mask_def4;
	u8	m_mask_def5;
	u8	m_def_mask0;
	u8	m_def_mask1;
	u8	m_def_mask2;
	u8	m_def_mask3;
	u8	m_def_mask4;
	u8	m_def_mask5;
	u8	m_def_pri;
	u8	pack12[11];
	u8	hw_status;
	u8	pack13;
	u8	hw_control1;
	u8	hw_control2;
	u8	hw_control3;
	u8	pack14[7];
	u8	led_on;		/* Leds */
	u8	pack15;
	u8	sfp_control;	/* SFP modules */
	u8	pack16;
	u8	alarm_control;	/* Alarm output */
	u8	pack17;
	u8	icps;		/* ICN clock pulse shaping */
	u8	mswitch;	/* Read mode switch */
	u8	pack18[6];
	u8	pb_dbug;
};

#if !defined(CONFIG_PIGGY_MAC_ADRESS_OFFSET)
#define CONFIG_PIGGY_MAC_ADRESS_OFFSET	0
#endif

int ethernet_present(void);
int ivm_read_eeprom(void);
void set_pin(int state, unsigned long mask);

int set_km_env(void);
int fdt_set_node_and_value(void *blob,
			char *nodename,
			char *regname,
			void *var,
			int size);
int fdt_get_node_and_value(void *blob,
				char *nodename,
				char *propname,
				void **var);

int i2c_soft_read_pin(void);
int i2c_make_abort(void);
#endif /* __KEYMILE_COMMON_H */
