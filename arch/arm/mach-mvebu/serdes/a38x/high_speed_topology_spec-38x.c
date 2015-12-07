/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "high_speed_topology_spec.h"
#include "sys_env_lib.h"

#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT
/*
 * This is an example implementation for this custom board
 * specific function
 */
static struct serdes_map custom_board_topology_config[] = {
	/* Customer Board Topology - reference from Marvell DB-GP board */
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA3, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA2, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

int hws_board_topology_load(struct serdes_map *serdes_map_array)
{
	serdes_map_array = custom_board_topology_config;
}
#endif

load_topology_func_ptr load_topology_func_arr[] = {
	load_topology_rd,	/* RD NAS */
	load_topology_db,	/* 6820 DB-BP (A38x) */
	load_topology_rd,	/* RD AP */
	load_topology_db_ap,	/* DB AP */
	load_topology_db_gp,	/* DB GP */
	load_topology_db_381,	/* 6821 DB-BP (A381) */
	load_topology_db_amc,	/* DB-AMC */
};

/*****************************************/
/** Load topology - Marvell 380 DB - BP **/
/*****************************************/
/* Configuration options */
struct serdes_map db_config_default[MAX_SERDES_LANES] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SATA3, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1363_c[MAX_SERDES_LANES] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX3, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SATA2, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
};

struct serdes_map db_config_slm1363_d[MAX_SERDES_LANES] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	{PEX3, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1363_e[MAX_SERDES_LANES] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{SATA2, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1363_f[MAX_SERDES_LANES] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX3, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SATA2, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1364_d[MAX_SERDES_LANES] = {
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII0, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1364_e[MAX_SERDES_LANES] = {
	{SGMII0, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0}
};

struct serdes_map db_config_slm1364_f[MAX_SERDES_LANES] = {
	{SGMII0, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0}
};

/*************************************************************************/
/** The following structs are mapping for DB board 'SatR' configuration **/
/*************************************************************************/
struct serdes_map db_satr_config_lane1[SATR_DB_LANE1_MAX_OPTIONS] = {
	/* 0 */ {DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 1 */ {PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 2 */ {SATA0, SERDES_SPEED_3_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 3 */ {SGMII0, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 4 */ {SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 5 */ {USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 6 */ {QSGMII, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_satr_config_lane2[SATR_DB_LANE2_MAX_OPTIONS] = {
	/* 0 */ {DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 1 */ {PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 2 */ {SATA1, SERDES_SPEED_3_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 3 */ {SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0}
};

/*******************************************************/
/* Configuration options DB ****************************/
/* mapping from TWSI address data to configuration map */
/*******************************************************/
struct serdes_map *topology_config_db[] = {
	db_config_slm1363_c,
	db_config_slm1363_d,
	db_config_slm1363_e,
	db_config_slm1363_f,
	db_config_slm1364_d,
	db_config_slm1364_e,
	db_config_slm1364_f,
	db_config_default
};

/*************************************/
/** Load topology - Marvell DB - AP **/
/*************************************/
struct serdes_map db_ap_config_default[MAX_SERDES_LANES] = {
	/* 0 */ {PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 1 */ {SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 2 */ {PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 3 */ {SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 4 */ {USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 5 */ {PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0}
};

/*************************************/
/** Load topology - Marvell DB - GP **/
/*************************************/
struct serdes_map db_gp_config_default[MAX_SERDES_LANES] = {
	/* 0 */ {PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	/* 1 */ {SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	/* 2 */ {SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	/* 3 */ {SATA3, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	/* 4 */ {SATA2, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	/* 5 */ {USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0,
		 0}
};

struct serdes_map db_amc_config_default[MAX_SERDES_LANES] = {
	/* 0 */ {PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	/* 1 */ {PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	/* 2 */ {PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	/* 3 */ {PEX3, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X4, 0, 0},
	/* 4 */ {SGMII1, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
	/* 5 */ {SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 0,
		 0},
};

/*****************************************/
/** Load topology - Marvell 381 DB - BP **/
/*****************************************/
/* Configuration options */
struct serdes_map db381_config_default[MAX_SERDES_LANES] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0}
};

struct serdes_map db_config_slm1427[MAX_SERDES_LANES] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 1, 1},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 1, 1}
};

struct serdes_map db_config_slm1426[MAX_SERDES_LANES] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 1, 1},
	{SGMII2, SERDES_SPEED_3_125_GBPS, SERDES_DEFAULT_MODE, 1, 1}
};

/*
 * this array must be aligned with enum topology_config_db381 enum,
 * every update to this array requires update to enum topology_config_db381
 * enum
 */
struct serdes_map *topology_config_db_381[] = {
	db_config_slm1427,
	db_config_slm1426,
	db381_config_default,
};

u8 topology_config_db_mode_get(void)
{
	u8 mode;

	DEBUG_INIT_FULL_S("\n### topology_config_db_mode_get ###\n");

	/* Default - return DB_CONFIG_DEFAULT */

	if (!i2c_read(DB_GET_MODE_SLM1363_ADDR, 0, 1, &mode, 1)) {
		switch (mode & 0xf) {
		case 0xc:
			DEBUG_INIT_S("\nInit DB board SLM 1363 C topology\n");
			return DB_CONFIG_SLM1363_C;
		case 0xd:
			DEBUG_INIT_S("\nInit DB board SLM 1363 D topology\n");
			return DB_CONFIG_SLM1363_D;
		case 0xe:
			DEBUG_INIT_S("\nInit DB board SLM 1363 E topology\n");
			return DB_CONFIG_SLM1363_E;
		case 0xf:
			DEBUG_INIT_S("\nInit DB board SLM 1363 F topology\n");
			return DB_CONFIG_SLM1363_F;
		default:	/* not the right module */
			break;
		}
	}

	/* SLM1364 Module */
	if (i2c_read(DB_GET_MODE_SLM1364_ADDR, 0, 1, &mode, 1)) {
		DEBUG_INIT_S("\nInit DB board default topology\n");
		return DB_CONFIG_DEFAULT;
	}

	switch (mode & 0xf) {
	case 0xd:
		DEBUG_INIT_S("\nInit DB board SLM 1364 D topology\n");
		return DB_CONFIG_SLM1364_D;
	case 0xe:
		DEBUG_INIT_S("\nInit DB board SLM 1364 E topology\n");
		return DB_CONFIG_SLM1364_E;
	case 0xf:
		DEBUG_INIT_S("\nInit DB board SLM 1364 F topology\n");
		return DB_CONFIG_SLM1364_F;
	default:		/* Default configuration */
		DEBUG_INIT_S("\nInit DB board default topology\n");
		return DB_CONFIG_DEFAULT;
	}
}

u8 topology_config_db_381_mode_get(void)
{
	u8 mode;

	DEBUG_INIT_FULL_S("\n### topology_config_db_381_mode_get ###\n");

	if (!i2c_read(DB381_GET_MODE_SLM1426_1427_ADDR, 0, 2, &mode, 1)) {
		switch (mode & 0xf) {
		case 0x1:
			DEBUG_INIT_S("\nInit DB-381 board SLM 1427 topology\n");
			return DB_CONFIG_SLM1427;
		case 0x2:
			DEBUG_INIT_S("\nInit DB-381 board SLM 1426 topology\n");
			return DB_CONFIG_SLM1426;
		default:	/* not the right module */
			break;
		}
	}

	/* in case not detected any supported module, use default topology */
	DEBUG_INIT_S("\nInit DB-381 board default topology\n");
	return DB_381_CONFIG_DEFAULT;
}

/*
 * Read SatR field 'sgmiispeed' and update lane topology SGMII entries
 * speed setup
 */
int update_topology_sgmii_speed(struct serdes_map *serdes_map_array)
{
	u32 serdes_type, lane_num;
	u8 config_val;

	/* Update SGMII speed settings by 'sgmiispeed' SatR value */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_type = serdes_map_array[lane_num].serdes_type;
		/*Read SatR configuration for SGMII speed */
		if ((serdes_type == SGMII0) || (serdes_type == SGMII1) ||
		    (serdes_type == SGMII2)) {
			/* Read SatR 'sgmiispeed' value */
			if (i2c_read(EEPROM_I2C_ADDR, 0, 2, &config_val, 1)) {
				printf("%s: TWSI Read of 'sgmiispeed' failed\n",
				       __func__);
				return MV_FAIL;
			}

			if (0 == (config_val & 0x40)) {
				serdes_map_array[lane_num].serdes_speed =
					SERDES_SPEED_1_25_GBPS;
			} else {
				serdes_map_array[lane_num].serdes_speed =
					SERDES_SPEED_3_125_GBPS;
			}
		}
	}
	return MV_OK;
}

struct serdes_map default_lane = {
	DEFAULT_SERDES, LAST_SERDES_SPEED, SERDES_DEFAULT_MODE
};
int is_custom_topology = 0;	/* indicate user of non-default topology */

/*
 * Read SatR fields (dbserdes1/2 , gpserdes1/2/5) and update lane
 * topology accordingly
 */
int update_topology_satr(struct serdes_map *serdes_map_array)
{
	u8 config_val, lane_select, i;
	u32 board_id = mv_board_id_get();

	switch (board_id) {
	case DB_68XX_ID:	/* read 'dbserdes1' & 'dbserdes2' */
	case DB_BP_6821_ID:
		if (i2c_read(EEPROM_I2C_ADDR, 1, 2, &config_val, 1)) {
			printf("%s: TWSI Read of 'dbserdes1/2' failed\n",
			       __func__);
			return MV_FAIL;
		}

		/* Lane #1 */
		lane_select = (config_val & SATR_DB_LANE1_CFG_MASK) >>
			SATR_DB_LANE1_CFG_OFFSET;
		if (lane_select >= SATR_DB_LANE1_MAX_OPTIONS) {
			printf("\n\%s: Error: invalid value for SatR field 'dbserdes1' (%x)\n",
			       __func__, lane_select);
			printf("\t_skipping Topology update (run 'SatR write default')\n");
			return MV_FAIL;
		}

		/*
		 * If modified default serdes_type for lane#1, update
		 * topology and mark it as custom
		 */
		if (serdes_map_array[1].serdes_type !=
		    db_satr_config_lane1[lane_select].serdes_type) {
			serdes_map_array[1] = db_satr_config_lane1[lane_select];
			is_custom_topology = 1;
			/* DB 381/2 board has inverted SerDes polarity */
			if (board_id == DB_BP_6821_ID)
				serdes_map_array[1].swap_rx =
					serdes_map_array[1].swap_tx = 1;
		}

		/* Lane #2 */
		lane_select = (config_val & SATR_DB_LANE2_CFG_MASK) >>
			SATR_DB_LANE2_CFG_OFFSET;
		if (lane_select >= SATR_DB_LANE2_MAX_OPTIONS) {
			printf("\n\%s: Error: invalid value for SatR field 'dbserdes2' (%x)\n",
			       __func__, lane_select);
			printf("\t_skipping Topology update (run 'SatR write default')\n");
			return MV_FAIL;
		}

		/*
		 * If modified default serdes_type for lane@2, update
		 * topology and mark it as custom
		 */
		if (serdes_map_array[2].serdes_type !=
		    db_satr_config_lane2[lane_select].serdes_type) {
			serdes_map_array[2] = db_satr_config_lane2[lane_select];
			is_custom_topology = 1;
			/* DB 381/2 board has inverted SerDes polarity */
			if (board_id == DB_BP_6821_ID)
				serdes_map_array[2].swap_rx =
					serdes_map_array[2].swap_tx = 1;
		}

		if (is_custom_topology == 1) {
			/*
			 * Check for conflicts with detected lane #1 and
			 * lane #2 (Disable conflicted lanes)
			 */
			for (i = 0; i < hws_serdes_get_max_lane(); i++) {
				if (i != 1 && serdes_map_array[1].serdes_type ==
				    serdes_map_array[i].serdes_type) {
					printf("\t_lane #%d Type conflicts with Lane #1 (Lane #%d disabled)\n",
					       i, i);
					serdes_map_array[i] =
						db_satr_config_lane1[0];
				}

				if (i != 2 &&
				    serdes_map_array[2].serdes_type ==
				    serdes_map_array[i].serdes_type) {
					printf("\t_lane #%d Type conflicts with Lane #2 (Lane #%d disabled)\n",
					       i, i);
					serdes_map_array[i] =
						db_satr_config_lane1[0];
				}
			}
		}

		break;		/* case DB_68XX_ID */
	case DB_GP_68XX_ID:	/* read 'gpserdes1' & 'gpserdes2' */
		if (i2c_read(EEPROM_I2C_ADDR, 2, 2, &config_val, 1)) {
			printf("%s: TWSI Read of 'gpserdes1/2' failed\n",
			       __func__);
			return MV_FAIL;
		}

		/*
		 * Lane #1:
		 * lane_select = 0 --> SATA0,
		 * lane_select = 1 --> PCIe0 (mini PCIe)
		 */
		lane_select = (config_val & SATR_GP_LANE1_CFG_MASK) >>
			SATR_GP_LANE1_CFG_OFFSET;
		if (lane_select == 1) {
			serdes_map_array[1].serdes_mode = PEX0;
			serdes_map_array[1].serdes_speed = SERDES_SPEED_5_GBPS;
			serdes_map_array[1].serdes_type = PEX_ROOT_COMPLEX_X1;
			/*
			 * If lane 1 is set to PCIe0 --> disable PCIe0
			 * on lane 0
			 */
			serdes_map_array[0] = default_lane;
			/* indicate user of non-default topology */
			is_custom_topology = 1;
		}
		printf("Lane 1 detection: %s\n",
		       lane_select ? "PCIe0 (mini PCIe)" : "SATA0");

		/*
		 * Lane #2:
		 * lane_select = 0 --> SATA1,
		 * lane_select = 1 --> PCIe1 (mini PCIe)
		 */
		lane_select = (config_val & SATR_GP_LANE2_CFG_MASK) >>
			SATR_GP_LANE2_CFG_OFFSET;
		if (lane_select == 1) {
			serdes_map_array[2].serdes_type = PEX1;
			serdes_map_array[2].serdes_speed = SERDES_SPEED_5_GBPS;
			serdes_map_array[2].serdes_mode = PEX_ROOT_COMPLEX_X1;
			/* indicate user of non-default topology */
			is_custom_topology = 1;
		}
		printf("Lane 2 detection: %s\n",
		       lane_select ? "PCIe1 (mini PCIe)" : "SATA1");
		break;		/* case DB_GP_68XX_ID */
	}

	if (is_custom_topology)
		printf("\nDetected custom SerDes topology (to restore default run 'SatR write default')\n\n");

	return MV_OK;
}

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
			      enum topology_config_db topology_mode)
{
	u32 dev_id = sys_env_device_id_get();
	u32 board_id = mv_board_id_get();

	switch (topology_mode) {
	case DB_CONFIG_DEFAULT:
		switch (dev_id) {
		case MV_6810:
			/*
			 * DB-AP : default for Lane3=SGMII2 -->
			 * 6810 supports only 2 SGMII interfaces:
			 * lane 3 disabled
			 */
			if (board_id == DB_AP_68XX_ID) {
				printf("Device 6810 supports only 2 SGMII interfaces: SGMII-2 @ lane3 disabled\n");
				topology_config_ptr[3] = default_lane;
			}

			/*
			 * 6810 has only 4 SerDes and the forth one is
			 * Serdes number 5 (i.e. Serdes 4 is not connected),
			 * therefore we need to copy SerDes 5 configuration
			 * to SerDes 4
			 */
			printf("Device 6810 does not supports SerDes Lane #4: replaced topology entry with lane #5\n");
			topology_config_ptr[4] = topology_config_ptr[5];

			/*
			 * No break between cases since the 1st
			 * 6820 limitation apply on 6810
			 */
		case MV_6820:
			/*
			 * DB-GP & DB-BP: default for Lane3=SATA3 -->
			 * 6810/20 supports only 2 SATA interfaces:
			 * lane 3 disabled
			 */
			if ((board_id == DB_68XX_ID) ||
			    (board_id == DB_GP_68XX_ID)) {
				printf("Device 6810/20 supports only 2 SATA interfaces: SATA Port 3 @ lane3 disabled\n");
				topology_config_ptr[3] = default_lane;
			}
			/*
			 * DB-GP on 6820 only: default for Lane4=SATA2
			 * --> 6820 supports only 2 SATA interfaces:
			 * lane 3 disabled
			 */
			if (board_id == DB_GP_68XX_ID && dev_id == MV_6820) {
				printf("Device 6820 supports only 2 SATA interfaces: SATA Port 2 @ lane4 disabled\n");
				topology_config_ptr[4] = default_lane;
			}
			break;
		default:
			break;
		}
		break;

	default:
		printf("sys_env_update_device_toplogy: selected topology is not supported by this routine\n");
		break;
	}

	return MV_OK;
}

int load_topology_db_381(struct serdes_map *serdes_map_array)
{
	u32 lane_num;
	u8 topology_mode;
	struct serdes_map *topology_config_ptr;
	u8 twsi_data;
	u8 usb3_host0_or_device = 0, usb3_host1_or_device = 0;

	printf("\nInitialize DB-88F6821-BP board topology\n");

	/* Getting the relevant topology mode (index) */
	topology_mode = topology_config_db_381_mode_get();
	topology_config_ptr = topology_config_db_381[topology_mode];

	/* Read USB3.0 mode: HOST/DEVICE */
	if (load_topology_usb_mode_get(&twsi_data) == MV_OK) {
		usb3_host0_or_device = (twsi_data & 0x1);
		/* Only one USB3 device is enabled */
		if (usb3_host0_or_device == 0)
			usb3_host1_or_device = ((twsi_data >> 1) & 0x1);
	}

	/* Updating the topology map */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_map_array[lane_num].serdes_mode =
			topology_config_ptr[lane_num].serdes_mode;
		serdes_map_array[lane_num].serdes_speed =
			topology_config_ptr[lane_num].serdes_speed;
		serdes_map_array[lane_num].serdes_type =
			topology_config_ptr[lane_num].serdes_type;
		serdes_map_array[lane_num].swap_rx =
			topology_config_ptr[lane_num].swap_rx;
		serdes_map_array[lane_num].swap_tx =
			topology_config_ptr[lane_num].swap_tx;

		/* Update USB3 device if needed */
		if (usb3_host0_or_device == 1 &&
		    serdes_map_array[lane_num].serdes_type == USB3_HOST0)
			serdes_map_array[lane_num].serdes_type = USB3_DEVICE;

		if (usb3_host1_or_device == 1 &&
		    serdes_map_array[lane_num].serdes_type == USB3_HOST1)
			serdes_map_array[lane_num].serdes_type = USB3_DEVICE;
	}

	/* If not detected any SerDes Site module, read 'SatR' lane setup */
	if (topology_mode == DB_381_CONFIG_DEFAULT)
		update_topology_satr(serdes_map_array);

	/* update 'sgmiispeed' settings */
	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_db(struct serdes_map *serdes_map_array)
{
	u32 lane_num;
	u8 topology_mode;
	struct serdes_map *topology_config_ptr;
	u8 twsi_data;
	u8 usb3_host0_or_device = 0, usb3_host1_or_device = 0;

	printf("\nInitialize DB-88F6820-BP board topology\n");

	/* Getting the relevant topology mode (index) */
	topology_mode = topology_config_db_mode_get();

	if (topology_mode == DB_NO_TOPOLOGY)
		topology_mode = DB_CONFIG_DEFAULT;

	topology_config_ptr = topology_config_db[topology_mode];

	/* Update the default board topology device flavours */
	CHECK_STATUS(hws_update_device_toplogy
		     (topology_config_ptr, topology_mode));

	/* Read USB3.0 mode: HOST/DEVICE */
	if (load_topology_usb_mode_get(&twsi_data) == MV_OK) {
		usb3_host0_or_device = (twsi_data & 0x1);
		/* Only one USB3 device is enabled */
		if (usb3_host0_or_device == 0)
			usb3_host1_or_device = ((twsi_data >> 1) & 0x1);
	}

	/* Updating the topology map */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_map_array[lane_num].serdes_mode =
			topology_config_ptr[lane_num].serdes_mode;
		serdes_map_array[lane_num].serdes_speed =
			topology_config_ptr[lane_num].serdes_speed;
		serdes_map_array[lane_num].serdes_type =
			topology_config_ptr[lane_num].serdes_type;
		serdes_map_array[lane_num].swap_rx =
			topology_config_ptr[lane_num].swap_rx;
		serdes_map_array[lane_num].swap_tx =
			topology_config_ptr[lane_num].swap_tx;

		/*
		 * Update USB3 device if needed - relevant for
		 * lane 3,4,5 only
		 */
		if (lane_num >= 3) {
			if ((serdes_map_array[lane_num].serdes_type ==
			     USB3_HOST0) && (usb3_host0_or_device == 1))
				serdes_map_array[lane_num].serdes_type =
					USB3_DEVICE;

			if ((serdes_map_array[lane_num].serdes_type ==
			     USB3_HOST1) && (usb3_host1_or_device == 1))
				serdes_map_array[lane_num].serdes_type =
					USB3_DEVICE;
		}
	}

	/* If not detected any SerDes Site module, read 'SatR' lane setup */
	if (topology_mode == DB_CONFIG_DEFAULT)
		update_topology_satr(serdes_map_array);

	/* update 'sgmiispeed' settings */
	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_db_ap(struct serdes_map *serdes_map_array)
{
	u32 lane_num;
	struct serdes_map *topology_config_ptr;

	DEBUG_INIT_FULL_S("\n### load_topology_db_ap ###\n");

	printf("\nInitialize DB-AP board topology\n");
	topology_config_ptr = db_ap_config_default;

	/* Update the default board topology device flavours */
	CHECK_STATUS(hws_update_device_toplogy
		     (topology_config_ptr, DB_CONFIG_DEFAULT));

	/* Updating the topology map */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_map_array[lane_num].serdes_mode =
			topology_config_ptr[lane_num].serdes_mode;
		serdes_map_array[lane_num].serdes_speed =
			topology_config_ptr[lane_num].serdes_speed;
		serdes_map_array[lane_num].serdes_type =
			topology_config_ptr[lane_num].serdes_type;
		serdes_map_array[lane_num].swap_rx =
			topology_config_ptr[lane_num].swap_rx;
		serdes_map_array[lane_num].swap_tx =
			topology_config_ptr[lane_num].swap_tx;
	}

	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_db_gp(struct serdes_map *serdes_map_array)
{
	u32 lane_num;
	struct serdes_map *topology_config_ptr;
	int is_sgmii = 0;

	DEBUG_INIT_FULL_S("\n### load_topology_db_gp ###\n");

	topology_config_ptr = db_gp_config_default;

	printf("\nInitialize DB-GP board topology\n");

	/* check S@R: if lane 5 is USB3 or SGMII */
	if (load_topology_rd_sgmii_usb(&is_sgmii) != MV_OK)
		printf("%s: TWSI Read failed - Loading Default Topology\n",
		       __func__);
	else {
		topology_config_ptr[5].serdes_type =
			is_sgmii ? SGMII2 : USB3_HOST1;
		topology_config_ptr[5].serdes_speed = is_sgmii ?
			SERDES_SPEED_3_125_GBPS : SERDES_SPEED_5_GBPS;
		topology_config_ptr[5].serdes_mode = SERDES_DEFAULT_MODE;
	}

	/* Update the default board topology device flavours */
	CHECK_STATUS(hws_update_device_toplogy
		     (topology_config_ptr, DB_CONFIG_DEFAULT));

	/* Updating the topology map */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_map_array[lane_num].serdes_mode =
			topology_config_ptr[lane_num].serdes_mode;
		serdes_map_array[lane_num].serdes_speed =
			topology_config_ptr[lane_num].serdes_speed;
		serdes_map_array[lane_num].serdes_type =
			topology_config_ptr[lane_num].serdes_type;
		serdes_map_array[lane_num].swap_rx =
			topology_config_ptr[lane_num].swap_rx;
		serdes_map_array[lane_num].swap_tx =
			topology_config_ptr[lane_num].swap_tx;
	}

	/*
	 * Update 'gpserdes1/2/3' lane configuration , and 'sgmiispeed'
	 * for SGMII lanes
	 */
	update_topology_satr(serdes_map_array);
	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_db_amc(struct serdes_map *serdes_map_array)
{
	u32 lane_num;
	struct serdes_map *topology_config_ptr;

	DEBUG_INIT_FULL_S("\n### load_topology_db_amc ###\n");

	printf("\nInitialize DB-AMC board topology\n");
	topology_config_ptr = db_amc_config_default;

	/* Update the default board topology device flavours */
	CHECK_STATUS(hws_update_device_toplogy
		     (topology_config_ptr, DB_CONFIG_DEFAULT));

	/* Updating the topology map */
	for (lane_num = 0; lane_num < hws_serdes_get_max_lane(); lane_num++) {
		serdes_map_array[lane_num].serdes_mode =
			topology_config_ptr[lane_num].serdes_mode;
		serdes_map_array[lane_num].serdes_speed =
			topology_config_ptr[lane_num].serdes_speed;
		serdes_map_array[lane_num].serdes_type =
			topology_config_ptr[lane_num].serdes_type;
		serdes_map_array[lane_num].swap_rx =
			topology_config_ptr[lane_num].swap_rx;
		serdes_map_array[lane_num].swap_tx =
			topology_config_ptr[lane_num].swap_tx;
	}

	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_rd(struct serdes_map *serdes_map_array)
{
	u8 mode;

	DEBUG_INIT_FULL_S("\n### load_topology_rd ###\n");

	DEBUG_INIT_S("\nInit RD board ");

	/* Reading mode */
	DEBUG_INIT_FULL_S("load_topology_rd: getting mode\n");
	if (i2c_read(EEPROM_I2C_ADDR, 0, 2, &mode, 1)) {
		DEBUG_INIT_S("load_topology_rd: TWSI Read failed\n");
		return MV_FAIL;
	}

	/* Updating the topology map */
	DEBUG_INIT_FULL_S("load_topology_rd: Loading board topology details\n");

	/* RD mode: 0 = NAS, 1 = AP */
	if (((mode >> 1) & 0x1) == 0) {
		CHECK_STATUS(load_topology_rd_nas(serdes_map_array));
	} else {
		CHECK_STATUS(load_topology_rd_ap(serdes_map_array));
	}

	update_topology_sgmii_speed(serdes_map_array);

	return MV_OK;
}

int load_topology_rd_nas(struct serdes_map *serdes_map_array)
{
	int is_sgmii = 0;
	u32 i;

	DEBUG_INIT_S("\nInit RD NAS topology ");

	/* check if lane 4 is USB3 or SGMII */
	if (load_topology_rd_sgmii_usb(&is_sgmii) != MV_OK) {
		DEBUG_INIT_S("load_topology_rd NAS: TWSI Read failed\n");
		return MV_FAIL;
	}

	/* Lane 0 */
	serdes_map_array[0].serdes_type = PEX0;
	serdes_map_array[0].serdes_speed = SERDES_SPEED_5_GBPS;
	serdes_map_array[0].serdes_mode = PEX_ROOT_COMPLEX_X1;

	/* Lane 1 */
	serdes_map_array[1].serdes_type = SATA0;
	serdes_map_array[1].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[1].serdes_mode = SERDES_DEFAULT_MODE;

	/* Lane 2 */
	serdes_map_array[2].serdes_type = SATA1;
	serdes_map_array[2].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[2].serdes_mode = SERDES_DEFAULT_MODE;

	/* Lane 3 */
	serdes_map_array[3].serdes_type = SATA3;
	serdes_map_array[3].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[3].serdes_mode = SERDES_DEFAULT_MODE;

	/* Lane 4 */
	if (is_sgmii == 1) {
		DEBUG_INIT_S("Serdes Lane 4 is SGMII\n");
		serdes_map_array[4].serdes_type = SGMII1;
		serdes_map_array[4].serdes_speed = SERDES_SPEED_3_125_GBPS;
		serdes_map_array[4].serdes_mode = SERDES_DEFAULT_MODE;
	} else {
		DEBUG_INIT_S("Serdes Lane 4 is USB3\n");
		serdes_map_array[4].serdes_type = USB3_HOST0;
		serdes_map_array[4].serdes_speed = SERDES_SPEED_5_GBPS;
		serdes_map_array[4].serdes_mode = SERDES_DEFAULT_MODE;
	}

	/* Lane 5 */
	serdes_map_array[5].serdes_type = SATA2;
	serdes_map_array[5].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[5].serdes_mode = SERDES_DEFAULT_MODE;

	/* init swap configuration */
	for (i = 0; i <= 5; i++) {
		serdes_map_array[i].swap_rx = 0;
		serdes_map_array[i].swap_tx = 0;
	}

	return MV_OK;
}

int load_topology_rd_ap(struct serdes_map *serdes_map_array)
{
	int is_sgmii = 0;
	u32 i;

	DEBUG_INIT_S("\nInit RD AP topology ");

	/* check if lane 4 is USB3 or SGMII */
	if (load_topology_rd_sgmii_usb(&is_sgmii) != MV_OK) {
		DEBUG_INIT_S("load_topology_rd AP: TWSI Read failed\n");
		return MV_FAIL;
	}

	/* Lane 0 */
	serdes_map_array[0].serdes_type = DEFAULT_SERDES;
	serdes_map_array[0].serdes_speed = LAST_SERDES_SPEED;
	serdes_map_array[0].serdes_mode = SERDES_DEFAULT_MODE;

	/* Lane 1 */
	serdes_map_array[1].serdes_type = PEX0;
	serdes_map_array[1].serdes_speed = SERDES_SPEED_5_GBPS;
	serdes_map_array[1].serdes_mode = PEX_ROOT_COMPLEX_X1;

	/* Lane 2 */
	serdes_map_array[2].serdes_type = PEX1;
	serdes_map_array[2].serdes_speed = SERDES_SPEED_5_GBPS;
	serdes_map_array[2].serdes_mode = PEX_ROOT_COMPLEX_X1;

	/* Lane 3 */
	serdes_map_array[3].serdes_type = SATA3;
	serdes_map_array[3].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[3].serdes_mode = SERDES_DEFAULT_MODE;

	/* Lane 4 */
	if (is_sgmii == 1) {
		DEBUG_INIT_S("Serdes Lane 4 is SGMII\n");
		serdes_map_array[4].serdes_type = SGMII1;
		serdes_map_array[4].serdes_speed = SERDES_SPEED_3_125_GBPS;
		serdes_map_array[4].serdes_mode = SERDES_DEFAULT_MODE;
	} else {
		DEBUG_INIT_S("Serdes Lane 4 is USB3\n");
		serdes_map_array[4].serdes_type = USB3_HOST0;
		serdes_map_array[4].serdes_speed = SERDES_SPEED_5_GBPS;
		serdes_map_array[4].serdes_mode = SERDES_DEFAULT_MODE;
	}

	/* Lane 5 */
	serdes_map_array[5].serdes_type = SATA2;
	serdes_map_array[5].serdes_speed = SERDES_SPEED_3_GBPS;
	serdes_map_array[5].serdes_mode = SERDES_DEFAULT_MODE;

	/* init swap configuration */
	for (i = 0; i <= 5; i++) {
		serdes_map_array[i].swap_rx = 0;
		serdes_map_array[i].swap_tx = 0;
	}

	return MV_OK;
}

int load_topology_rd_sgmii_usb(int *is_sgmii)
{
	u8 mode;

	/*
	 * DB-GP board: Device 6810 supports only 2 GbE ports:
	 * SGMII2 not supported (USE USB3 Host instead)
	 */
	if (sys_env_device_id_get() == MV_6810) {
		printf("Device 6810 supports only 2 GbE ports: SGMII-2 @ lane5 disabled (setting USB3.0 H1 instead)\n");
		*is_sgmii = 0;
		return MV_OK;
	}

	if (!i2c_read(RD_GET_MODE_ADDR, 1, 2, &mode, 1)) {
		*is_sgmii = ((mode >> 2) & 0x1);
	} else {
		/* else use the default - USB3 */
		*is_sgmii = 0;
	}

	if (*is_sgmii)
		is_custom_topology = 1;

	printf("Lane 5 detection: %s\n",
	       *is_sgmii ? "SGMII2" : "USB3.0 Host Port 1");

	return MV_OK;
}

/*
 * 'usb3port0'/'usb3port1' fields are located in EEPROM,
 * at 3rd byte(offset=2), bit 0:1 (respectively)
 */
int load_topology_usb_mode_get(u8 *twsi_data)
{
	if (!i2c_read(EEPROM_I2C_ADDR, 2, 2, twsi_data, 1))
		return MV_OK;

	return MV_ERROR;
}
