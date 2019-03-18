/*
 * CPSW Ethernet Switch Driver
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _CPSW_H_
#define _CPSW_H_

struct cpsw_slave_data {
	u32		slave_reg_ofs;
	u32		sliver_reg_ofs;
	int		phy_addr;
	int		phy_if;
	int		phy_of_handle;
};

enum {
	CPSW_CTRL_VERSION_1 = 0,
	CPSW_CTRL_VERSION_2	/* am33xx like devices */
};

struct cpsw_platform_data {
	u32	mdio_base;
	u32	cpsw_base;
	u32	mac_id;
	u32	gmii_sel;
	int	mdio_div;
	int	channels;	/* number of cpdma channels (symmetric)	*/
	u32	cpdma_reg_ofs;	/* cpdma register offset		*/
	int	slaves;		/* number of slave cpgmac ports		*/
	u32	ale_reg_ofs;	/* address lookup engine reg offset	*/
	int	ale_entries;	/* ale table size			*/
	u32	host_port_reg_ofs;	/* cpdma host port registers	*/
	u32	hw_stats_reg_ofs;	/* cpsw hw stats counters	*/
	u32	bd_ram_ofs;		/* Buffer Descriptor RAM offset */
	u32	mac_control;
	struct cpsw_slave_data	*slave_data;
	void	(*control)(int enabled);
	u32	host_port_num;
	u32	active_slave;
	bool	rmii_clock_external;
	u8	version;
	const char *phy_sel_compat;
};

int cpsw_register(struct cpsw_platform_data *data);
int ti_cm_get_macid(struct udevice *dev, int slave, u8 *mac_addr);
int cpsw_get_slave_phy_addr(struct udevice *dev, int slave);

#endif /* _CPSW_H_  */
