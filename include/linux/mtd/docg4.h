/*
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DOCG4_H__
#define __DOCG4_H__

#include <common.h>
#include <linux/mtd/nand.h>

extern int docg4_nand_init(struct mtd_info *mtd,
			   struct nand_chip *nand, int devnum);

/* SPL-related definitions */
#define DOCG4_IPL_LOAD_BLOCK_COUNT 2  /* number of blocks that IPL loads */
#define DOCG4_BLOCK_CAPACITY_SPL 0x10000 /* reliable mode; redundant pages */

#define DOC_IOSPACE_DATA		0x0800

/* register offsets */
#define DOC_CHIPID			0x1000
#define DOC_DEVICESELECT		0x100a
#define DOC_ASICMODE			0x100c
#define DOC_DATAEND			0x101e
#define DOC_NOP				0x103e

#define DOC_FLASHSEQUENCE		0x1032
#define DOC_FLASHCOMMAND		0x1034
#define DOC_FLASHADDRESS		0x1036
#define DOC_FLASHCONTROL		0x1038
#define DOC_ECCCONF0			0x1040
#define DOC_ECCCONF1			0x1042
#define DOC_HAMMINGPARITY		0x1046
#define DOC_BCH_SYNDROM(idx)		(0x1048 + idx)

#define DOC_ASICMODECONFIRM		0x1072
#define DOC_CHIPID_INV			0x1074
#define DOC_POWERMODE			0x107c

#define DOCG4_MYSTERY_REG		0x1050

/* apparently used only to write oob bytes 6 and 7 */
#define DOCG4_OOB_6_7			0x1052

/* DOC_FLASHSEQUENCE register commands */
#define DOC_SEQ_RESET			0x00
#define DOCG4_SEQ_PAGE_READ		0x03
#define DOCG4_SEQ_FLUSH			0x29
#define DOCG4_SEQ_PAGEWRITE		0x16
#define DOCG4_SEQ_PAGEPROG		0x1e
#define DOCG4_SEQ_BLOCKERASE		0x24

/* DOC_FLASHCOMMAND register commands */
#define DOCG4_CMD_PAGE_READ             0x00
#define DOC_CMD_ERASECYCLE2		0xd0
#define DOCG4_CMD_FLUSH                 0x70
#define DOCG4_CMD_READ2                 0x30
#define DOC_CMD_PROG_BLOCK_ADDR		0x60
#define DOCG4_CMD_PAGEWRITE		0x80
#define DOC_CMD_PROG_CYCLE2		0x10
#define DOC_CMD_RESET			0xff

/* DOC_POWERMODE register bits */
#define DOC_POWERDOWN_READY		0x80

/* DOC_FLASHCONTROL register bits */
#define DOC_CTRL_CE			0x10
#define DOC_CTRL_UNKNOWN		0x40
#define DOC_CTRL_FLASHREADY		0x01

/* DOC_ECCCONF0 register bits */
#define DOC_ECCCONF0_READ_MODE		0x8000
#define DOC_ECCCONF0_UNKNOWN		0x2000
#define DOC_ECCCONF0_ECC_ENABLE	        0x1000
#define DOC_ECCCONF0_DATA_BYTES_MASK	0x07ff

/* DOC_ECCCONF1 register bits */
#define DOC_ECCCONF1_BCH_SYNDROM_ERR	0x80
#define DOC_ECCCONF1_ECC_ENABLE         0x07
#define DOC_ECCCONF1_PAGE_IS_WRITTEN	0x20

/* DOC_ASICMODE register bits */
#define DOC_ASICMODE_RESET		0x00
#define DOC_ASICMODE_NORMAL		0x01
#define DOC_ASICMODE_POWERDOWN		0x02
#define DOC_ASICMODE_MDWREN		0x04
#define DOC_ASICMODE_BDETCT_RESET	0x08
#define DOC_ASICMODE_RSTIN_RESET	0x10
#define DOC_ASICMODE_RAM_WE		0x20

/* good status values read after read/write/erase operations */
#define DOCG4_PROGSTATUS_GOOD          0x51
#define DOCG4_PROGSTATUS_GOOD_2        0xe0

/*
 * On read operations (page and oob-only), the first byte read from I/O reg is a
 * status.  On error, it reads 0x73; otherwise, it reads either 0x71 (first read
 * after reset only) or 0x51, so bit 1 is presumed to be an error indicator.
 */
#define DOCG4_READ_ERROR           0x02 /* bit 1 indicates read error */

/* anatomy of the device */
#define DOCG4_CHIP_SIZE        0x8000000
#define DOCG4_PAGE_SIZE        0x200
#define DOCG4_PAGES_PER_BLOCK  0x200
#define DOCG4_BLOCK_SIZE       (DOCG4_PAGES_PER_BLOCK * DOCG4_PAGE_SIZE)
#define DOCG4_NUMBLOCKS        (DOCG4_CHIP_SIZE / DOCG4_BLOCK_SIZE)
#define DOCG4_OOB_SIZE         0x10
#define DOCG4_CHIP_SHIFT       27    /* log_2(DOCG4_CHIP_SIZE) */
#define DOCG4_PAGE_SHIFT       9     /* log_2(DOCG4_PAGE_SIZE) */
#define DOCG4_ERASE_SHIFT      18    /* log_2(DOCG4_BLOCK_SIZE) */

/* all but the last byte is included in ecc calculation */
#define DOCG4_BCH_SIZE         (DOCG4_PAGE_SIZE + DOCG4_OOB_SIZE - 1)

#define DOCG4_USERDATA_LEN     520 /* 512 byte page plus 8 oob avail to user */

/* expected values from the ID registers */
#define DOCG4_IDREG1_VALUE     0x0400
#define DOCG4_IDREG2_VALUE     0xfbff

/* primitive polynomial used to build the Galois field used by hw ecc gen */
#define DOCG4_PRIMITIVE_POLY   0x4443

#define DOCG4_M                14  /* Galois field is of order 2^14 */
#define DOCG4_T                4   /* BCH alg corrects up to 4 bit errors */

#define DOCG4_FACTORY_BBT_PAGE 16 /* page where read-only factory bbt lives */

#endif	/* __DOCG4_H__ */
