/* By Thomas.Lange@Corelatus.com 001025

   Definitions for EEPROM/VOLT METER  DS2438 */

#ifndef INCeeaccessh
#define INCeeaccessh

int ee_do_command( u8 *Tx, int Tx_len, u8 *Rx, int Rx_len, int Send_skip );
int ee_init_data(void);
int ee_crc_ok( u8 *Buffer, int Len, u8 Crc );

#ifndef TRUE
#define TRUE 1
#endif

#endif /* INCeeaccessh */
