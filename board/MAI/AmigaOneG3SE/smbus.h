#ifndef _SMBUS_H_
#define _SMBUS_H_

#include "short_types.h"

#define SM_DIMM0_ADDR 0x51
#define SM_DIMM1_ADDR 0x52

void sm_write_mode(void);
void sm_read_mode(void);
void sm_write_byte(uint8 writeme);
uint8 sm_read_byte(void);
int sm_get_ack(void);
void sm_write_ack(void);
void sm_write_nack(void);
void sm_send_start(void);
void sm_send_stop(void);
int sm_read_byte_from_device(uint8 addr, uint8 reg, uint8 *storage);
int sm_get_data(uint8 *DataArray, int dimm_socket);
void sm_init(void);
void sm_term(void);
#endif
