/*
 * Copyright 2008,2010 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _MMC_H_
#define _MMC_H_

#include <linux/list.h>
#include <linux/compiler.h>

#define SD_VERSION_SD	0x20000
#define SD_VERSION_2	(SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0	(SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10	(SD_VERSION_SD | 0x1a)
#define MMC_VERSION_MMC		0x10000
#define MMC_VERSION_UNKNOWN	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2		(MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4		(MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2		(MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3		(MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4		(MMC_VERSION_MMC | 0x40)

#define MMC_MODE_HS		0x001
#define MMC_MODE_HS_52MHz	0x010
#define MMC_MODE_4BIT		0x100
#define MMC_MODE_8BIT		0x200
#define MMC_MODE_SPI		0x400
#define MMC_MODE_HC		0x800

#define MMC_MODE_MASK_WIDTH_BITS (MMC_MODE_4BIT | MMC_MODE_8BIT)
#define MMC_MODE_WIDTH_BITS_SHIFT 8

#define SD_DATA_4BIT	0x00040000

#define IS_SD(x) (x->version & SD_VERSION_SD)

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR		-16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17 /* Unusable Card */
#define COMM_ERR		-18 /* Communications Error */
#define TIMEOUT			-19

#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59

#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY	0x00020000
#define SD_HIGHSPEED_SUPPORTED	0x00020000

#define MMC_HS_TIMING		0x00000100
#define MMC_HS_52MHZ		0x2

#define OCR_BUSY		0x80000000
#define OCR_HCS			0x40000000
#define OCR_VOLTAGE_MASK	0x007FFF80
#define OCR_ACCESS_MODE		0x60000000

#define SECURE_ERASE		0x80000000

#define MMC_STATUS_MASK		(~0x0206BF7F)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE	(0xf << 9)
#define MMC_STATUS_ERROR	(1 << 19)

#define MMC_STATE_PRG		(7 << 9)

#define MMC_VDD_165_195		0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000	/* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET		0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01 /* Set bits in EXT_CSD byte
						addressed by index which are
						1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
						addressed by index, which are
						1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_PART_CONF		179	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

#define EXT_CSD_CARD_TYPE_26	(1 << 0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1 << 1)	/* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */

#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMCPART_NOAVAILABLE	(0xff)
#define PART_ACCESS_MASK	(0x7)
#define PART_SUPPORT		(0x1)

struct mmc_cid {
	unsigned long psn;
	unsigned short oid;
	unsigned char mid;
	unsigned char prv;
	unsigned char mdt;
	char pnm[7];
};

struct mmc_cmd {
	ushort cmdidx;
	uint resp_type;
	uint cmdarg;
	uint response[4];
};

struct mmc_data {
	union {
		char *dest;
		const char *src; /* src buffers don't get written to */
	};
	uint flags;
	uint blocks;
	uint blocksize;
};

struct mmc {
	struct list_head link;
	char name[32];
	void *priv;
	uint voltages;
	uint version;
	uint has_init;
	uint f_min;
	uint f_max;
	int high_capacity;
	uint bus_width;
	uint clock;
	uint card_caps;
	uint host_caps;
	uint ocr;
	uint scr[2];
	uint csd[4];
	uint cid[4];
	ushort rca;
	char part_config;
	char part_num;
	uint tran_speed;
	uint read_bl_len;
	uint write_bl_len;
	uint erase_grp_size;
	u64 capacity;
	block_dev_desc_t block_dev;
	int (*send_cmd)(struct mmc *mmc,
			struct mmc_cmd *cmd, struct mmc_data *data);
	void (*set_ios)(struct mmc *mmc);
	int (*init)(struct mmc *mmc);
	int (*getcd)(struct mmc *mmc);
	uint b_max;
};

int mmc_register(struct mmc *mmc);
int mmc_initialize(bd_t *bis);
int mmc_init(struct mmc *mmc);
int mmc_read(struct mmc *mmc, u64 src, uchar *dst, int size);
void mmc_set_clock(struct mmc *mmc, uint clock);
struct mmc *find_mmc_device(int dev_num);
int mmc_set_dev(int dev_num);
void print_mmc_devices(char separator);
int get_mmc_num(void);
int board_mmc_getcd(struct mmc *mmc);
int mmc_switch_part(int dev_num, unsigned int part_num);
int mmc_getcd(struct mmc *mmc);
void spl_mmc_load(void) __noreturn;

#ifdef CONFIG_GENERIC_MMC
#define mmc_host_is_spi(mmc)	((mmc)->host_caps & MMC_MODE_SPI)
struct mmc *mmc_spi_init(uint bus, uint cs, uint speed, uint mode);
#else
int mmc_legacy_init(int verbose);
#endif

#endif /* _MMC_H_ */
