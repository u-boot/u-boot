/*
 * (C) Copyright 2003
 * Data Flash Atmel Description File
 * Author : Hamid Ikdoumi (Atmel)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* File Name		: dataflash.h					*/
/* Object		: Data Flash Atmel Description File		*/
/* Translator		:						*/
/*									*/
/* 1.0 03/04/01 HI	: Creation					*/
/* 1.2 20/10/02 FB	: Adapatation Service and Lib v3		*/
/*----------------------------------------------------------------------*/

#ifndef _DataFlash_h
#define _DataFlash_h


#include <asm/arch/hardware.h>
#include "config.h"

/*number of protected area*/
#ifdef	CONFIG_NEW_PARTITION
# define NB_DATAFLASH_AREA	6
#else
# define NB_DATAFLASH_AREA	4
#endif

#ifdef CFG_NO_FLASH

/*-----------------------------------------------------------------------
 * return codes from flash_write():
 */
# define ERR_OK				0
# define ERR_TIMOUT			1
# define ERR_NOT_ERASED			2
# define ERR_PROTECTED			4
# define ERR_INVAL			8
# define ERR_ALIGN			16
# define ERR_UNKNOWN_FLASH_VENDOR	32
# define ERR_UNKNOWN_FLASH_TYPE		64
# define ERR_PROG_ERROR			128

/*-----------------------------------------------------------------------
 * Protection Flags for flash_protect():
 */
# define FLAG_PROTECT_SET		0x01
# define FLAG_PROTECT_CLEAR		0x02
# define FLAG_PROTECT_INVALID		0x03

/*-----------------------------------------------------------------------
 * Set Environment according to label:
 */
# define	FLAG_SETENV		0x80
#endif /* CFG_NO_FLASH */

/*define the area structure*/
typedef struct {
	unsigned long start;
	unsigned long end;
	unsigned char protected;
	unsigned char setenv;
	unsigned char label[20];
} dataflash_protect_t;

typedef unsigned int AT91S_DataFlashStatus;

/*----------------------------------------------------------------------*/
/* DataFlash Structures							*/
/*----------------------------------------------------------------------*/

/*---------------------------------------------*/
/* DataFlash Descriptor Structure Definition   */
/*---------------------------------------------*/
typedef struct _AT91S_DataflashDesc {
	unsigned char *tx_cmd_pt;
	unsigned int tx_cmd_size;
	unsigned char *rx_cmd_pt;
	unsigned int rx_cmd_size;
	unsigned char *tx_data_pt;
	unsigned int tx_data_size;
	unsigned char *rx_data_pt;
	unsigned int rx_data_size;
	volatile unsigned char state;
	volatile unsigned char DataFlash_state;
	unsigned char command[8];
} AT91S_DataflashDesc, *AT91PS_DataflashDesc;

/*---------------------------------------------*/
/* DataFlash device definition structure       */
/*---------------------------------------------*/
typedef struct _AT91S_Dataflash {
	int pages_number;			/* dataflash page number */
	int pages_size;				/* dataflash page size */
	int page_offset;			/* page offset in command */
	int byte_mask;				/* byte mask in command */
	int cs;
	dataflash_protect_t area_list[NB_DATAFLASH_AREA]; /* area protection status */
} AT91S_DataflashFeatures, *AT91PS_DataflashFeatures;

/*---------------------------------------------*/
/* DataFlash Structure Definition	       */
/*---------------------------------------------*/
typedef struct _AT91S_DataFlash {
	AT91PS_DataflashDesc pDataFlashDesc;	/* dataflash descriptor */
	AT91PS_DataflashFeatures pDevice;	/* Pointer on a dataflash features array */
} AT91S_DataFlash, *AT91PS_DataFlash;


typedef struct _AT91S_DATAFLASH_INFO {

	AT91S_DataflashDesc Desc;
	AT91S_DataflashFeatures Device; /* Pointer on a dataflash features array */
	unsigned long logical_address;
	unsigned long end_address;
	unsigned int id;			/* device id */
} AT91S_DATAFLASH_INFO, *AT91PS_DATAFLASH_INFO;


/*-------------------------------------------------------------------------------------------------*/

#define AT45DB161	0x2c
#define AT45DB321	0x34
#define AT45DB642	0x3c
#define AT45DB128	0x10
#define	PAGES_PER_BLOCK	8

#define AT91C_DATAFLASH_TIMEOUT		10000	/* For AT91F_DataFlashWaitReady */

/* DataFlash return value */
#define DATAFLASH_BUSY			0x00
#define DATAFLASH_OK			0x01
#define DATAFLASH_ERROR			0x02
#define DATAFLASH_MEMORY_OVERFLOW	0x03
#define DATAFLASH_BAD_COMMAND		0x04
#define DATAFLASH_BAD_ADDRESS		0x05


/* Driver State */
#define IDLE		0x0
#define BUSY		0x1
#define ERROR		0x2

/* DataFlash Driver State */
#define GET_STATUS	0x0F

/*-------------------------------------------------------------------------------------------------*/
/* Command Definition										   */
/*-------------------------------------------------------------------------------------------------*/

/* READ COMMANDS */
#define DB_CONTINUOUS_ARRAY_READ	0xE8	/* Continuous array read */
#define DB_BURST_ARRAY_READ		0xE8	/* Burst array read */
#define DB_PAGE_READ			0xD2	/* Main memory page read */
#define DB_BUF1_READ			0xD4	/* Buffer 1 read */
#define DB_BUF2_READ			0xD6	/* Buffer 2 read */
#define DB_STATUS			0xD7	/* Status Register */

/* PROGRAM and ERASE COMMANDS */
#define DB_BUF1_WRITE			0x84	/* Buffer 1 write */
#define DB_BUF2_WRITE			0x87	/* Buffer 2 write */
#define DB_BUF1_PAGE_ERASE_PGM		0x83	/* Buffer 1 to main memory page program with built-In erase */
#define DB_BUF1_PAGE_ERASE_FASTPGM	0x93	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF2_PAGE_ERASE_PGM		0x86	/* Buffer 2 to main memory page program with built-In erase */
#define DB_BUF2_PAGE_ERASE_FASTPGM	0x96	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF1_PAGE_PGM		0x88	/* Buffer 1 to main memory page program without built-In erase */
#define DB_BUF1_PAGE_FASTPGM		0x98	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_BUF2_PAGE_PGM		0x89	/* Buffer 2 to main memory page program without built-In erase */
#define DB_BUF2_PAGE_FASTPGM		0x99	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_PAGE_ERASE			0x81	/* Page Erase */
#define DB_BLOCK_ERASE			0x50	/* Block Erase */
#define DB_PAGE_PGM_BUF1		0x82	/* Main memory page through buffer 1 */
#define DB_PAGE_FASTPGM_BUF1		0x92	/* Main memory page through buffer 1, Fast program */
#define DB_PAGE_PGM_BUF2		0x85	/* Main memory page through buffer 2 */
#define DB_PAGE_FastPGM_BUF2		0x95	/* Main memory page through buffer 2, Fast program */

/* ADDITIONAL COMMANDS */
#define DB_PAGE_2_BUF1_TRF		0x53	/* Main memory page to buffer 1 transfert */
#define DB_PAGE_2_BUF2_TRF		0x55	/* Main memory page to buffer 2 transfert */
#define DB_PAGE_2_BUF1_CMP		0x60	/* Main memory page to buffer 1 compare */
#define DB_PAGE_2_BUF2_CMP		0x61	/* Main memory page to buffer 2 compare */
#define DB_AUTO_PAGE_PGM_BUF1		0x58	/* Auto page rewrite throught buffer 1 */
#define DB_AUTO_PAGE_PGM_BUF2		0x59	/* Auto page rewrite throught buffer 2 */

/*-------------------------------------------------------------------------------------------------*/

extern int size_dataflash (AT91PS_DataFlash pdataFlash, unsigned long addr, unsigned long size);
extern int prot_dataflash (AT91PS_DataFlash pdataFlash, unsigned long addr);
extern int addr2ram(ulong addr);
extern int dataflash_real_protect (int flag, unsigned long start_addr, unsigned long end_addr);
extern int addr_dataflash (unsigned long addr);
extern int read_dataflash (unsigned long addr, unsigned long size, char *result);
extern int write_dataflash (unsigned long addr, unsigned long dest, unsigned long size);
extern void dataflash_print_info (void);
extern void dataflash_perror (int err);

#ifdef	CONFIG_NEW_DF_PARTITION
extern int AT91F_DataflashSetEnv (void); #endif
#endif

#endif
