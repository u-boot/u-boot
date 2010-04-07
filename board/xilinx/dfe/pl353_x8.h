 /******************************************************************************
*
* $xilinx_legal_disclaimer
*
******************************************************************************/

#ifndef pl353_H
#define pl353_H

#include <common.h>
/* pl353 base address*/
#define PARPORT_CRTL_BASEADDR			0xE000E000
#define NOR_FLASH_BASEADDR                      0xE2000000

/* Now for the register offsets */
#define PARPORT_MC_OFFSET			0x000	/* Begining of Memory controller config reg offset */

#define PARPORT_MC_STATUS			0x000
#define PARPORT_MC_INTERFACE_CONFIG		0x004
#define PARPORT_MC_SET_CONFIG			0x008
#define PARPORT_MC_CLR_CONFIG			0x00C
#define PARPORT_MC_DIRECT_CMD			0x010
#define PARPORT_MC_SET_CYCLES			0x014
#define PARPORT_MC_SET_OPMODE			0x018
#define PARPORT_MC_REFRESH_PERIOD_0		0x020
#define PARPORT_MC_REFRESH_PERIOD_1		0x024

#define PARPORT_CS_OFFSET			0x100   /* Begining of Chip select config reg offset*/

#define PARPORT_CS_INTERFACE_1_CHIP_3_OFFSET	0x1E0	/* Interface 1 chip 3 configuration */
#define PARPORT_CS_INTERFACE_1_CHIP_2_OFFSET	0x1C0	/* Interface 1 chip 2 configuration */
#define PARPORT_CS_INTERFACE_1_CHIP_1_OFFSET	0x1A0	/* Interface 1 chip 1 configuration */
#define PARPORT_CS_INTERFACE_1_CHIP_0_OFFSET	0x180	/* Interface 1 chip 0 configuration */
#define PARPORT_CS_INTERFACE_0_CHIP_3_OFFSET	0x160	/* Interface 0 chip 3 configuration */
#define PARPORT_CS_INTERFACE_0_CHIP_2_OFFSET	0x140	/* Interface 0 chip 2 configuration */
#define PARPORT_CS_INTERFACE_0_CHIP_1_OFFSET	0x120	/* Interface 0 chip 1 configuration */
#define PARPORT_CS_INTERFACE_0_CHIP_0_OFFSET	0x100	/* Interface 0 chip 0 configuration */

#define PARPORT_UC_OFFSET			0x200   /* Begining of User config reg offset*/

#define PARPORT_UC_CONFIG_OFFSET		0x204	/* user config reg */
#define PARPORT_UC_STATUS_OFFSET		0x200	/* user status reg */

#define PARPORT_IT_OFFSET			0xE00	/* Inegration test */

#define PARPORT_ID_OFFSET			0xFE0   /* Begining of PrimeCell ID config reg offset*/
#define PARPORT_ID_PCELL_3_OFFSET		0xFFC
#define PARPORT_ID_PCELL_2_OFFSET		0xFF8
#define PARPORT_ID_PCELL_1_OFFSET		0xFF4
#define PARPORT_ID_PCELL_0_OFFSET		0xFF0
#define PARPORT_ID_PERIP_3_OFFSET		0xFEC
#define PARPORT_ID_PERIP_2_OFFSET		0xFE8
#define PARPORT_ID_PERIP_1_OFFSET		0xFE4
#define PARPORT_ID_PERIP_0_OFFSET		0xFE0

/*              Read Write macros         */

/* Write to memory location or register */
#define X_mWriteReg(BASE_ADDRESS, RegOffset, data) \
           *(unsigned long *)(BASE_ADDRESS + RegOffset) = ((unsigned long) data);
/* Read from memory location or register */
#define X_mReadReg(BASE_ADDRESS, RegOffset) \
           *(unsigned long *)(BASE_ADDRESS + RegOffset);

typedef volatile u32 XIo_Address;

typedef u32 AddressType;

void Xil_Out8(XIo_Address OutAddress, u8 Value);
u8 Xil_In8(XIo_Address InAddress);

/* Function definitionas */
/*
 * init_nor_flash init the parameters of pl353 for the P30 flash
 */
void init_nor_flash(void);

/*
 * wirte_nor_flash
 */
void write_half_word_nor_flash(u32 address, u8 data);
void write_byte_nor_flash(u32 address, u8 data);
/*
 * read_nor_flash returns the data of a memeory location
 */
u16  read_half_word_nor_flash(u32 address);

/*
 * read_status_reg_nor_flash returns the status register of a block address
 */
u16  read_status_reg_nor_flash(u32 address);

/*
 * clear_status_reg_nor_flash clears the status register of a block address
 */
void clear_status_reg_nor_flash(u32 address);

/*
 * unlock_nor_flash put the selected block of address in unlock mode
 */
void unlock_nor_flash(u32 blockAddress);

/*
 * lock_nor_flash put the selected block of address in lock mode
 */
void lock_nor_flash(u32 blockAddress);

/*
 * block_erase_nor_flash
 */
void block_erase_nor_flash(u32 blockAddress);

/*
 * bufferred_write_nor_flash
 */
void buffered_wirte_nor_flash(u32 address, u8 *dataBuffer, u16 wordCount);

#endif
