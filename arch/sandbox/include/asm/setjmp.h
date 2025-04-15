/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _ASM_SETJMP_H_
#define _ASM_SETJMP_H_

struct jmp_buf_data {
	/*
	 * We're not sure how long this should be:
	 *
	 *   amd64: 200 bytes
	 *   arm64: 392 bytes
	 *   armhf: 392 bytes
	 *
	 * So allow space for all of those, plus some extra.
	 * We don't need to worry about 16-byte alignment, since this does not
	 * run on Windows.
	 */
	unsigned long data[128];
};

#endif /* _ASM_SETJMP_H_ */
