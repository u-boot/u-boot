/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom.
 *
 */

#ifndef __CHIMP_H__
#define __CHIMP_H__

#include <linux/compiler.h>

/**
 * chimp_fastboot_optee() - api to load bnxt firmware
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_fastboot_optee(void);

/**
 * chimp_health_status_optee() - get chimp health status
 *
 * Chimp health status could be firmware is in good condition or
 * bad condition because of crash/hang.
 *
 * @status: pointer to get chimp health status
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_health_status_optee(u32 *status);

/**
 * chimp_handshake_status_optee() - get chimp handshake status.
 *
 * To know firmware is loaded and running.
 *
 * @timeout: timeout value, if 0 then default timeout is considered by op-tee
 * @hstatus: pointer to chimp handshake status
 *
 * @return: 0 on success and -ve on failure
 */
int chimp_handshake_status_optee(u32 timeout, u32 *hstatus);

#endif
