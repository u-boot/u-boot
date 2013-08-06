/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BOARD_SOFT_SWITCH_H__
#define __BOARD_SOFT_SWITCH_H__

#include <asm/soft_switch.h>

/* switch 0 port A */
#define CAN_EN                 0x1
#define CAN_STB                0x2
#define CAN0_ERR_EN            0x4
#define CAN0RX_EN              0x8
#define CNT0UD_EN              0x10
#define CNT0DG_EN              0x20
#define CNT0ZM_EN              0x40
#define RMII_CLK_EN            0x80

/* switch 0 port B */
#define UART0RTS_EN            0x1
#define UART0RX_EN             0x2
#define UART0CTS_EN            0x4
#define UART0CTS_RTS_LPBK      0x8
#define UART0CTS_RST_EN        0x10
#define UART0CTS_146_EN        0x20
#define TEMP_IRQ_EN            0x40
#define TEMP_THERM_EN          0x80

/* switch 1 port A */
#define OVERRIDE_SMC0_LP0_BOOT 0x1
#define SMC0_EPPI2_LP1_SWITCH  0x2
#define SMC0_LP0_EN            0x8
#define LED1_GPIO_EN           0x10
#define LED2_GPIO_EN           0x20
#define LED3_GPIO_EN           0x40
#define LED4_GPIO_EN           0x80

/* switch 1 port B */
#define PUSHBUTTON1_EN         0x1
#define PUSHBUTTON2_EN         0x2
#define SD_CD_EN               0x4
#define SD_WP_EN               0x8
#define SPIFLASH_CS_EN         0x10
#define SPI0D2_EN              0x20
#define SPI0D3_EN              0x40

/* switch 2 port A */
#define PHYINT_EN              0x1
#define PHY_PWR_DWN_INT        0x2
#define PHYAD0                 0x4
#define ETHERNET_EN            0x8
#define WAKE_PUSHBUTTON_EN     0x10
#define PD0_SPI0D2_EN          0x20
#define PD1_SPI0D3_EN          0x40
#define PD2_SPI0MISO_EN        0x80

/* switch 2 port B */
#define PD3_SPI0MOSI_EN        0x1
#define PD4_SPI0CK_EN          0x2

#ifdef CONFIG_BFIN_BOARD_VERSION_1_0
#define SWITCH_ADDR     0x21
#else
#define SWITCH_ADDR     0x20
#endif

#define NUM_SWITCH      3
#define IODIRA          0x0
#define IODIRB          0x1
#define OLATA           0x14
#define OLATB           0x15

int setup_board_switches(void);

#endif /* __BOARD_SOFT_SWITCH_H__ */
