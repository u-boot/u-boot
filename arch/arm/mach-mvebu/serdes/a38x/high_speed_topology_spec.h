/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _HIGHSPEED_TOPOLOGY_SPEC_H
#define _HIGHSPEED_TOPOLOGY_SPEC_H

#include "high_speed_env_spec.h"

/* Topology map options for the DB_A38X_BP board */
enum topology_config_db {
	DB_CONFIG_SLM1363_C,
	DB_CONFIG_SLM1363_D,
	DB_CONFIG_SLM1363_E,
	DB_CONFIG_SLM1363_F,
	DB_CONFIG_SLM1364_D,
	DB_CONFIG_SLM1364_E,
	DB_CONFIG_SLM1364_F,
	DB_CONFIG_DEFAULT,
	DB_NO_TOPOLOGY
};

/*
 * this enum must be aligned with topology_config_db_381 array,
 * every update to this enum requires update to topology_config_db_381
 * array
 */
enum topology_config_db381 {
	DB_CONFIG_SLM1427,	/* enum for db_config_slm1427 */
	DB_CONFIG_SLM1426,	/* enum for db_config_slm1426 */
	DB_381_CONFIG_DEFAULT,
	DB_381_NO_TOPOLOGY
};

/* A generic function pointer for loading the board topology map */
typedef int (*load_topology_func_ptr)(struct serdes_map *serdes_map_array);

extern load_topology_func_ptr load_topology_func_arr[];

/*
 * topology_config_db_mode_get -
 *
 * DESCRIPTION:		Gets the relevant topology mode (index).
 *			for load_topology_db use only.
 * INPUT:		None.
 * OUTPUT:		None.
 * RETURNS:		the topology mode
 */
u8 topology_config_db_mode_get(void);

/*
 * load_topology_xxx -
 *
 * DESCRIPTION:		Loads the board topology for the XXX board
 * INPUT:		serdes_map_array - The struct that will contain
 *			the board topology map
 * OUTPUT:		The board topology map.
 * RETURNS:		MV_OK   for success
 *			MV_FAIL	for failure (a wrong topology mode was read
 *			from the board)
 */

/* load_topology_db - Loads the board topology for DB Board */
int load_topology_db(struct serdes_map *serdes_map_array);

/* load_topology_rd - Loads the board topology for RD Board */
int load_topology_rd(struct serdes_map *serdes_map_array);

/* load_topology_rd_nas - Loads the board topology for RD NAS Board */
int load_topology_rd_nas(struct serdes_map *serdes_map_array);

/* load_topology_rd_ap - Loads the board topology for RD Ap Board */
int load_topology_rd_ap(struct serdes_map *serdes_map_array);

/* load_topology_db_ap - Loads the board topology for DB-AP Board */
int load_topology_db_ap(struct serdes_map *serdes_map_array);

/* load_topology_db_gp - Loads the board topology for DB GP Board */
int load_topology_db_gp(struct serdes_map *serdes_map_array);

/* load_topology_db_381 - Loads the board topology for 381 DB-BP Board */
int load_topology_db_381(struct serdes_map *serdes_map_array);

/* load_topology_db_amc - Loads the board topology for DB-AMC Board */
int load_topology_db_amc(struct serdes_map *serdes_map_array);

/*
 * hws_update_device_toplogy
 * DESCRIPTION: Update the default board topology for specific device Id
 * INPUT:
 *	topology_config_ptr - pointer to the Serdes mapping
 *	topology_mode - topology mode (index)
 * OUTPUT: None
 * RRETURNS:
 *	MV_OK - if updating the board topology success
 *	MV_BAD_PARAM - if the input parameter is wrong
 */
int hws_update_device_toplogy(struct serdes_map *topology_config_ptr,
			      enum topology_config_db topology_mode);

/*
 * load_topology_rd_sgmii_usb -
 *
 * DESCRIPTION:			For RD board check if lane 4 is USB3 or SGMII
 * INPUT:			None
 * OUTPUT:			is_sgmii - return 1 if lane 4 is SGMII
 *				return 0 if lane 4 is USB.
 * RETURNS:			MV_OK for success
 */
int load_topology_rd_sgmii_usb(int *is_sgmii);

/*
 * load_topology_usb_mode_get -
 *
 * DESCRIPTION:			For DB board check if USB3.0 mode
 * INPUT:			None
 * OUTPUT:			twsi_data - return data read from S@R via I2C
 * RETURNS:			MV_OK for success
 */
int load_topology_usb_mode_get(u8 *twsi_data);

#endif /* _HIGHSPEED_TOPOLOGY_SPEC_H */
