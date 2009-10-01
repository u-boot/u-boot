/*
 *  Copyright (C) 2005-2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  Kyungmin Park <kyungmin.park@samsung.com>
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

#ifndef __SAMSUNG_ONENAND_H__
#define __SAMSUNG_ONENAND_H__

/*
 * OneNAND Controller
 */

#ifndef __ASSEMBLY__
struct samsung_onenand {
	unsigned long	mem_cfg;	/* 0x0000 */
	unsigned char	res1[0xc];
	unsigned long	burst_len;	/* 0x0010 */
	unsigned char	res2[0xc];
	unsigned long	mem_reset;	/* 0x0020 */
	unsigned char	res3[0xc];
	unsigned long	int_err_stat;	/* 0x0030 */
	unsigned char	res4[0xc];
	unsigned long	int_err_mask;	/* 0x0040 */
	unsigned char	res5[0xc];
	unsigned long	int_err_ack;	/* 0x0050 */
	unsigned char	res6[0xc];
	unsigned long	ecc_err_stat;	/* 0x0060 */
	unsigned char	res7[0xc];
	unsigned long	manufact_id;	/* 0x0070 */
	unsigned char	res8[0xc];
	unsigned long	device_id;	/* 0x0080 */
	unsigned char	res9[0xc];
	unsigned long	data_buf_size;	/* 0x0090 */
	unsigned char	res10[0xc];
	unsigned long	boot_buf_size;	/* 0x00A0 */
	unsigned char	res11[0xc];
	unsigned long	buf_amount;	/* 0x00B0 */
	unsigned char	res12[0xc];
	unsigned long	tech;		/* 0x00C0 */
	unsigned char	res13[0xc];
	unsigned long	fba;		/* 0x00D0 */
	unsigned char	res14[0xc];
	unsigned long	fpa;		/* 0x00E0 */
	unsigned char	res15[0xc];
	unsigned long	fsa;		/* 0x00F0 */
	unsigned char	res16[0x3c];
	unsigned long	sync_mode;	/* 0x0130 */
	unsigned char	res17[0xc];
	unsigned long	trans_spare;	/* 0x0140 */
	unsigned char	res18[0x3c];
	unsigned long	err_page_addr;	/* 0x0180 */
	unsigned char	res19[0x1c];
	unsigned long	int_pin_en;	/* 0x01A0 */
	unsigned char	res20[0x1c];
	unsigned long	acc_clock;	/* 0x01C0 */
	unsigned char	res21[0x1c];
	unsigned long	err_blk_addr;	/* 0x01E0 */
	unsigned char	res22[0xc];
	unsigned long	flash_ver_id;	/* 0x01F0 */
	unsigned char	res23[0x6c];
	unsigned long	watchdog_cnt_low;	/* 0x0260 */
	unsigned char	res24[0xc];
	unsigned long	watchdog_cnt_hi;	/* 0x0270 */
	unsigned char	res25[0xc];
	unsigned long	sync_write;	/* 0x0280 */
	unsigned char	res26[0x1c];
	unsigned long	cold_reset;	/* 0x02A0 */
	unsigned char	res27[0xc];
	unsigned long	ddp_device;	/* 0x02B0 */
	unsigned char	res28[0xc];
	unsigned long	multi_plane;	/* 0x02C0 */
	unsigned char	res29[0x1c];
	unsigned long	trans_mode;	/* 0x02E0 */
	unsigned char	res30[0x1c];
	unsigned long	ecc_err_stat2;	/* 0x0300 */
	unsigned char	res31[0xc];
	unsigned long	ecc_err_stat3;	/* 0x0310 */
	unsigned char	res32[0xc];
	unsigned long	ecc_err_stat4;	/* 0x0320 */
	unsigned char	res33[0x1c];
	unsigned long	dev_page_size;	/* 0x0340 */
	unsigned char	res34[0x4c];
	unsigned long	int_mon_status;	/* 0x0390 */
};
#endif

#define ONENAND_MEM_RESET_HOT	0x3
#define ONENAND_MEM_RESET_COLD	0x2
#define ONENAND_MEM_RESET_WARM	0x1

#define INT_ERR_ALL	0x3fff
#define CACHE_OP_ERR    (1 << 13)
#define RST_CMP         (1 << 12)
#define RDY_ACT         (1 << 11)
#define INT_ACT         (1 << 10)
#define UNSUP_CMD       (1 << 9)
#define LOCKED_BLK      (1 << 8)
#define BLK_RW_CMP      (1 << 7)
#define ERS_CMP         (1 << 6)
#define PGM_CMP         (1 << 5)
#define LOAD_CMP        (1 << 4)
#define ERS_FAIL        (1 << 3)
#define PGM_FAIL        (1 << 2)
#define INT_TO          (1 << 1)
#define LD_FAIL_ECC_ERR (1 << 0)

#define TSRF		(1 << 0)

/* common initialize function */
extern void s3c_onenand_init(struct mtd_info *);

#endif
