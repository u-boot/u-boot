/**************** DEFINES for Intel 28F008S5 FLASH chip **********************/

/* register addresses, valid only following a I8S5_CMD_RD_ID command */
#define I8S5_ADDR_MAN	0x00000	/* manufacturer's id */
#define I8S5_ADDR_DEV	0x00001	/* device id */
#define I8S5_ADDR_CFGM	0x00003	/* master lock configuration */
#define I8S5_ADDR_CFG(b) (((b)<<16)|2) /* block lock configuration */

/* Commands */
#define I8S5_CMD_RST	0xFF	/* reset flash */
#define I8S5_CMD_RD_ID	0x90	/* read the id and lock bits */
#define I8S5_CMD_RD_STAT 0x70	/* read the status register */
#define I8S5_CMD_CLR_STAT 0x50	/* clear the staus register */
#define I8S5_CMD_ERASE1	0x20	/* first word for block erase */
#define I8S5_CMD_ERASE2	0xD0	/* second word for block erase */
#define I8S5_CMD_PROG	0x40	/* program word command */
#define I8S5_CMD_LOCK	0x60	/* first word for all lock commands */
#define I8S5_CMD_SET_LOCK_BLK 0x01 /* 2nd word for set block lock bit */
#define I8S5_CMD_SET_LOCK_MSTR 0xF1 /* 2nd word for set master lock bit */
#define I8S5_CMD_CLR_LOCK_BLK 0xD0 /* 2nd word for clear block lock bit */

/* status register bits */
#define I8S5_STAT_DPS	0x02	/* Device Protect Status */
#define I8S5_STAT_PSS	0x04	/* Program Suspend Status */
#define I8S5_STAT_VPPS	0x08	/* VPP Status */
#define I8S5_STAT_PSLBS	0x10	/* Program and Set Lock Bit Status */
#define I8S5_STAT_ECLBS	0x20	/* Erase and Clear Lock Bit Status */
#define I8S5_STAT_ESS	0x40	/* Erase Suspend Status */
#define I8S5_STAT_RDY	0x80	/* Write State Machine Status, 1=rdy */

#define I8S5_STAT_ERR	(I8S5_STAT_VPPS | I8S5_STAT_DPS | \
			    I8S5_STAT_ECLBS | I8S5_STAT_PSLBS)

/* ID and Lock Configuration */
#define I8S5_RD_ID_LOCK	0x01	/* Bit 0 of each byte */
#define I8S5_RD_ID_MAN	0x89	/* Manufacturer code = 0x89 */
#define I8S5_RD_ID_DEV	0xA6	/* Device code = 0xA6, 28F008S5 */

/* dimensions */
#define I8S5_NBLOCKS	16		/* a 28F008S5 consists of 16 blocks */
#define I8S5_BLKSZ	(64*1024)	/* of 64Kbyte each */
#define I8S5_SIZE	(I8S5_BLKSZ * I8S5_NBLOCKS)

/**************** DEFINES for Intel 28F800B5 FLASH chip **********************/

/* register addresses, valid only following a I8S5_CMD_RD_ID command */
#define I8B5_ADDR_MAN	0x00000	/* manufacturer's id */
#define I8B5_ADDR_DEV	0x00001	/* device id */

/* Commands */
#define I8B5_CMD_RST	0xFF	/* reset flash */
#define I8B5_CMD_RD_ID	0x90	/* read the id and lock bits */
#define I8B5_CMD_RD_STAT 0x70	/* read the status register */
#define I8B5_CMD_CLR_STAT 0x50	/* clear the staus register */
#define I8B5_CMD_ERASE1	0x20	/* first word for block erase */
#define I8B5_CMD_ERASE2	0xD0	/* second word for block erase */
#define I8B5_CMD_PROG	0x40	/* program word command */

/* status register bits */
#define I8B5_STAT_VPPS	0x08	/* VPP Status */
#define I8B5_STAT_DWS	0x10	/* Program and Set Lock Bit Status */
#define I8B5_STAT_ES	0x20	/* Erase and Clear Lock Bit Status */
#define I8B5_STAT_ESS	0x40	/* Erase Suspend Status */
#define I8B5_STAT_RDY	0x80	/* Write State Machine Status, 1=rdy */

#define I8B5_STAT_ERR	(I8B5_STAT_VPPS | I8B5_STAT_DWS | I8B5_STAT_ES)

/* ID Configuration */
#define I8B5_RD_ID_MAN	0x89	/* Manufacturer code = 0x89 */
#define I8B5_RD_ID_DEV1	0x889D	/* Device code = 0x889D, 28F800B5 */

/* dimensions */
#define I8B5_NBLOCKS	8		/* a 28F008S5 consists of 16 blocks */
#define I8B5_BLKSZ	(128*1024)	/* of 64Kbyte each */
#define I8B5_SIZE	(I8B5_BLKSZ * I8B5_NBLOCKS)

/****************** DEFINES for Cogent CMA302 Flash **************************/

/*
 * Quoted from the CMA302 manual:
 *
 * Although the CMA302 supports 64-bit reads, all writes must be done with
 * word size only. When programming the CMA302, the FLASH devices appear as 2
 * banks of interleaved, 32-bit wide FLASH. Each 32-bit word consists of four
 * 28F008S5 devices. The first bank is accessed when the word address is even,
 * while the second bank is accessed when the word address is odd. This must
 * be taken into account when programming the desired word. Also, when locking
 * blocks, software must lock both banks. The CMA302 does not directly support
 * byte writing.  Programming and/or erasing individual bytes is done with
 * selective use of the Write Command.  By not placing the Write Command value
 * on a particular byte lane, that byte will not be written with the following
 * Write Data. Also, remember that within a byte lane (i.e. D0-7), there are
 * two 28F008S5 devices, one for each bank or every other word.
 *
 * End quote.
 *
 * Each 28F008S5 is 8Mbit, with 8 bit wide data. i.e. each is 1Mbyte. The
 * chips are arranged on the CMA302 in multiples of two banks, each bank having
 * 4 chips. Each bank must be accessed as a single 32 bit wide device (i.e.
 * aligned on a 32 bit boundary), with each byte lane within the 32 bits (0-3)
 * going to each of the 4 chips and the word address selecting the bank, even
 * being the low bank and odd the high bank. For 64bit reads, both banks are
 * read simultaneously with the second bank on byte lanes 4-7. Each 28F008S5
 * consists of 16 64Kbyte "block"s. Before programming a byte, the block that
 * the byte resides within must be erased. So if you want to program contiguous
 * memory locations, you must erase all 8 chips at the same time. i.e. the
 * flash on the CMA302 can be viewed as a number of 512Kbyte blocks.
 *
 * Note: I am going to treat banks as 8 Mbytes (1Meg of 64bit words), whereas
 * the example code treats them as a pair of interleaved 1 Mbyte x 32bit banks.
 */

typedef unsigned long c302f_word_t;	/* 32 or 64 bit unsigned integer */
typedef volatile c302f_word_t *c302f_addr_t;
typedef unsigned long c302f_size_t;	/* want this big - at least 32 bit */

/* layout of banks on cma302 board */
#define C302F_BNK_WIDTH		8	/* each bank is 8 chips wide */
#define C302F_BNK_WSHIFT	3	/* log base 2 of C302F_BNK_WIDTH */
#define C302F_BNK_NBLOCKS	I8S5_NBLOCKS
#define C302F_BNK_BLKSZ		(I8S5_BLKSZ * C302F_BNK_WIDTH)
#define C302F_BNK_SIZE		(I8S5_SIZE * C302F_BNK_WIDTH)

#define C302F_MAX_BANKS		2	/* up to 2 banks (8M each) on CMA302 */

/* align addresses and sizes to bank boundaries */
#define C302F_BNK_ADDR_ALIGN(a)	((c302f_addr_t)((c302f_size_t)(a) \
				    & ~(C302F_BNK_WIDTH - 1)))
#define C302F_BNK_SIZE_ALIGN(s)	((c302f_size_t)C302F_BNK_ADDR_ALIGN( \
				    (c302f_size_t)(s) + (C302F_BNK_WIDTH - 1)))

/* align addresses and sizes to block boundaries */
#define C302F_BLK_ADDR_ALIGN(a)	((c302f_addr_t)((c302f_size_t)(a) \
				    & ~(C302F_BNK_BLKSZ - 1)))
#define C302F_BLK_SIZE_ALIGN(s)	((c302f_size_t)C302F_BLK_ADDR_ALIGN( \
				    (c302f_size_t)(s) + (C302F_BNK_BLKSZ - 1)))

/* add a byte offset to a flash address */
#define C302F_ADDR_ADD_BYTEOFF(a,o) \
				(c302f_addr_t)((c302f_size_t)(a) + (o))

/* get base address of bank b, given flash base address a */
#define C302F_BNK_ADDR_BASE(a,b) \
				C302F_ADDR_ADD_BYTEOFF((a), \
				    (c302f_size_t)(b) * C302F_BNK_SIZE)

/* adjust an address a (within a bank) to next word, block or bank */
#define C302F_BNK_ADDR_NEXT_WORD(a) \
				C302F_ADDR_ADD_BYTEOFF((a), C302F_BNK_WIDTH)
#define C302F_BNK_ADDR_NEXT_BLK(a) \
				C302F_ADDR_ADD_BYTEOFF((a), C302F_BNK_BLKSZ)
#define C302F_BNK_ADDR_NEXT_BNK(a) \
				C302F_ADDR_ADD_BYTEOFF((a), C302F_BNK_SIZE)

/* get bank address of chip register r given a bank base address a */
#define C302F_BNK_ADDR_I8S5REG(a,r) \
				C302F_ADDR_ADD_BYTEOFF((a), \
				    (r) << C302F_BNK_WSHIFT)

/* make a bank representation for each chip address */

#define C302F_BNK_ADDR_MAN(a)	C302F_BNK_ADDR_I8S5REG((a), I8S5_ADDR_MAN)
#define C302F_BNK_ADDR_DEV(a)	C302F_BNK_ADDR_I8S5REG((a), I8S5_ADDR_DEV)
#define C302F_BNK_ADDR_CFGM(a)	C302F_BNK_ADDR_I8S5REG((a), I8S5_ADDR_CFGM)
#define C302F_BNK_ADDR_CFG(b,a)	C302F_BNK_ADDR_I8S5REG((a), I8S5_ADDR_CFG(b))

/*
 * replicate a chip cmd/stat/rd value into each byte position within a word
 * so that multiple chips are accessed in a single word i/o operation
 *
 * this must be as wide as the c302f_word_t type
 */
#define C302F_FILL_WORD(o)	(((unsigned long)(o) << 24) | \
				    ((unsigned long)(o) << 16) | \
				    ((unsigned long)(o) << 8) | \
				    (unsigned long)(o))

/* make a bank representation for each chip cmd/stat/rd value */

/* Commands */
#define C302F_BNK_CMD_RST	C302F_FILL_WORD(I8S5_CMD_RST)
#define C302F_BNK_CMD_RD_ID	C302F_FILL_WORD(I8S5_CMD_RD_ID)
#define C302F_BNK_CMD_RD_STAT	C302F_FILL_WORD(I8S5_CMD_RD_STAT)
#define C302F_BNK_CMD_CLR_STAT	C302F_FILL_WORD(I8S5_CMD_CLR_STAT)
#define C302F_BNK_CMD_ERASE1	C302F_FILL_WORD(I8S5_CMD_ERASE1)
#define C302F_BNK_CMD_ERASE2	C302F_FILL_WORD(I8S5_CMD_ERASE2)
#define C302F_BNK_CMD_PROG	C302F_FILL_WORD(I8S5_CMD_PROG)
#define C302F_BNK_CMD_LOCK	C302F_FILL_WORD(I8S5_CMD_LOCK)
#define C302F_BNK_CMD_SET_LOCK_BLK C302F_FILL_WORD(I8S5_CMD_SET_LOCK_BLK)
#define C302F_BNK_CMD_SET_LOCK_MSTR C302F_FILL_WORD(I8S5_CMD_SET_LOCK_MSTR)
#define C302F_BNK_CMD_CLR_LOCK_BLK C302F_FILL_WORD(I8S5_CMD_CLR_LOCK_BLK)

/* status register bits */
#define C302F_BNK_STAT_DPS	C302F_FILL_WORD(I8S5_STAT_DPS)
#define C302F_BNK_STAT_PSS	C302F_FILL_WORD(I8S5_STAT_PSS)
#define C302F_BNK_STAT_VPPS	C302F_FILL_WORD(I8S5_STAT_VPPS)
#define C302F_BNK_STAT_PSLBS	C302F_FILL_WORD(I8S5_STAT_PSLBS)
#define C302F_BNK_STAT_ECLBS	C302F_FILL_WORD(I8S5_STAT_ECLBS)
#define C302F_BNK_STAT_ESS	C302F_FILL_WORD(I8S5_STAT_ESS)
#define C302F_BNK_STAT_RDY	C302F_FILL_WORD(I8S5_STAT_RDY)

#define C302F_BNK_STAT_ERR	C302F_FILL_WORD(I8S5_STAT_ERR)

/* ID and Lock Configuration */
#define C302F_BNK_RD_ID_LOCK	C302F_FILL_WORD(I8S5_RD_ID_LOCK)
#define C302F_BNK_RD_ID_MAN	C302F_FILL_WORD(I8S5_RD_ID_MAN)
#define C302F_BNK_RD_ID_DEV	C302F_FILL_WORD(I8S5_RD_ID_DEV)

/*************** DEFINES for Cogent Motherboard Flash ************************/

typedef unsigned short cmbf_word_t;	/* 16 bit unsigned integer */
typedef volatile cmbf_word_t *cmbf_addr_t;
typedef unsigned long cmbf_size_t;	/* want this big - at least 32 bit */

/* layout of banks on cogent motherboard - only 1 bank, 16 bit wide */
#define CMBF_BNK_WIDTH		1	/* each bank is one chip wide */
#define CMBF_BNK_WSHIFT	0	/* log base 2 of CMBF_BNK_WIDTH */
#define CMBF_BNK_NBLOCKS	I8B5_NBLOCKS
#define CMBF_BNK_BLKSZ		(I8B5_BLKSZ * CMBF_BNK_WIDTH)
#define CMBF_BNK_SIZE		(I8B5_SIZE * CMBF_BNK_WIDTH)

#define CMBF_MAX_BANKS		1	/* only 1 x 1Mbyte bank on cogent m/b */

/* align addresses and sizes to bank boundaries */
#define CMBF_BNK_ADDR_ALIGN(a)	((c302f_addr_t)((c302f_size_t)(a) \
				    & ~(CMBF_BNK_WIDTH - 1)))
#define CMBF_BNK_SIZE_ALIGN(s)	((c302f_size_t)CMBF_BNK_ADDR_ALIGN( \
				    (c302f_size_t)(s) + (CMBF_BNK_WIDTH - 1)))

/* align addresses and sizes to block boundaries */
#define CMBF_BLK_ADDR_ALIGN(a)	((c302f_addr_t)((c302f_size_t)(a) \
				    & ~(CMBF_BNK_BLKSZ - 1)))
#define CMBF_BLK_SIZE_ALIGN(s)	((c302f_size_t)CMBF_BLK_ADDR_ALIGN( \
				    (c302f_size_t)(s) + (CMBF_BNK_BLKSZ - 1)))

/* add a byte offset to a flash address */
#define CMBF_ADDR_ADD_BYTEOFF(a,o) \
				(c302f_addr_t)((c302f_size_t)(a) + (o))

/* get base address of bank b, given flash base address a */
#define CMBF_BNK_ADDR_BASE(a,b) \
				CMBF_ADDR_ADD_BYTEOFF((a), \
				    (c302f_size_t)(b) * CMBF_BNK_SIZE)

/* adjust an address a (within a bank) to next word, block or bank */
#define CMBF_BNK_ADDR_NEXT_WORD(a) \
				CMBF_ADDR_ADD_BYTEOFF((a), CMBF_BNK_WIDTH)
#define CMBF_BNK_ADDR_NEXT_BLK(a) \
				CMBF_ADDR_ADD_BYTEOFF((a), CMBF_BNK_BLKSZ)
#define CMBF_BNK_ADDR_NEXT_BNK(a) \
				CMBF_ADDR_ADD_BYTEOFF((a), CMBF_BNK_SIZE)

/* get bank address of chip register r given a bank base address a */
#define CMBF_BNK_ADDR_I8B5REG(a,r) \
				CMBF_ADDR_ADD_BYTEOFF((a), \
				    (r) << CMBF_BNK_WSHIFT)

/* make a bank representation for each chip address */

#define CMBF_BNK_ADDR_MAN(a)	CMBF_BNK_ADDR_I8B5REG((a), I8B5_ADDR_MAN)
#define CMBF_BNK_ADDR_DEV(a)	CMBF_BNK_ADDR_I8B5REG((a), I8B5_ADDR_DEV)
#define CMBF_BNK_ADDR_CFGM(a)	CMBF_BNK_ADDR_I8B5REG((a), I8B5_ADDR_CFGM)
#define CMBF_BNK_ADDR_CFG(b,a)	CMBF_BNK_ADDR_I8B5REG((a), I8B5_ADDR_CFG(b))

/*
 * replicate a chip cmd/stat/rd value into each byte position within a word
 * so that multiple chips are accessed in a single word i/o operation
 *
 * this must be as wide as the c302f_word_t type
 */
#define CMBF_FILL_WORD(o)	(((unsigned long)(o) << 24) | \
				    ((unsigned long)(o) << 16) | \
				    ((unsigned long)(o) << 8) | \
				    (unsigned long)(o))

/* make a bank representation for each chip cmd/stat/rd value */

/* Commands */
#define CMBF_BNK_CMD_RST	CMBF_FILL_WORD(I8B5_CMD_RST)
#define CMBF_BNK_CMD_RD_ID	CMBF_FILL_WORD(I8B5_CMD_RD_ID)
#define CMBF_BNK_CMD_RD_STAT	CMBF_FILL_WORD(I8B5_CMD_RD_STAT)
#define CMBF_BNK_CMD_CLR_STAT	CMBF_FILL_WORD(I8B5_CMD_CLR_STAT)
#define CMBF_BNK_CMD_ERASE1	CMBF_FILL_WORD(I8B5_CMD_ERASE1)
#define CMBF_BNK_CMD_ERASE2	CMBF_FILL_WORD(I8B5_CMD_ERASE2)
#define CMBF_BNK_CMD_PROG	CMBF_FILL_WORD(I8B5_CMD_PROG)
#define CMBF_BNK_CMD_LOCK	CMBF_FILL_WORD(I8B5_CMD_LOCK)
#define CMBF_BNK_CMD_SET_LOCK_BLK CMBF_FILL_WORD(I8B5_CMD_SET_LOCK_BLK)
#define CMBF_BNK_CMD_SET_LOCK_MSTR CMBF_FILL_WORD(I8B5_CMD_SET_LOCK_MSTR)
#define CMBF_BNK_CMD_CLR_LOCK_BLK CMBF_FILL_WORD(I8B5_CMD_CLR_LOCK_BLK)

/* status register bits */
#define CMBF_BNK_STAT_DPS	CMBF_FILL_WORD(I8B5_STAT_DPS)
#define CMBF_BNK_STAT_PSS	CMBF_FILL_WORD(I8B5_STAT_PSS)
#define CMBF_BNK_STAT_VPPS	CMBF_FILL_WORD(I8B5_STAT_VPPS)
#define CMBF_BNK_STAT_PSLBS	CMBF_FILL_WORD(I8B5_STAT_PSLBS)
#define CMBF_BNK_STAT_ECLBS	CMBF_FILL_WORD(I8B5_STAT_ECLBS)
#define CMBF_BNK_STAT_ESS	CMBF_FILL_WORD(I8B5_STAT_ESS)
#define CMBF_BNK_STAT_RDY	CMBF_FILL_WORD(I8B5_STAT_RDY)

#define CMBF_BNK_STAT_ERR	CMBF_FILL_WORD(I8B5_STAT_ERR)

/* ID and Lock Configuration */
#define CMBF_BNK_RD_ID_LOCK	CMBF_FILL_WORD(I8B5_RD_ID_LOCK)
#define CMBF_BNK_RD_ID_MAN	CMBF_FILL_WORD(I8B5_RD_ID_MAN)
#define CMBF_BNK_RD_ID_DEV	CMBF_FILL_WORD(I8B5_RD_ID_DEV)
