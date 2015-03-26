/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __silicon_if_H
#define __silicon_if_H

/* max number of devices supported by driver */
#ifdef CO_CPU_RUN
#define HWS_MAX_DEVICE_NUM (1)
#else
#define HWS_MAX_DEVICE_NUM (16)
#endif

#endif /* __silicon_if_H */
