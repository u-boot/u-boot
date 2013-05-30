/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>
#include <asm/io.h>
#include <i2c.h>
#include "soft_switch.h"

struct switch_config {
	uchar dir0; /* IODIRA */
	uchar dir1; /* IODIRB */
	uchar value0; /* OLATA */
	uchar value1; /* OLATB */
};

static struct switch_config switch_config_array[NUM_SWITCH] = {
	{
/*
	U45 Port A                     U45 Port B

	7---------------  RMII_CLK_EN  |  7--------------- ~TEMP_THERM_EN
	| 6------------- ~CNT0ZM_EN    |  | 6------------- ~TEMP_IRQ_EN
	| | 5----------- ~CNT0DG_EN    |  | | 5----------- ~UART0CTS_146_EN
	| | | 4--------- ~CNT0UD_EN    |  | | | 4--------- ~UART0CTS_RST_EN
	| | | | 3------- ~CAN0RX_EN    |  | | | | 3------- ~UART0CTS_RTS_LPBK
	| | | | | 2----- ~CAN0_ERR_EN  |  | | | | | 2----- ~UART0CTS_EN
	| | | | | | 1--- ~CAN_STB      |  | | | | | | 1--- ~UART0RX_EN
	| | | | | | | 0-  CAN_EN       |  | | | | | | | 0- ~UART0RTS_EN
	| | | | | | | |                |  | | | | | | | |
	O O O O O O O O                |  O O O O O O O O   (I/O direction)
	1 0 0 0 0 0 1 1                |  1 1 1 1 1 0 0 0   (value being set)
*/
		.dir0 = 0x0, /* all output */
		.dir1 = 0x0, /* all output */
		.value0 = RMII_CLK_EN | CAN_STB | CAN_EN,
		.value1 = TEMP_THERM_EN | TEMP_IRQ_EN | UART0CTS_146_EN
				| UART0CTS_RST_EN | UART0CTS_RTS_LPBK,
	},
	{
/*
	U46 Port A                       U46 Port B

	7--------------- ~LED4_GPIO_EN   |  7---------------  EMPTY
	| 6------------- ~LED3_GPIO_EN   |  | 6------------- ~SPI0D3_EN
	| | 5----------- ~LED2_GPIO_EN   |  | | 5----------- ~SPI0D2_EN
	| | | 4--------- ~LED1_GPIO_EN   |  | | | 4--------- ~SPIFLASH_CS_EN
	| | | | 3-------  SMC0_LP0_EN    |  | | | | 3------- ~SD_WP_EN
	| | | | | 2-----  EMPTY          |  | | | | | 2----- ~SD_CD_EN
	| | | | | | 1---  SMC0_EPPI2     |  | | | | | | 1--- ~PUSHBUTTON2_EN
			  _LP1_SWITCH
	| | | | | | | 0-  OVERRIDE_SMC0  |  | | | | | | | 0- ~PUSHBUTTON1_EN
			  _LP0_BOOT
	| | | | | | | |                  |  | | | | | | | |
	O O O O O O O O                  |  O O O O O O O O   (I/O direction)
	0 0 0 0 0 X 0 1                  |  X 0 0 0 0 0 0 0   (value being set)
*/
		.dir0 = 0x0, /* all output */
		.dir1 = 0x0, /* all output */
#ifdef CONFIG_BFIN_LINKPORT
		.value0 = OVERRIDE_SMC0_LP0_BOOT,
#else
		.value0 = SMC0_EPPI2_LP1_SWITCH,
#endif
		.value1 = 0x0,
	},
	{
/*
	U47 Port A                         U47 Port B

	7--------------- ~PD2_SPI0MISO |  7---------------  EMPTY
			  _EI3_EN
	| 6------------- ~PD1_SPI0D3   |  | 6-------------  EMPTY
			  _EPPI1D17
			  _SPI0SEL2
			  _EI3_EN
	| | 5----------- ~PD0_SPI0D2   |  | | 5-----------  EMPTY
			  _EPPI1D16
			  _SPI0SEL3
			  _EI3_EN
	| | | 4--------- ~WAKE_PUSH    |  | | | 4---------  EMPTY
			  BUTTON_EN
	| | | | 3------- ~ETHERNET_EN  |  | | | | 3-------  EMPTY
	| | | | | 2-----  PHYAD0       |  | | | | | 2-----  EMPTY
	| | | | | | 1---  PHY_PWR      |  | | | | | | 1--- ~PD4_SPI0CK_EI3_EN
			  _DWN_INT
	| | | | | | | 0- ~PHYINT_EN    |  | | | | | | | 0- ~PD3_SPI0MOSI_EI3_EN
	| | | | | | | |                |  | | | | | | | |
	O O O O O I I O                |  O O O O O O O O   (I/O direction)
	1 1 1 0 0 0 0 0                |  X X X X X X 1 1   (value being set)
*/
		.dir0 = 0x6, /* bits 1 and 2 input, all others output */
		.dir1 = 0x0, /* all output */
		.value0 = PD1_SPI0D3_EN | PD0_SPI0D2_EN,
		.value1 = 0,
	},
};

static int setup_soft_switch(int addr, struct switch_config *config)
{
	int ret = 0;

	ret = i2c_write(addr, OLATA, 1, &config->value0, 1);
	if (ret)
		return ret;
	ret = i2c_write(addr, OLATB, 1, &config->value1, 1);
	if (ret)
		return ret;

	ret = i2c_write(addr, IODIRA, 1, &config->dir0, 1);
	if (ret)
		return ret;
	return i2c_write(addr, IODIRB, 1, &config->dir1, 1);
}

int config_switch_bit(int addr, int port, int bit, int dir, uchar value)
{
	int ret, data_reg, dir_reg;
	uchar tmp;

	if (port == IO_PORT_A) {
		data_reg = OLATA;
		dir_reg = IODIRA;
	} else {
		data_reg = OLATB;
		dir_reg = IODIRB;
	}

	if (dir == IO_PORT_INPUT) {
		ret = i2c_read(addr, dir_reg, 1, &tmp, 1);
		if (ret)
			return ret;
		tmp |= bit;
		return i2c_write(addr, dir_reg, 1, &tmp, 1);
	} else {
		ret = i2c_read(addr, data_reg, 1, &tmp, 1);
		if (ret)
			return ret;
		if (value)
			tmp |= bit;
		else
			tmp &= ~bit;
		ret = i2c_write(addr, data_reg, 1, &tmp, 1);
		if (ret)
			return ret;
		ret = i2c_read(addr, dir_reg, 1, &tmp, 1);
		if (ret)
			return ret;
		tmp &= ~bit;
		return i2c_write(addr, dir_reg, 1, &tmp, 1);
	}
}

int setup_board_switches(void)
{
	int ret;
	int i;

	for (i = 0; i < NUM_SWITCH; i++) {
		ret = setup_soft_switch(SWITCH_ADDR + i,
				&switch_config_array[i]);
		if (ret)
			return ret;
	}
	return 0;
}
