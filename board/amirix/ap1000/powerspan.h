/**
 * @file powerspan.h Header file for PowerSpan II code.
 */

/*
 * (C) Copyright 2005
 * AMIRIX Systems Inc.
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

#ifndef POWERSPAN_H
#define POWERSPAN_H

#define CLEAR_MASTER_ABORT 0xdeadbeef
#define NO_DEVICE_FOUND     -1
#define ILLEGAL_REG_OFFSET  -2
#define I2C_BUSY            -3
#define I2C_ERR             -4

#define REG_P1_CSR          0x004
#define REGS_P1_BST         0x018
#define REG_P1_ERR_CSR      0x150
#define REG_P1_MISC_CSR     0x160
#define REGS_P1_TGT_CSR     0x100
#define REGS_P1_TGT_TADDR   0x104
#define REGS_PB_SLAVE_CSR   0x200
#define REGS_PB_SLAVE_TADDR 0x204
#define REGS_PB_SLAVE_BADDR 0x208
#define REG_CONFIG_ADDRESS  0x290
#define REG_CONFIG_DATA     0x294
#define REG_PB_ERR_CSR      0x2B0
#define REG_PB_MISC_CSR     0x2C0
#define REG_MISC_CSR        0x400
#define REG_I2C_CSR         0x408
#define REG_RESET_CSR       0x40C
#define REG_ISR0            0x410
#define REG_ISR1            0x414
#define REG_IER0            0x418
#define REG_MBOX_MAP        0x420
#define REG_HW_MAP          0x42C
#define REG_IDR             0x444

#define CSR_MEMORY_SPACE_ENABLE 0x00000002
#define CSR_PCI_MASTER_ENABLE   0x00000004

#define P1_BST_OFF  0x04

#define PX_ERR_ERR_STATUS   0x01000000

#define PX_MISC_CSR_MAX_RETRY_MASK  0x00000F00
#define PX_MISC_CSR_MAX_RETRY       0x00000F00
#define PX_MISC_REG_BAR_ENABLE      0x00008000
#define PB_MISC_TEA_ENABLE          0x00000010
#define PB_MISC_MAC_TEA             0x00000040

#define P1_TGT_IMAGE_OFF    0x010
#define PX_TGT_CSR_IMG_EN   0x80000000
#define PX_TGT_CSR_TA_EN    0x40000000
#define PX_TGT_CSR_BAR_EN   0x20000000
#define PX_TGT_CSR_MD_EN    0x10000000
#define PX_TGT_CSR_MODE     0x00800000
#define PX_TGT_CSR_DEST     0x00400000
#define PX_TGT_CSR_MEM_IO   0x00200000
#define PX_TGT_CSR_GBL      0x00080000
#define PX_TGT_CSR_CL       0x00040000
#define PX_TGT_CSR_PRKEEP   0x00000080

#define PX_TGT_CSR_BS_MASK      0x0F000000
#define PX_TGT_MEM_IO           0x00200000
#define PX_TGT_CSR_RTT_MASK     0x001F0000
#define PX_TGT_CSR_RTT_READ     0x000A0000
#define PX_TGT_CSR_WTT_MASK     0x00001F00
#define PX_TGT_CSR_WTT_WFLUSH   0x00000200
#define PX_TGT_CSR_END_MASK     0x00000060
#define PX_TGT_CSR_BIG_END      0x00000040
#define PX_TGT_CSR_TRUE_LEND    0x00000060
#define PX_TGT_CSR_RDAMT_MASK   0x00000007

#define PX_TGT_CSR_BS_64MB  0xa
#define PX_TGT_CSR_BS_16MB  0x8

#define PX_TGT_USE_MEM_IO   1
#define PX_TGT_NOT_MEM_IO   0

#define PB_SLAVE_IMAGE_OFF  0x010
#define PB_SLAVE_CSR_IMG_EN 0x80000000
#define PB_SLAVE_CSR_TA_EN  0x40000000
#define PB_SLAVE_CSR_MD_EN  0x20000000
#define PB_SLAVE_CSR_MODE   0x00800000
#define PB_SLAVE_CSR_DEST   0x00400000
#define PB_SLAVE_CSR_MEM_IO 0x00200000
#define PB_SLAVE_CSR_PRKEEP 0x00000080

#define PB_SLAVE_CSR_BS_MASK    0x1F000000
#define PB_SLAVE_CSR_END_MASK   0x00000060
#define PB_SLAVE_CSR_BIG_END    0x00000040
#define PB_SLAVE_CSR_TRUE_LEND  0x00000060
#define PB_SLAVE_CSR_RDAMT_MASK 0x00000007

#define PB_SLAVE_USE_MEM_IO 1
#define PB_SLAVE_NOT_MEM_IO 0


#define MISC_CSR_PCI1_LOCK  0x00000080

#define I2C_CSR_ADDR      0xFF000000  /* Specifies I2C Device Address to be Accessed */
#define I2C_CSR_DATA      0x00FF0000  /* Specifies the Required Data for a Write */
#define I2C_CSR_DEV_CODE  0x0000F000  /* Device Select. I2C 4-bit Device Code */
#define I2C_CSR_CS        0x00000E00  /* Chip Select */
#define I2C_CSR_RW        0x00000100  /* Read/Write */
#define I2C_CSR_ACT       0x00000080  /* I2C Interface Active */
#define I2C_CSR_ERR       0x00000040  /* Error */

#define I2C_EEPROM_DEV      0xa
#define I2C_EEPROM_CHIP_SEL 0

#define I2C_READ    0
#define I2C_WRITE   1

#define RESET_CSR_EEPROM_LOAD 0x00000010

#define ISR_CLEAR_ALL   0xFFFFFFFF

#define IER0_DMA_INTS_EN    0x0F000000
#define IER0_PCI_1_EN       0x00400000
#define IER0_HW_INTS_EN     0x003F0000
#define IER0_MB_INTS_EN     0x000000FF
#define IER0_DEFAULT        (IER0_DMA_INTS_EN | IER0_PCI_1_EN | IER0_HW_INTS_EN | IER0_MB_INTS_EN)

#define MBOX_MAP_TO_INT4    0xCCCCCCCC

#define HW_MAP_HW4_TO_INT4  0x000C0000

#define IDR_PCI_A_OUT   0x40000000
#define IDR_MBOX_OUT    0x10000000


int pci_read_config_byte(int bus, int dev, int fn, int reg, unsigned char* val);
int pci_write_config_byte(int bus, int dev, int fn, int reg, unsigned char val);
int pci_read_config_word(int bus, int dev, int fn, int reg, unsigned short* val);
int pci_write_config_word(int bus, int dev, int fn, int reg, unsigned short val);
int pci_read_config_dword(int bus, int dev, int fn, int reg, unsigned long* val);
int pci_write_config_dword(int bus, int dev, int fn, int reg, unsigned long val);

unsigned int PowerSpanRead(unsigned int theOffset);
void PowerSpanWrite(unsigned int theOffset, unsigned int theValue);

int I2CAccess(unsigned char theI2CAddress, unsigned char theDevCode, unsigned char theChipSel, unsigned char* theValue, int RWFlag);

int PCIWriteConfig(int bus, int dev, int fn, int reg, int width, unsigned long val);
int PCIReadConfig(int bus, int dev, int fn, int reg, int width, unsigned long* val);

int SetSlaveImage(int theImageIndex, unsigned int theBlockSize, int theMemIOFlag, int theEndianness, unsigned int theLocalBaseAddr, unsigned int thePCIBaseAddr);
int SetTargetImage(int theImageIndex, unsigned int theBlockSize, int theMemIOFlag, int theEndianness, unsigned int theLocalBaseAddr, unsigned int thePCIBaseAddr);

#endif
