/*
 * Copyright (C) ST-Ericsson SA 2009
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _UX500_GPIO_h
#define _UX500_GPIO_h

#include <asm/types.h>
#include <asm/io.h>
#include <asm/errno.h>

#include <asm/arch/sys_proto.h>
#include <asm/arch/u8500.h>

#define GPIO_TOTAL_PINS                 268

#define GPIO_PINS_PER_BLOCK	32
#define GPIO_BLOCKS_COUNT       (GPIO_TOTAL_PINS/GPIO_PINS_PER_BLOCK + 1)
#define GPIO_BLOCK(pin)		(((pin + GPIO_PINS_PER_BLOCK) >> 5) - 1)


struct gpio_register {
	u32 gpio_dat;	/* data register : 0x000 */
	u32 gpio_dats;	/* data Set register : 0x004 */
	u32 gpio_datc;	/* data Clear register : 0x008 */
	u32 gpio_pdis;	/* Pull disable register : 0x00C */
	u32 gpio_dir;	/* data direction register : 0x010 */
	u32 gpio_dirs;	/* data dir Set register : 0x014 */
	u32 gpio_dirc;	/* data dir Clear register : 0x018 */
	u32 gpio_slpm;	/* Sleep mode register : 0x01C */
	u32 gpio_afsa;	/* AltFun A Select reg : 0x020 */
	u32 gpio_afsb;	/* AltFun B Select reg : 0x024 */
	u32 gpio_lowemi;/* low EMI Select reg : 0x028 */
	u32 reserved_1[(0x040 - 0x02C) >> 2];	/*0x028-0x3C Reserved*/
	u32 gpio_rimsc;	/* rising edge intr set/clear : 0x040 */
	u32 gpio_fimsc;	/* falling edge intr set/clear register : 0x044 */
	u32 gpio_mis;	/* masked interrupt status register : 0x048 */
	u32 gpio_ic;	/* Interrupt Clear register : 0x04C */
	u32 gpio_rwimsc;/* Rising-edge Wakeup IMSC register : 0x050 */
	u32 gpio_fwimsc;/* Falling-edge Wakeup IMSC register : 0x054 */
	u32 gpio_wks;	/* Wakeup Status register : 0x058 */
};

/* Error values returned by functions */
enum gpio_error {
	GPIO_OK = 0,
	GPIO_UNSUPPORTED_HW = -2,
	GPIO_UNSUPPORTED_FEATURE = -3,
	GPIO_INVALID_PARAMETER = -4,
	GPIO_REQUEST_NOT_APPLICABLE = -5,
	GPIO_REQUEST_PENDING = -6,
	GPIO_NOT_CONFIGURED = -7,
	GPIO_INTERNAL_ERROR = -8,
	GPIO_INTERNAL_EVENT = 1,
	GPIO_REMAINING_EVENT = 2,
	GPIO_NO_MORE_PENDING_EVENT = 3,
	GPIO_INVALID_CLIENT = -25,
	GPIO_INVALID_PIN = -26,
	GPIO_PIN_BUSY = -27,
	GPIO_PIN_NOT_ALLOCATED = -28,
	GPIO_WRONG_CLIENT = -29,
	GPIO_UNSUPPORTED_ALTFUNC = -30,
};

/*GPIO DEVICE ID */
enum gpio_device_id {
	GPIO_DEVICE_ID_0,
	GPIO_DEVICE_ID_1,
	GPIO_DEVICE_ID_2,
	GPIO_DEVICE_ID_3,
	GPIO_DEVICE_ID_INVALID
};

/*
 * Alternate Function:
 *  refered in altfun_table to pointout particular altfun to be enabled
 *  when using GPIO_ALT_FUNCTION A/B/C enable/disable operation
 */
enum gpio_alt_function {
	GPIO_ALT_UART_0_MODEM,
	GPIO_ALT_UART_0_NO_MODEM,
	GPIO_ALT_UART_1,
	GPIO_ALT_UART_2,
	GPIO_ALT_I2C_0,
	GPIO_ALT_I2C_1,
	GPIO_ALT_I2C_2,
	GPIO_ALT_I2C_3,
	GPIO_ALT_MSP_0,
	GPIO_ALT_MSP_1,
	GPIO_ALT_MSP_2,
	GPIO_ALT_MSP_3,
	GPIO_ALT_MSP_4,
	GPIO_ALT_MSP_5,
	GPIO_ALT_SSP_0,
	GPIO_ALT_SSP_1,
	GPIO_ALT_MM_CARD0,
	GPIO_ALT_SD_CARD0,
	GPIO_ALT_DMA_0,
	GPIO_ALT_DMA_1,
	GPIO_ALT_HSI0,
	GPIO_ALT_CCIR656_INPUT,
	GPIO_ALT_CCIR656_OUTPUT,
	GPIO_ALT_LCD_PANEL,
	GPIO_ALT_MDIF,
	GPIO_ALT_SDRAM,
	GPIO_ALT_HAMAC_AUDIO_DBG,
	GPIO_ALT_HAMAC_VIDEO_DBG,
	GPIO_ALT_CLOCK_RESET,
	GPIO_ALT_TSP,
	GPIO_ALT_IRDA,
	GPIO_ALT_USB_MINIMUM,
	GPIO_ALT_USB_I2C,
	GPIO_ALT_OWM,
	GPIO_ALT_PWL,
	GPIO_ALT_FSMC,
	GPIO_ALT_COMP_FLASH,
	GPIO_ALT_SRAM_NOR_FLASH,
	GPIO_ALT_FSMC_ADDLINE_0_TO_15,
	GPIO_ALT_SCROLL_KEY,
	GPIO_ALT_MSHC,
	GPIO_ALT_HPI,
	GPIO_ALT_USB_OTG,
	GPIO_ALT_SDIO,
	GPIO_ALT_HSMMC,
	GPIO_ALT_FSMC_ADD_DATA_0_TO_25,
	GPIO_ALT_HSI1,
	GPIO_ALT_NOR,
	GPIO_ALT_NAND,
	GPIO_ALT_KEYPAD,
	GPIO_ALT_VPIP,
	GPIO_ALT_CAM,
	GPIO_ALT_CCP1,
	GPIO_ALT_EMMC,
	GPIO_ALT_POP_EMMC,
	GPIO_ALT_FUNMAX		/* Add new alt func before this */
};

/* Defines pin assignment(Software mode or Alternate mode) */
enum gpio_mode {
	GPIO_MODE_LEAVE_UNCHANGED,	/* Parameter will be ignored */
	GPIO_MODE_SOFTWARE,	/* Pin connected to GPIO (SW controlled) */
	GPIO_ALTF_A,		/* Pin connected to altfunc 1 (HW periph 1) */
	GPIO_ALTF_B,		/* Pin connected to altfunc 2 (HW periph 2) */
	GPIO_ALTF_C,		/* Pin connected to altfunc 3 (HW periph 3) */
	GPIO_ALTF_FIND,		/* Pin connected to altfunc 3 (HW periph 3) */
	GPIO_ALTF_DISABLE	/* Pin connected to altfunc 3 (HW periph 3) */
};

/* Defines GPIO pin direction */
enum gpio_direction {
	GPIO_DIR_LEAVE_UNCHANGED,	/* Parameter will be ignored */
	GPIO_DIR_INPUT,		/* GPIO set as input */
	GPIO_DIR_OUTPUT		/* GPIO set as output */
};

/* Interrupt trigger mode */
enum gpio_trig {
	GPIO_TRIG_LEAVE_UNCHANGED,	/* Parameter will be ignored */
	GPIO_TRIG_DISABLE,	/* Trigger no IT */
	GPIO_TRIG_RISING_EDGE,	/* Trigger an IT on rising edge */
	GPIO_TRIG_FALLING_EDGE,	/* Trigger an IT on falling edge */
	GPIO_TRIG_BOTH_EDGES,	/* Trigger an IT on rising and falling edge */
	GPIO_TRIG_HIGH_LEVEL,	/* Trigger an IT on high level */
	GPIO_TRIG_LOW_LEVEL	/* Trigger an IT on low level */
};

/* Configuration parameters for one GPIO pin.*/
struct gpio_config {
	enum gpio_mode mode;
	enum gpio_direction direction;
	enum gpio_trig trig;
	char *dev_name;		/* Who owns the gpio pin */
};

/* GPIO pin data*/
enum gpio_data {
	GPIO_DATA_LOW,
	GPIO_DATA_HIGH
};

/* GPIO behaviour in sleep mode */
enum gpio_sleep_mode {
	GPIO_SLEEP_MODE_LEAVE_UNCHANGED,	/* Parameter will be ignored */
	GPIO_SLEEP_MODE_INPUT_DEFAULTVOLT,	/* GPIO is an input with pull
						   up/down enabled when in sleep
						   mode. */
	GPIO_SLEEP_MODE_CONTROLLED_BY_GPIO	/* GPIO pin is controlled by
						   GPIO IP. So mode, direction
						   and data values for GPIO pin
						   in sleep mode are determined
						   by configuration set to GPIO
						   pin before entering to sleep
						   mode. */
};

/* GPIO ability to wake the system up from sleep mode.*/
enum gpio_wake {
	GPIO_WAKE_LEAVE_UNCHANGED,	/* Parameter will be ignored */
	GPIO_WAKE_DISABLE,	/* No wake of system from sleep mode. */
	GPIO_WAKE_LOW_LEVEL,	/* Wake the system up on a LOW level. */
	GPIO_WAKE_HIGH_LEVEL,	/* Wake the system up on a HIGH level. */
	GPIO_WAKE_RISING_EDGE,	/* Wake the system up on a RISING edge. */
	GPIO_WAKE_FALLING_EDGE,	/* Wake the system up on a FALLING edge. */
	GPIO_WAKE_BOTH_EDGES	/* Wake the system up on both RISE and FALL. */
};

/* Configuration parameters for one GPIO pin in sleep mode.*/
struct gpio_sleep_config {
	enum gpio_sleep_mode sleep_mode;/* GPIO behaviour in sleep mode. */
	enum gpio_wake wake;		/* GPIO ability to wake up system. */
};

extern int gpio_setpinconfig(int pin_id, struct gpio_config *pin_config);
extern int gpio_resetpinconfig(int pin_id, char *dev_name);
extern int gpio_writepin(int pin_id, enum gpio_data value, char *dev_name);
extern int gpio_readpin(int pin_id, enum gpio_data *value);
extern int gpio_altfuncenable(enum gpio_alt_function altfunc,
				      char *dev_name);
extern int gpio_altfuncdisable(enum gpio_alt_function altfunc,
				       char *dev_name);

struct gpio_altfun_data {
	u16 altfun;
	u16 start;
	u16 end;
	u16 cont;
	u8 type;
};
#endif
