/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Hacked for the marvell db64360 eval board by
 * Ingo Assmus <ingo.assmus@keymile.com>
 */

/*************** DEFINES for Intel StrataFlash FLASH chip ********************/

/*
 * acceptable chips types are:
 *
 *	28F320J5, 28F640J5, 28F320J3A, 28F640J3A and 28F128J3A
 */

/* register addresses, valid only following an CHIP_CMD_RD_ID command */
#define CHIP_ADDR_REG_MAN	0x000000	/* manufacturer's id */
#define CHIP_ADDR_REG_DEV	0x000001	/* device id */
#define CHIP_ADDR_REG_CFGM	0x000003	/* master lock config */
#define CHIP_ADDR_REG_CFG(b)	(((b)<<16)|2)	/* lock config for block b */

/* Commands */
#define CHIP_CMD_RST		0xFF		/* reset flash */
#define CHIP_CMD_RD_ID		0x90		/* read the id and lock bits */
#define CHIP_CMD_RD_QUERY	0x98		/* read device capabilities */
#define CHIP_CMD_RD_STAT	0x70		/* read the status register */
#define CHIP_CMD_CLR_STAT	0x50		/* clear the staus register */
#define CHIP_CMD_WR_BUF		0xE8		/* clear the staus register */
#define CHIP_CMD_PROG		0x40		/* program word command */
#define CHIP_CMD_ERASE1		0x20		/* 1st word for block erase */
#define CHIP_CMD_ERASE2		0xD0		/* 2nd word for block erase */
#define CHIP_CMD_ERASE_SUSP	0xB0		/* suspend block erase */
#define CHIP_CMD_LOCK		0x60		/* 1st word for all lock cmds */
#define CHIP_CMD_SET_LOCK_BLK	0x01		/* 2nd wrd set block lock bit */
#define CHIP_CMD_SET_LOCK_MSTR	0xF1		/* 2nd wrd set master lck bit */
#define CHIP_CMD_CLR_LOCK_BLK	0xD0		/* 2nd wrd clear blk lck bit */

/* status register bits */
#define CHIP_STAT_DPS		0x02		/* Device Protect Status */
#define CHIP_STAT_VPPS		0x08		/* VPP Status */
#define CHIP_STAT_PSLBS		0x10		/* Program+Set Lock Bit Stat */
#define CHIP_STAT_ECLBS		0x20		/* Erase+Clr Lock Bit Stat */
#define CHIP_STAT_ESS		0x40		/* Erase Suspend Status */
#define CHIP_STAT_RDY		0x80		/* WSM Mach Status, 1=rdy */

#define CHIP_STAT_ERR		(CHIP_STAT_VPPS | CHIP_STAT_DPS | \
				    CHIP_STAT_ECLBS | CHIP_STAT_PSLBS)

/* ID and Lock Configuration */
#define CHIP_RD_ID_LOCK		0x01		/* Bit 0 of each byte */
#define CHIP_RD_ID_MAN		0x89		/* Manufacturer code = 0x89 */
#define CHIP_RD_ID_DEV		CONFIG_SYS_FLASH_ID

/* dimensions */
#define CHIP_WIDTH		2		/* chips are in 16 bit mode */
#define CHIP_WSHIFT		1		/* (log2 of CHIP_WIDTH) */
#define CHIP_NBLOCKS		128
#define CHIP_BLKSZ		(128 * 1024)	/* of 128Kbytes each */
#define CHIP_SIZE		(CHIP_BLKSZ * CHIP_NBLOCKS)

/********************** DEFINES for Hymod Flash ******************************/

/*
 * The hymod board has 2 x 28F320J5 chips running in
 * 16 bit mode, for a 32 bit wide bank.
 */

typedef unsigned short bank_word_t;		/* 8/16/32/64bit unsigned int */
typedef volatile bank_word_t *bank_addr_t;
typedef unsigned long bank_size_t;		/* want this big - >= 32 bit */

#define BANK_CHIP_WIDTH		1		/* each bank is 1 chip wide */
#define BANK_CHIP_WSHIFT	0		/* (log2 of BANK_CHIP_WIDTH) */

#define BANK_WIDTH		(CHIP_WIDTH * BANK_CHIP_WIDTH)
#define BANK_WSHIFT		(CHIP_WSHIFT + BANK_CHIP_WSHIFT)
#define BANK_NBLOCKS		CHIP_NBLOCKS
#define BANK_BLKSZ		(CHIP_BLKSZ * BANK_CHIP_WIDTH)
#define BANK_SIZE		(CHIP_SIZE * BANK_CHIP_WIDTH)

#define MAX_BANKS		1		/* only one bank possible */

/* align bank addresses and sizes to bank word boundaries */
#define BANK_ADDR_WORD_ALIGN(a)	((bank_addr_t)((bank_size_t)(a) \
				    & ~(BANK_WIDTH - 1)))
#define BANK_SIZE_WORD_ALIGN(s)	((bank_size_t)BANK_ADDR_WORD_ALIGN( \
				    (bank_size_t)(s) + (BANK_WIDTH - 1)))

/* align bank addresses and sizes to bank block boundaries */
#define BANK_ADDR_BLK_ALIGN(a)	((bank_addr_t)((bank_size_t)(a) \
				    & ~(BANK_BLKSZ - 1)))
#define BANK_SIZE_BLK_ALIGN(s)	((bank_size_t)BANK_ADDR_BLK_ALIGN( \
				    (bank_size_t)(s) + (BANK_BLKSZ - 1)))

/* align bank addresses and sizes to bank boundaries */
#define BANK_ADDR_BANK_ALIGN(a)	((bank_addr_t)((bank_size_t)(a) \
				    & ~(BANK_SIZE - 1)))
#define BANK_SIZE_BANK_ALIGN(s)	((bank_size_t)BANK_ADDR_BANK_ALIGN( \
				    (bank_size_t)(s) + (BANK_SIZE - 1)))

/* add an offset to a bank address */
#define BANK_ADDR_OFFSET(a, o)	(bank_addr_t)((bank_size_t)(a) + \
				    (bank_size_t)(o))

/* get base address of bank b, given flash base address a */
#define BANK_ADDR_BASE(a, b)	BANK_ADDR_OFFSET(BANK_ADDR_BANK_ALIGN(a), \
				    (bank_size_t)(b) * BANK_SIZE)

/* adjust a bank address to start of next word, block or bank */
#define BANK_ADDR_NEXT_WORD(a)	BANK_ADDR_OFFSET(BANK_ADDR_WORD_ALIGN(a), \
				    BANK_WIDTH)
#define BANK_ADDR_NEXT_BLK(a)	BANK_ADDR_OFFSET(BANK_ADDR_BLK_ALIGN(a), \
				    BANK_BLKSZ)
#define BANK_ADDR_NEXT_BANK(a)	BANK_ADDR_OFFSET(BANK_ADDR_BANK_ALIGN(a), \
				    BANK_SIZE)

/* get bank address of chip register r given a bank base address a */
#define BANK_ADDR_REG(a, r)	BANK_ADDR_OFFSET(BANK_ADDR_BANK_ALIGN(a), \
				    ((bank_size_t)(r) << BANK_WSHIFT))

/* make a bank address for each chip register address */

#define BANK_ADDR_REG_MAN(a)	BANK_ADDR_REG((a), CHIP_ADDR_REG_MAN)
#define BANK_ADDR_REG_DEV(a)	BANK_ADDR_REG((a), CHIP_ADDR_REG_DEV)
#define BANK_ADDR_REG_CFGM(a)	BANK_ADDR_REG((a), CHIP_ADDR_REG_CFGM)
#define BANK_ADDR_REG_CFG(b,a)	BANK_ADDR_REG((a), CHIP_ADDR_REG_CFG(b))

/*
 * replicate a chip cmd/stat/rd value into each byte position within a word
 * so that multiple chips are accessed in a single word i/o operation
 *
 * this must be as wide as the bank_word_t type, and take into account the
 * chip width and bank layout
 */

#define BANK_FILL_WORD(o)	((bank_word_t)(o))

/* make a bank word value for each chip cmd/stat/rd value */

/* Commands */
#define BANK_CMD_RST		BANK_FILL_WORD(CHIP_CMD_RST)
#define BANK_CMD_RD_ID		BANK_FILL_WORD(CHIP_CMD_RD_ID)
#define BANK_CMD_RD_STAT	BANK_FILL_WORD(CHIP_CMD_RD_STAT)
#define BANK_CMD_CLR_STAT	BANK_FILL_WORD(CHIP_CMD_CLR_STAT)
#define BANK_CMD_ERASE1		BANK_FILL_WORD(CHIP_CMD_ERASE1)
#define BANK_CMD_ERASE2		BANK_FILL_WORD(CHIP_CMD_ERASE2)
#define BANK_CMD_PROG		BANK_FILL_WORD(CHIP_CMD_PROG)
#define BANK_CMD_LOCK		BANK_FILL_WORD(CHIP_CMD_LOCK)
#define BANK_CMD_SET_LOCK_BLK	BANK_FILL_WORD(CHIP_CMD_SET_LOCK_BLK)
#define BANK_CMD_SET_LOCK_MSTR	BANK_FILL_WORD(CHIP_CMD_SET_LOCK_MSTR)
#define BANK_CMD_CLR_LOCK_BLK	BANK_FILL_WORD(CHIP_CMD_CLR_LOCK_BLK)

/* status register bits */
#define BANK_STAT_DPS		BANK_FILL_WORD(CHIP_STAT_DPS)
#define BANK_STAT_PSS		BANK_FILL_WORD(CHIP_STAT_PSS)
#define BANK_STAT_VPPS		BANK_FILL_WORD(CHIP_STAT_VPPS)
#define BANK_STAT_PSLBS		BANK_FILL_WORD(CHIP_STAT_PSLBS)
#define BANK_STAT_ECLBS		BANK_FILL_WORD(CHIP_STAT_ECLBS)
#define BANK_STAT_ESS		BANK_FILL_WORD(CHIP_STAT_ESS)
#define BANK_STAT_RDY		BANK_FILL_WORD(CHIP_STAT_RDY)

#define BANK_STAT_ERR		BANK_FILL_WORD(CHIP_STAT_ERR)

/* ID and Lock Configuration */
#define BANK_RD_ID_LOCK		BANK_FILL_WORD(CHIP_RD_ID_LOCK)
#define BANK_RD_ID_MAN		BANK_FILL_WORD(CHIP_RD_ID_MAN)
#define BANK_RD_ID_DEV		BANK_FILL_WORD(CHIP_RD_ID_DEV)
