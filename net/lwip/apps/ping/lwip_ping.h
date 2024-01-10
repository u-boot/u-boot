/* SPDX-License-Identifier: GPL-2.0+ */

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#ifndef LWIP_PING_H
#define LWIP_PING_H

#include <lwip/ip_addr.h>

void ping_raw_init(void);
void ping_send_now(void);

#endif /* LWIP_PING_H */
