/*
 * board.h
 *
 * TI AM335x boards information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * TI AM335x parts define a system EEPROM that defines certain sub-fields.
 * We use these fields to in turn see what board we are on, and what
 * that might require us to set or not set.
 */
#define HDR_NO_OF_MAC_ADDR	3
#define HDR_ETH_ALEN		6
#define HDR_NAME_LEN		8

struct am335x_baseboard_id {
	unsigned int  magic;
	char name[HDR_NAME_LEN];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[HDR_NO_OF_MAC_ADDR][HDR_ETH_ALEN];
};

typedef struct _BSP_VS_HWPARAM    // v1.0
{
	uint32_t Magic;
	uint32_t HwRev;
	uint32_t SerialNumber;
	char PrdDate[11];    // as a string ie. "01.01.2006"
	uint16_t SystemId;
	uint8_t MAC1[6];        // internal EMAC
	uint8_t MAC2[6];        // SMSC9514
	uint8_t MAC3[6];        // WL1271 WLAN
} __attribute__ ((packed)) BSP_VS_HWPARAM;

static inline int board_is_bone(struct am335x_baseboard_id *header)
{
	return !strncmp(header->name, "A335BONE", HDR_NAME_LEN);
}

static inline int board_is_bone_lt(struct am335x_baseboard_id *header)
{
	return !strncmp(header->name, "A335BNLT", HDR_NAME_LEN);
}

static inline int board_is_evm_sk(struct am335x_baseboard_id *header)
{
	return !strncmp("A335X_SK", header->name, HDR_NAME_LEN);
}

static inline int board_is_idk(struct am335x_baseboard_id *header)
{
	return !strncmp(header->config, "SKU#02", 6);
}

static inline int board_is_gp_evm(struct am335x_baseboard_id *header)
{
	return !strncmp("A33515BB", header->name, HDR_NAME_LEN);
}

static inline int board_is_evm_15_or_later(struct am335x_baseboard_id *header)
{
	return (board_is_gp_evm(header) &&
		strncmp("1.5", header->version, 3) <= 0);
}

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_uart1_pin_mux(void);
void enable_uart2_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_uart5_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_i2c1_pin_mux(void);
void enable_board_pin_mux(void);
#endif
