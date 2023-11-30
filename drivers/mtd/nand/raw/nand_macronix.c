// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Free Electrons
 * Copyright (C) 2017 NextThing Co
 *
 * Author: Boris Brezillon <boris.brezillon@free-electrons.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <dm/device_compat.h>
#include <linux/mtd/rawnand.h>

#define ONFI_FEATURE_ADDR_30LFXG18AC_OTP	0x90
#define MACRONIX_30LFXG18AC_OTP_START_PAGE	2
#define MACRONIX_30LFXG18AC_OTP_PAGES		30
#define MACRONIX_30LFXG18AC_OTP_PAGE_SIZE	2112
#define MACRONIX_30LFXG18AC_OTP_SIZE_BYTES	\
	(MACRONIX_30LFXG18AC_OTP_PAGES *	\
	 MACRONIX_30LFXG18AC_OTP_PAGE_SIZE)

#define MACRONIX_30LFXG18AC_OTP_EN		BIT(0)

static int macronix_30lfxg18ac_get_otp_info(struct mtd_info *mtd, size_t len,
					    size_t *retlen,
					    struct otp_info *buf)
{
	if (len < sizeof(*buf))
		return -EINVAL;

	/* Always report that OTP is unlocked. Reason is that this
	 * type of flash chip doesn't provide way to check that OTP
	 * is locked or not: subfeature parameter is implemented as
	 * volatile register. Technically OTP region could be locked
	 * and become readonly, but as there is no way to check it,
	 * don't allow to lock it ('_lock_user_prot_reg' callback
	 * always returns -EOPNOTSUPP) and thus we report that OTP
	 * is unlocked.
	 */
	buf->locked = 0;
	buf->start = 0;
	buf->length = MACRONIX_30LFXG18AC_OTP_SIZE_BYTES;

	*retlen = sizeof(*buf);

	return 0;
}

static int macronix_30lfxg18ac_otp_enable(struct nand_chip *nand)
{
	u8 feature_buf[ONFI_SUBFEATURE_PARAM_LEN] = { 0 };
	struct mtd_info *mtd;

	mtd = nand_to_mtd(nand);
	feature_buf[0] = MACRONIX_30LFXG18AC_OTP_EN;

	return nand->onfi_set_features(mtd, nand, ONFI_FEATURE_ADDR_30LFXG18AC_OTP, feature_buf);
}

static int macronix_30lfxg18ac_otp_disable(struct nand_chip *nand)
{
	u8 feature_buf[ONFI_SUBFEATURE_PARAM_LEN] = { 0 };
	struct mtd_info *mtd;

	mtd = nand_to_mtd(nand);
	return nand->onfi_set_features(mtd, nand, ONFI_FEATURE_ADDR_30LFXG18AC_OTP, feature_buf);
}

static int __macronix_30lfxg18ac_rw_otp(struct mtd_info *mtd,
					loff_t offs_in_flash,
					size_t len, size_t *retlen,
					u_char *buf, bool write)
{
	struct nand_chip *nand;
	size_t bytes_handled;
	off_t offs_in_page;
	u64 page;
	int ret;

	nand = mtd_to_nand(mtd);
	nand->select_chip(mtd, 0);

	ret = macronix_30lfxg18ac_otp_enable(nand);
	if (ret)
		goto out_otp;

	page = offs_in_flash;
	/* 'page' will be result of division. */
	offs_in_page = do_div(page, MACRONIX_30LFXG18AC_OTP_PAGE_SIZE);
	bytes_handled = 0;

	while (bytes_handled < len &&
	       page < MACRONIX_30LFXG18AC_OTP_PAGES) {
		size_t bytes_to_handle;
		u64 phys_page = page + MACRONIX_30LFXG18AC_OTP_START_PAGE;

		bytes_to_handle = min_t(size_t, len - bytes_handled,
					MACRONIX_30LFXG18AC_OTP_PAGE_SIZE -
					offs_in_page);

		if (write)
			ret = nand_prog_page_op(nand, phys_page, offs_in_page,
						&buf[bytes_handled], bytes_to_handle);
		else
			ret = nand_read_page_op(nand, phys_page, offs_in_page,
						&buf[bytes_handled], bytes_to_handle);
		if (ret)
			goto out_otp;

		bytes_handled += bytes_to_handle;
		offs_in_page = 0;
		page++;
	}

	*retlen = bytes_handled;

out_otp:
	if (ret)
		dev_err(mtd->dev, "failed to perform OTP IO: %i\n", ret);

	ret = macronix_30lfxg18ac_otp_disable(nand);
	if (ret)
		dev_err(mtd->dev, "failed to leave OTP mode after %s\n",
			write ? "write" : "read");

	nand->select_chip(mtd, -1);

	return ret;
}

static int macronix_30lfxg18ac_write_otp(struct mtd_info *mtd, loff_t to,
					 size_t len, size_t *rlen,
					 u_char *buf)
{
	return __macronix_30lfxg18ac_rw_otp(mtd, to, len, rlen, (u_char *)buf,
					    true);
}

static int macronix_30lfxg18ac_read_otp(struct mtd_info *mtd, loff_t from,
					size_t len, size_t *rlen,
					u_char *buf)
{
	return __macronix_30lfxg18ac_rw_otp(mtd, from, len, rlen, buf, false);
}

static int macronix_30lfxg18ac_lock_otp(struct mtd_info *mtd, loff_t from,
					size_t len)
{
	/* See comment in 'macronix_30lfxg18ac_get_otp_info()'. */
	return -EOPNOTSUPP;
}

static void macronix_nand_setup_otp(struct nand_chip *chip)
{
	static const char * const supported_otp_models[] = {
		"MX30LF1G18AC",
		"MX30LF2G18AC",
		"MX30LF4G18AC",
	};
	int i;

	if (!chip->onfi_version ||
	    !(le16_to_cpu(chip->onfi_params.opt_cmd)
	      & ONFI_OPT_CMD_SET_GET_FEATURES))
		return;

	for (i = 0; i < ARRAY_SIZE(supported_otp_models); i++) {
		if (!strcmp(chip->onfi_params.model, supported_otp_models[i])) {
			struct mtd_info *mtd;

			mtd = nand_to_mtd(chip);
			mtd->_get_user_prot_info = macronix_30lfxg18ac_get_otp_info;
			mtd->_read_user_prot_reg = macronix_30lfxg18ac_read_otp;
			mtd->_write_user_prot_reg = macronix_30lfxg18ac_write_otp;
			mtd->_lock_user_prot_reg = macronix_30lfxg18ac_lock_otp;
			return;
		}
	}
}

static int macronix_nand_init(struct nand_chip *chip)
{
	if (nand_is_slc(chip))
		chip->bbt_options |= NAND_BBT_SCAN2NDPAGE;

	macronix_nand_setup_otp(chip);

	return 0;
}

const struct nand_manufacturer_ops macronix_nand_manuf_ops = {
	.init = macronix_nand_init,
};
