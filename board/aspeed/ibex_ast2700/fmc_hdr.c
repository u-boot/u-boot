// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */

#include <asm/arch/fmc_hdr.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <errno.h>
#include <spl.h>
#include <string.h>

int fmc_hdr_get_prebuilt(uint32_t type, uint32_t *ofst, uint32_t *size)
{
	struct fmc_hdr_preamble *preamble;
	struct fmc_hdr_body *body;
	struct fmc_hdr *hdr;
	uint32_t t, s, o;
	int i;

	if (type >= PBT_NUM)
		return -EINVAL;

	if (!ofst || !size)
		return -EINVAL;

	hdr = (struct fmc_hdr *)(_start - sizeof(*hdr));
	preamble = &hdr->preamble;
	body = &hdr->body;

	if (preamble->magic != HDR_MAGIC)
		return -EIO;

	for (i = 0, o = sizeof(*hdr) + body->fmc_size; i < HDR_PB_MAX; ++i) {
		t = body->pbs[i].type;
		s = body->pbs[i].size;

		/* skip if unrecognized, yet */
		if (t >= PBT_NUM) {
			o += s;
			continue;
		}

		/* prebuilt end mark */
		if (t == 0 && s == 0)
			break;

		/* return the prebuilt info if found */
		if (t == type) {
			*ofst = o;
			*size = s;

			goto found;
		}

		/* update offset for next prebuilt */
		o += s;
	}

	return -ENODATA;

found:
	return 0;
}
