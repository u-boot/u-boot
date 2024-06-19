/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 */
#ifndef _TIMER_DEFS_H_
#define _TIMER_DEFS_H_

struct davinci_timer {
	u_int32_t	pid12;
	u_int32_t	emumgt;
	u_int32_t	na1;
	u_int32_t	na2;
	u_int32_t	tim12;
	u_int32_t	tim34;
	u_int32_t	prd12;
	u_int32_t	prd34;
	u_int32_t	tcr;
	u_int32_t	tgcr;
	u_int32_t	wdtcr;
};

#endif /* _TIMER_DEFS_H_ */
