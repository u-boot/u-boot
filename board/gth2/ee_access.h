/* By Thomas.Lange@Corelatus.com 001025 */

/* Definitions for EEPROM/VOLT METER  DS2438 */
/* Copyright (C) 2000-2005 Corelatus AB */

#ifndef INCeeaccessh
#define INCeeaccessh

#include <asm/types.h>
#include "ee_dev.h"

int ee_do_cpu_command( u8 *Tx, int Tx_len, u8 *Rx, int Rx_len, int Send_skip );
int ee_init_cpu_data(void);

int ee_crc_ok( u8 *Buffer, int Len, u8 Crc );

/* Defs for altera reg */
#define EE_WRITE_SHIFT 8 /* bits to shift left */
#define EE_READ_SHIFT 16 /* bits to shift left */
#define EE_DONE  0x80000000
#define EE_BUSY  0x40000000
#define EE_ERROR 0x20000000

/* Commands */
#define EE_CMD_NOP      0
#define EE_CMD_INIT_RES 1
#define EE_CMD_WR_BYTE  2
#define EE_CMD_RD_BYTE  3

#endif /* INCeeaccessh */
