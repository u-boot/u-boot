/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _post_h
#define _post_h

/* port to use for post codes */
#define POST_PORT		0x80

/* post codes which represent various stages of init */
#define POST_START		0x1e
#define POST_CAR_START		0x1f

#define POST_START_STACK	0x29
#define POST_START_DONE		0x2a

/* Output a post code using al - value must be 0 to 0xff */
#ifdef __ASSEMBLY__
#define post_code(value) \
	movb	$value, %al; \
	outb	%al, $POST_PORT
#else
static inline void post_code(int code)
{
	outb(code, POST_PORT);
}
#endif

#endif
