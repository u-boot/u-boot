/*
 * sdp.h - Serial Download Protocol
 *
 * Copyright (C) 2017 Toradex
 * Author: Stefan Agner <stefan.agner@toradex.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SDP_H_
#define __SDP_H_

int sdp_init(int controller_index);
void sdp_handle(int controller_index);

#endif /* __SDP_H_ */
