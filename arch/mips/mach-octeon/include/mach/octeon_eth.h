/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_ETH_H__
#define __OCTEON_ETH_H__

#include <phy.h>
#include <miiphy.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/octeon_fdt.h>

struct eth_device;

/** Ethernet device private data structure for octeon ethernet */
struct octeon_eth_info {
	u64 link_state;
	u32 port;		   /** ipd port */
	u32 interface;		   /** Port interface */
	u32 index;		   /** port index on interface */
	int node;		   /** OCX node number */
	u32 initted_flag;	   /** 0 if port not initialized */
	struct mii_dev *mii_bus;   /** MII bus for PHY */
	struct phy_device *phydev; /** PHY device */
	struct eth_device *ethdev; /** Eth device this priv is part of */
	int mii_addr;
	int phy_fdt_offset;		    /** Offset of PHY info in device tree */
	int fdt_offset;			    /** Offset of Eth interface in DT */
	int phy_offset;			    /** Offset of PHY dev in device tree */
	enum cvmx_phy_type phy_device_type; /** Type of PHY */
	/* current link status, use to reconfigure on status changes */
	u64 packets_sent;
	u64 packets_received;
	u32 link_speed : 2;
	u32 link_duplex : 1;
	u32 link_status : 1;
	u32 loopback : 1;
	u32 enabled : 1;
	u32 is_c45 : 1;		    /** Set if we need to use clause 45 */
	u32 vitesse_sfp_config : 1; /** Need Vitesse SFP config */
	u32 ti_gpio_config : 1;	    /** Need TI GPIO config */
	u32 bgx_mac_set : 1;	    /** Has the BGX MAC been set already */
	u64 last_bgx_mac;	    /** Last BGX MAC address set */
	u64 gmx_base;		    /** Base address to access GMX CSRs */
	bool mod_abs;		    /** True if module is absent */

	/**
	 * User defined function to check if a SFP+ module is absent or not.
	 *
	 * @param	dev	Ethernet device
	 * @param	data	User supplied data
	 */
	int (*check_mod_abs)(struct eth_device *dev, void *data);

	/** User supplied data for check_mod_abs */
	void *mod_abs_data;
	/**
	 * Called to check the status of a port.  This is used for some
	 * Vitesse and Inphi phys to probe the sFP adapter.
	 */
	int (*phy_port_check)(struct phy_device *dev);
	/**
	 * Called whenever mod_abs changes state
	 *
	 * @param	dev	Ethernet device
	 * @param	mod_abs	True if module is absent
	 *
	 * @return	0 for success, otherwise error
	 */
	int (*mod_abs_changed)(struct eth_device *dev, bool mod_abs);
	/** SDK phy information data structure */
	cvmx_phy_info_t phy_info;
#ifdef CONFIG_OCTEON_SFP
	/** Information about connected SFP/SFP+/SFP28/QSFP+/QSFP28 module */
	struct octeon_sfp_info sfp;
#endif
};

/**
 * Searches for an ethernet device based on interface and index.
 *
 * @param interface - interface number to search for
 * @param index - index to search for
 *
 * @returns pointer to ethernet device or NULL if not found.
 */
struct eth_device *octeon_find_eth_by_interface_index(int interface, int index);

/**
 * User-defined function called when the link state changes
 *
 * @param[in]	dev		Ethernet device
 * @param	link_state	new link state
 *
 * NOTE: This is defined as a weak function.
 */
void board_net_set_link(struct eth_device *dev, cvmx_helper_link_info_t link_state);

/**
 * Registers a function to be called when the link goes down.  The function is
 * often used for things like reading the SFP+ EEPROM.
 *
 * @param	dev		Ethernet device
 * @param	phy_port_check	Function to call
 */
void octeon_eth_register_phy_port_check(struct eth_device *dev,
					int (*phy_port_check)(struct phy_device *dev));

/**
 * This weak function is called after the phy driver is connected but before
 * it is initialized.
 *
 * @param	dev	Ethernet device for phy
 *
 * @return	0 to continue, or -1 for error to stop setting up the phy
 */
int octeon_eth_board_post_setup_phy(struct eth_device *dev);

/**
 * Registers a function to be called whenever a mod_abs change is detected.
 *
 * @param	dev		Ethernet device
 * @param	mod_abs_changed	Function to be called
 */
void octeon_eth_register_mod_abs_changed(struct eth_device *dev,
					 int (*mod_abs_changed)(struct eth_device *dev,
								bool mod_abs));

/**
 * Checks for state changes with the link state or module state
 *
 * @param	dev	Ethernet device to check
 *
 * NOTE: If the module state is changed then the module callback is called.
 */
void octeon_phy_port_check(struct eth_device *dev);

#endif /* __OCTEON_ETH_H__ */
