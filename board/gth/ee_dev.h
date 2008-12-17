/* By Thomas.Lange@Corelatus.com 001025
   $Revision: 1.6 $

   Definitions for EEPROM/VOLT METER  DS2438
   Copyright (C) 2000-2001 Corelatus AB */

#ifndef INCeedevh
#define INCeedevh

#define E_DEBUG(fmt,args...) if( Debug ) printk(KERN_DEBUG"EE: " fmt, ##args)

#define PORT_B_PAR ((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbpar
#define PORT_B_ODR ((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbodr
#define PORT_B_DIR ((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdir
#define PORT_B_DAT ((volatile immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_pbdat

#define SET_PORT_B_INPUT(Mask)  PORT_B_DIR &= ~(Mask)
#define SET_PORT_B_OUTPUT(Mask) PORT_B_DIR |= Mask

#define WRITE_PORT_B(Mask,Value) { \
			if (Value) PORT_B_DAT |= Mask; \
			else PORT_B_DAT &= ~(Mask); \
		}
#define WRITE_PORT(Value) WRITE_PORT_B(PB_EEPROM,Value)

#define READ_PORT (PORT_B_DAT&PB_EEPROM)

/* 64 bytes chip */
#define EE_CHIP_SIZE 64

/* We use this resistor for measuring the current drain on 3.3V */
#define CURRENT_RESISTOR 0.022

/* microsecs
   Pull line down at least this long for reset pulse */
#define RESET_LOW_TIME    490

/* Read presence pulse after we release reset pulse */
#define PRESENCE_TIMEOUT  100
#define PRESENCE_LOW_TIME 200

#define WRITE_0_LOW 80
#define WRITE_1_LOW 2
#define TOTAL_WRITE_LOW 80

#define READ_LOW        2
#define READ_TIMEOUT   10
#define TOTAL_READ_LOW 80

/*** Rom function commands ***/
#define READ_ROM   0x33
#define MATCH_ROM  0x55
#define SKIP_ROM   0xCC
#define SEARCH_ROM 0xF0


/*** Memory_command_function ***/
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD  0xBE
#define COPY_SCRATCHPAD  0x48
#define RECALL_MEMORY    0xB8
#define CONVERT_TEMP     0x44
#define CONVERT_VOLTAGE  0xB4

/* Chip is divided in 8 pages, 8 bytes each */

#define EE_PAGE_SIZE 8

/* All chip data we want are in page 0 */

/* Bytes in page 0 */
#define EE_P0_STATUS   0
#define EE_P0_TEMP_LSB 1
#define EE_P0_TEMP_MSB 2
#define EE_P0_VOLT_LSB 3
#define EE_P0_VOLT_MSB 4
#define EE_P0_CURRENT_LSB 5
#define EE_P0_CURRENT_MSB 6


/* 40 byte user data is located at page 3-7 */
#define EE_USER_PAGE_0 3
#define USER_PAGES 5

#endif /* INCeedevh */
