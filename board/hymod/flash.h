/*
 * (C) Copyright 2000
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*************** DEFINES for Intel StrataFlash FLASH chip ********************/

/* Commands */
#define ISF_CMD_RST		0xFF		/* reset flash */
#define ISF_CMD_RD_ID		0x90		/* read the id and lock bits */
#define ISF_CMD_RD_QUERY	0x98		/* read device capabilities */
#define ISF_CMD_RD_STAT		0x70		/* read the status register */
#define ISF_CMD_CLR_STAT	0x50		/* clear the staus register */
#define ISF_CMD_WR_BUF		0xE8		/* clear the staus register */
#define ISF_CMD_PROG		0x40		/* program word command */
#define ISF_CMD_ERASE1		0x20		/* 1st word for block erase */
#define ISF_CMD_ERASE2		0xD0		/* 2nd word for block erase */
#define ISF_CMD_ERASE_SUSP	0xB0		/* suspend block erase */
#define ISF_CMD_LOCK		0x60		/* 1st word for all lock cmds */
#define ISF_CMD_SET_LOCK_BLK	0x01		/* 2nd wrd set block lock bit */
#define ISF_CMD_SET_LOCK_MSTR	0xF1		/* 2nd wrd set master lck bit */
#define ISF_CMD_CLR_LOCK_BLK	0xD0		/* 2nd wrd clear blk lck bit */

/* status register bits */
#define ISF_STAT_DPS		0x02		/* Device Protect Status */
#define ISF_STAT_VPPS		0x08		/* VPP Status */
#define ISF_STAT_PSLBS		0x10		/* Program+Set Lock Bit Stat */
#define ISF_STAT_ECLBS		0x20		/* Erase+Clr Lock Bit Stat */
#define ISF_STAT_ESS		0x40		/* Erase Suspend Status */
#define ISF_STAT_RDY		0x80		/* WSM Mach Status, 1=rdy */

#define ISF_STAT_ERR		(ISF_STAT_VPPS | ISF_STAT_DPS | \
				    ISF_STAT_ECLBS | ISF_STAT_PSLBS)

/* register addresses, valid only following an ISF_CMD_RD_ID command */
#define ISF_REG_MAN_CODE	0x00		/* manufacturer code */
#define ISF_REG_DEV_CODE	0x01		/* device code */
#define ISF_REG_BLK_LCK		0x02		/* block lock configuration */
#define ISF_REG_MST_LCK		0x03		/* master lock configuration */

/********************** DEFINES for Hymod Flash ******************************/

/*
 * this code requires that the flash on any Hymod board appear as a bank
 * of two (identical) 16bit Intel StrataFlash chips with 64Kword erase
 * sectors (or blocks), running in x16 bit mode and connected side-by-side
 * to make a 32-bit wide bus.
 */

typedef unsigned long bank_word_t;
typedef bank_word_t bank_blk_t[64 * 1024];

#define BANK_FILL_WORD(b)	(((bank_word_t)(b) << 16) | (bank_word_t)(b))

#ifdef EXAMPLE

/* theoretically the following examples should also work */

/* one flash chip in x8 mode with 128Kword sectors and 8bit bus */
typedef unsigned char bank_word_t;
typedef bank_word_t bank_blk_t[128 * 1024];
#define BANK_FILL_WORD(b)	((bank_word_t)(b))

/* four flash chips in x16 mode with 32Kword sectors and 64bit bus */
typedef unsigned long long bank_word_t;
typedef bank_word_t bank_blk_t[32 * 1024];
#define BANK_FILL_WORD(b)	( \
				    ((bank_word_t)(b) << 48) \
				    ((bank_word_t)(b) << 32) \
				    ((bank_word_t)(b) << 16) \
				    ((bank_word_t)(b) <<  0) \
				)

#endif /* EXAMPLE */

/* the sizes of these two types should probably be the same */
typedef bank_word_t *bank_addr_t;
typedef unsigned long bank_size_t;

/* align bank addresses and sizes to bank word boundaries */
#define BANK_ADDR_WORD_ALIGN(a)	((bank_addr_t)((bank_size_t)(a) \
				    & ~(sizeof (bank_word_t) - 1)))
#define BANK_SIZE_WORD_ALIGN(s)	(((bank_size_t)(s) + sizeof (bank_word_t) - 1) \
				    & ~(sizeof (bank_word_t) - 1))

/* align bank addresses and sizes to bank block boundaries */
#define BANK_ADDR_BLK_ALIGN(a)	((bank_addr_t)((bank_size_t)(a) \
				    & ~(sizeof (bank_blk_t) - 1)))
#define BANK_SIZE_BLK_ALIGN(s)	(((bank_size_t)(s) + sizeof (bank_blk_t) - 1) \
				    & ~(sizeof (bank_blk_t) - 1))

/* add an offset to a bank address */
#define BANK_ADDR_OFFSET(a, o)	((bank_addr_t)((bank_size_t)(a) + \
				    (bank_size_t)(o)))

/* adjust a bank address to start of next word, block or bank */
#define BANK_ADDR_NEXT_WORD(a)	BANK_ADDR_OFFSET(BANK_ADDR_WORD_ALIGN(a), \
				    sizeof (bank_word_t))
#define BANK_ADDR_NEXT_BLK(a)	BANK_ADDR_OFFSET(BANK_ADDR_BLK_ALIGN(a), \
				    sizeof (bank_blk_t))

/* get bank address of register r given a bank base address a and block num b */
#define BANK_ADDR_REG(a, b, r)	BANK_ADDR_OFFSET(BANK_ADDR_OFFSET((a), \
				    (bank_size_t)(b) * sizeof (bank_blk_t)), \
					(bank_size_t)(r) * sizeof (bank_word_t))

/* make a bank word value for each StrataFlash value */

/* Commands */
#define BANK_CMD_RST		BANK_FILL_WORD(ISF_CMD_RST)
#define BANK_CMD_RD_ID		BANK_FILL_WORD(ISF_CMD_RD_ID)
#define BANK_CMD_RD_STAT	BANK_FILL_WORD(ISF_CMD_RD_STAT)
#define BANK_CMD_CLR_STAT	BANK_FILL_WORD(ISF_CMD_CLR_STAT)
#define BANK_CMD_ERASE1		BANK_FILL_WORD(ISF_CMD_ERASE1)
#define BANK_CMD_ERASE2		BANK_FILL_WORD(ISF_CMD_ERASE2)
#define BANK_CMD_PROG		BANK_FILL_WORD(ISF_CMD_PROG)
#define BANK_CMD_LOCK		BANK_FILL_WORD(ISF_CMD_LOCK)
#define BANK_CMD_SET_LOCK_BLK	BANK_FILL_WORD(ISF_CMD_SET_LOCK_BLK)
#define BANK_CMD_SET_LOCK_MSTR	BANK_FILL_WORD(ISF_CMD_SET_LOCK_MSTR)
#define BANK_CMD_CLR_LOCK_BLK	BANK_FILL_WORD(ISF_CMD_CLR_LOCK_BLK)

/* status register bits */
#define BANK_STAT_DPS		BANK_FILL_WORD(ISF_STAT_DPS)
#define BANK_STAT_PSS		BANK_FILL_WORD(ISF_STAT_PSS)
#define BANK_STAT_VPPS		BANK_FILL_WORD(ISF_STAT_VPPS)
#define BANK_STAT_PSLBS		BANK_FILL_WORD(ISF_STAT_PSLBS)
#define BANK_STAT_ECLBS		BANK_FILL_WORD(ISF_STAT_ECLBS)
#define BANK_STAT_ESS		BANK_FILL_WORD(ISF_STAT_ESS)
#define BANK_STAT_RDY		BANK_FILL_WORD(ISF_STAT_RDY)

#define BANK_STAT_ERR		BANK_FILL_WORD(ISF_STAT_ERR)

/* make a bank register address for each StrataFlash register address */

#define BANK_REG_MAN_CODE(a)	BANK_ADDR_REG((a), 0, ISF_REG_MAN_CODE)
#define BANK_REG_DEV_CODE(a)	BANK_ADDR_REG((a), 0, ISF_REG_DEV_CODE)
#define BANK_REG_BLK_LCK(a, b)	BANK_ADDR_REG((a), (b), ISF_REG_BLK_LCK)
#define BANK_REG_MST_LCK(a)	BANK_ADDR_REG((a), 0, ISF_REG_MST_LCK)
