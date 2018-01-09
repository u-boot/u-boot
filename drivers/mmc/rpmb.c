/*
 * Copyright 2014, Staubli Faverges
 * Pierre Aubert
 *
 * eMMC- Replay Protected Memory Block
 * According to JEDEC Standard No. 84-A441
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <memalign.h>
#include <mmc.h>
#include <u-boot/sha256.h>
#include "mmc_private.h"

/* Request codes */
#define RPMB_REQ_KEY		1
#define RPMB_REQ_WCOUNTER	2
#define RPMB_REQ_WRITE_DATA	3
#define RPMB_REQ_READ_DATA	4
#define RPMB_REQ_STATUS		5

/* Response code */
#define RPMB_RESP_KEY		0x0100
#define RPMB_RESP_WCOUNTER	0x0200
#define RPMB_RESP_WRITE_DATA	0x0300
#define RPMB_RESP_READ_DATA	0x0400

/* Error codes */
#define RPMB_OK			0
#define RPMB_ERR_GENERAL	1
#define RPMB_ERR_AUTH	2
#define RPMB_ERR_COUNTER	3
#define RPMB_ERR_ADDRESS	4
#define RPMB_ERR_WRITE		5
#define RPMB_ERR_READ		6
#define RPMB_ERR_KEY		7
#define RPMB_ERR_CNT_EXPIRED	0x80
#define RPMB_ERR_MSK		0x7

/* Sizes of RPMB data frame */
#define RPMB_SZ_STUFF		196
#define RPMB_SZ_MAC		32
#define RPMB_SZ_DATA		256
#define RPMB_SZ_NONCE		16

#define SHA256_BLOCK_SIZE	64

/* Error messages */
static const char * const rpmb_err_msg[] = {
	"",
	"General failure",
	"Authentication failure",
	"Counter failure",
	"Address failure",
	"Write failure",
	"Read failure",
	"Authentication key not yet programmed",
};


/* Structure of RPMB data frame. */
struct s_rpmb {
	unsigned char stuff[RPMB_SZ_STUFF];
	unsigned char mac[RPMB_SZ_MAC];
	unsigned char data[RPMB_SZ_DATA];
	unsigned char nonce[RPMB_SZ_NONCE];
	unsigned int write_counter;
	unsigned short address;
	unsigned short block_count;
	unsigned short result;
	unsigned short request;
};

static int mmc_set_blockcount(struct mmc *mmc, unsigned int blockcount,
			      bool is_rel_write)
{
	struct mmc_cmd cmd = {0};

	cmd.cmdidx = MMC_CMD_SET_BLOCK_COUNT;
	cmd.cmdarg = blockcount & 0x0000FFFF;
	if (is_rel_write)
		cmd.cmdarg |= 1 << 31;
	cmd.resp_type = MMC_RSP_R1;

	return mmc_send_cmd(mmc, &cmd, NULL);
}
static int mmc_rpmb_request(struct mmc *mmc, const struct s_rpmb *s,
			    unsigned int count, bool is_rel_write)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data;
	int ret;

	ret = mmc_set_blockcount(mmc, count, is_rel_write);
	if (ret) {
#ifdef CONFIG_MMC_RPMB_TRACE
		printf("%s:mmc_set_blockcount-> %d\n", __func__, ret);
#endif
		return 1;
	}

	cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1b;

	data.src = (const char *)s;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_WRITE;

	ret = mmc_send_cmd(mmc, &cmd, &data);
	if (ret) {
#ifdef CONFIG_MMC_RPMB_TRACE
		printf("%s:mmc_send_cmd-> %d\n", __func__, ret);
#endif
		return 1;
	}
	return 0;
}
static int mmc_rpmb_response(struct mmc *mmc, struct s_rpmb *s,
			     unsigned short expected)
{
	struct mmc_cmd cmd = {0};
	struct mmc_data data;
	int ret;

	ret = mmc_set_blockcount(mmc, 1, false);
	if (ret) {
#ifdef CONFIG_MMC_RPMB_TRACE
		printf("%s:mmc_set_blockcount-> %d\n", __func__, ret);
#endif
		return -1;
	}
	cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1;

	data.dest = (char *)s;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	ret = mmc_send_cmd(mmc, &cmd, &data);
	if (ret) {
#ifdef CONFIG_MMC_RPMB_TRACE
		printf("%s:mmc_send_cmd-> %d\n", __func__, ret);
#endif
		return -1;
	}
	/* Check the response and the status */
	if (be16_to_cpu(s->request) != expected) {
#ifdef CONFIG_MMC_RPMB_TRACE
		printf("%s:response= %x\n", __func__,
		       be16_to_cpu(s->request));
#endif
		return -1;
	}
	ret = be16_to_cpu(s->result);
	if (ret) {
		printf("%s %s\n", rpmb_err_msg[ret & RPMB_ERR_MSK],
		       (ret & RPMB_ERR_CNT_EXPIRED) ?
		       "Write counter has expired" : "");
	}

	/* Return the status of the command */
	return ret;
}
static int mmc_rpmb_status(struct mmc *mmc, unsigned short expected)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_STATUS);
	if (mmc_rpmb_request(mmc, rpmb_frame, 1, false))
		return -1;

	/* Read the result */
	return mmc_rpmb_response(mmc, rpmb_frame, expected);
}
static void rpmb_hmac(unsigned char *key, unsigned char *buff, int len,
		      unsigned char *output)
{
	sha256_context ctx;
	int i;
	unsigned char k_ipad[SHA256_BLOCK_SIZE];
	unsigned char k_opad[SHA256_BLOCK_SIZE];

	sha256_starts(&ctx);

	/* According to RFC 4634, the HMAC transform looks like:
	   SHA(K XOR opad, SHA(K XOR ipad, text))

	   where K is an n byte key.
	   ipad is the byte 0x36 repeated blocksize times
	   opad is the byte 0x5c repeated blocksize times
	   and text is the data being protected.
	*/

	for (i = 0; i < RPMB_SZ_MAC; i++) {
		k_ipad[i] = key[i] ^ 0x36;
		k_opad[i] = key[i] ^ 0x5c;
	}
	/* remaining pad bytes are '\0' XOR'd with ipad and opad values */
	for ( ; i < SHA256_BLOCK_SIZE; i++) {
		k_ipad[i] = 0x36;
		k_opad[i] = 0x5c;
	}
	sha256_update(&ctx, k_ipad, SHA256_BLOCK_SIZE);
	sha256_update(&ctx, buff, len);
	sha256_finish(&ctx, output);

	/* Init context for second pass */
	sha256_starts(&ctx);

	/* start with outer pad */
	sha256_update(&ctx, k_opad, SHA256_BLOCK_SIZE);

	/* then results of 1st hash */
	sha256_update(&ctx, output, RPMB_SZ_MAC);

	/* finish up 2nd pass */
	sha256_finish(&ctx, output);
}
int mmc_rpmb_get_counter(struct mmc *mmc, unsigned long *pcounter)
{
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);

	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_WCOUNTER);
	if (mmc_rpmb_request(mmc, rpmb_frame, 1, false))
		return -1;

	/* Read the result */
	ret = mmc_rpmb_response(mmc, rpmb_frame, RPMB_RESP_WCOUNTER);
	if (ret)
		return ret;

	*pcounter = be32_to_cpu(rpmb_frame->write_counter);
	return 0;
}
int mmc_rpmb_set_key(struct mmc *mmc, void *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	/* Fill the request */
	memset(rpmb_frame, 0, sizeof(struct s_rpmb));
	rpmb_frame->request = cpu_to_be16(RPMB_REQ_KEY);
	memcpy(rpmb_frame->mac, key, RPMB_SZ_MAC);

	if (mmc_rpmb_request(mmc, rpmb_frame, 1, true))
		return -1;

	/* read the operation status */
	return mmc_rpmb_status(mmc, RPMB_RESP_KEY);
}
int mmc_rpmb_read(struct mmc *mmc, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	int i;

	for (i = 0; i < cnt; i++) {
		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_READ_DATA);
		if (mmc_rpmb_request(mmc, rpmb_frame, 1, false))
			break;

		/* Read the result */
		if (mmc_rpmb_response(mmc, rpmb_frame, RPMB_RESP_READ_DATA))
			break;

		/* Check the HMAC if key is provided */
		if (key) {
			unsigned char ret_hmac[RPMB_SZ_MAC];

			rpmb_hmac(key, rpmb_frame->data, 284, ret_hmac);
			if (memcmp(ret_hmac, rpmb_frame->mac, RPMB_SZ_MAC)) {
				printf("MAC error on block #%d\n", i);
				break;
			}
		}
		/* Copy data */
		memcpy(addr + i * RPMB_SZ_DATA, rpmb_frame->data, RPMB_SZ_DATA);
	}
	return i;
}
int mmc_rpmb_write(struct mmc *mmc, void *addr, unsigned short blk,
		  unsigned short cnt, unsigned char *key)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct s_rpmb, rpmb_frame, 1);
	unsigned long wcount;
	int i;

	for (i = 0; i < cnt; i++) {
		if (mmc_rpmb_get_counter(mmc, &wcount)) {
			printf("Cannot read RPMB write counter\n");
			break;
		}

		/* Fill the request */
		memset(rpmb_frame, 0, sizeof(struct s_rpmb));
		memcpy(rpmb_frame->data, addr + i * RPMB_SZ_DATA, RPMB_SZ_DATA);
		rpmb_frame->address = cpu_to_be16(blk + i);
		rpmb_frame->block_count = cpu_to_be16(1);
		rpmb_frame->write_counter = cpu_to_be32(wcount);
		rpmb_frame->request = cpu_to_be16(RPMB_REQ_WRITE_DATA);
		/* Computes HMAC */
		rpmb_hmac(key, rpmb_frame->data, 284, rpmb_frame->mac);

		if (mmc_rpmb_request(mmc, rpmb_frame, 1, true))
			break;

		/* Get status */
		if (mmc_rpmb_status(mmc, RPMB_RESP_WRITE_DATA))
			break;
	}
	return i;
}
