/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* common srom defs */
#define FIRST_DEVICE            0x00
#define SECOND_DEVICE           0x04
#define FIRST_BLOCK             0x00
#define SECOND_BLOCK            0x02
#define BLOCK_SIZE              0x100
#define ERROR                   (-1)

#define CLK2P0TO1_1MB_PB_0P5DH  0x79000100
#define CLK2P5TO1_1MB_PB_0P5DH  0x7B000100

#define CPU_TYPE_740            0x08
#define CPU_TYPE_750            0x08
#define CPU_TYPE                ((get_pvr()>>16)&0xffff)

#define ABS(x)                  ((x<0)?-x:x)
#define SROM_SHORT(pX)          (*(u8 *)(pX) | *((u8 *)(pX)+1) << 8)

/* bab7xx ELTEC srom */
#define I2C_BUS_DAT             (CONFIG_SYS_ISA_IO + 0x220)
#define I2C_BUS_DIR             (CONFIG_SYS_ISA_IO + 0x221)

/* srom at mpc107 */
#define MPC107_I2CADDR          (mpc107_eumb_addr + 0x3000)     /* address      */
#define MPC107_I2CFDR           (mpc107_eumb_addr + 0x3004)     /* freq divider */
#define MPC107_I2CCR            (mpc107_eumb_addr + 0x3008)     /* control      */
#define MPC107_I2CSR            (mpc107_eumb_addr + 0x300c)     /* status       */
#define MPC107_I2CDR            (mpc107_eumb_addr + 0x3010)     /* data         */
#define MPC107_I2C_TIMEOUT      10000000

/* i82559 */
#define EE_ADDR_BITS            6
#define EE_SIZE                 0x40                            /* 0x40 words */
#define EE_CHECKSUM             0xBABA

/* dc21143 */
#define DEC_SROM_SIZE           128


/*
 * structure of revision srom
 */
typedef struct  {
    char    magic[8];           /* 000 - Magic number */
    char    revrev[2];          /* 008 - Revision of structure */
    unsigned short size;        /* 00A - Size of CRC area */
    unsigned long  crc;         /* 00C - CRC */
    char    board[16];          /* 010 - Board Revision information */
    char    option[4][16];      /* 020 - Option Revision information */
    char    serial[8];          /* 060 - Board serial number */
    char    etheraddr[6];       /* 068 - Ethernet node addresse */
    char    reserved[2];        /* 06E - Reserved */
    char    revision[7][2];     /* 070 - Revision codes */
    char    category[2];        /* 07E - Category codes */
    char    text[64];           /* 080 - Text field */
    char    res[64];            /* 0C0 - Reserved */
} revinfo;

unsigned long el_srom_checksum (unsigned char *ptr, unsigned long size);
int el_srom_load  (unsigned char addr, unsigned char *buf, int cnt,
		   unsigned char device, unsigned char block);
int el_srom_store (unsigned char addr, unsigned char *buf, int cnt,
		   unsigned char device, unsigned char block);

int mpc107_i2c_init (unsigned long eumb_addr, unsigned long divider);
int mpc107_i2c_read_byte (unsigned char device, unsigned char block, unsigned char offset);
int mpc107_i2c_write_byte (unsigned char device, unsigned char block,
			   unsigned char offset, unsigned char val);
int mpc107_srom_load (unsigned char addr, unsigned char *pBuf, int cnt,
		      unsigned char device, unsigned char block);
int mpc107_srom_store (unsigned char addr, unsigned char *pBuf, int cnt,
		       unsigned char device, unsigned char block);

int dc_srom_load (unsigned short *dest);
int dc_srom_store (unsigned short *src);

unsigned short eepro100_srom_checksum (unsigned short *sromdata);
void eepro100_srom_load (unsigned short *destination);
int  eepro100_srom_store (unsigned short *source);
