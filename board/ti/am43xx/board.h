/*
 * board.h
 *
 * TI AM437x boards information header
 * Derived from AM335x board.
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include <asm/arch/omap.h>

static char *const am43xx_board_name = (char *)AM4372_BOARD_NAME_START;
static char *const am43xx_board_rev = (char *)AM4372_BOARD_VERSION_START;

/*
 * TI AM437x EVMs define a system EEPROM that defines certain sub-fields.
 * We use these fields to in turn see what board we are on, and what
 * that might require us to set or not set.
 */
#define HDR_NO_OF_MAC_ADDR	3
#define HDR_ETH_ALEN		6
#define HDR_NAME_LEN		8

#define DEV_ATTR_MAX_OFFSET	5
#define DEV_ATTR_MIN_OFFSET	0

struct am43xx_board_id {
	unsigned int  magic;
	char name[HDR_NAME_LEN];
	char version[4];
	char serial[12];
	char config[32];
	char mac_addr[HDR_NO_OF_MAC_ADDR][HDR_ETH_ALEN];
};

static inline int board_is_eposevm(void)
{
	return !strncmp(am43xx_board_name, "AM43EPOS", HDR_NAME_LEN);
}

static inline int board_is_gpevm(void)
{
	return !strncmp(am43xx_board_name, "AM43__GP", HDR_NAME_LEN);
}

static inline int board_is_sk(void)
{
	return !strncmp(am43xx_board_name, "AM43__SK", HDR_NAME_LEN);
}

static inline int board_is_idk(void)
{
	return !strncmp(am43xx_board_name, "AM43_IDK", HDR_NAME_LEN);
}

static inline int board_is_evm_14_or_later(void)
{
	return (board_is_gpevm() && strncmp("1.4", am43xx_board_rev, 3) <= 0);
}

static inline int board_is_evm_12_or_later(void)
{
	return (board_is_gpevm() && strncmp("1.2", am43xx_board_rev, 3) <= 0);
}

void enable_uart0_pin_mux(void);
void enable_board_pin_mux(void);
void enable_i2c0_pin_mux(void);
#endif
