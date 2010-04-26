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

#if !defined(CONFIG_PIGGY_MAC_ADRESS_OFFSET)
#define CONFIG_PIGGY_MAC_ADRESS_OFFSET	0
#endif

int ethernet_present(void);
int ivm_read_eeprom(void);

#ifdef CONFIG_KEYMILE_HDLC_ENET
int keymile_hdlc_enet_initialize(bd_t *bis);
#endif

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
#endif /* __KEYMILE_COMMON_H */
