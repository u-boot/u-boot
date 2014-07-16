/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 *
*/

#ifndef __SECURE_MX6Q_H__
#define __SECURE_MX6Q_H__

#include <linux/types.h>

/* -------- start of HAB API updates ------------*/
/* The following are taken from HAB4 SIS */

/* Status definitions */
enum hab_status {
	HAB_STS_ANY = 0x00,
	HAB_FAILURE = 0x33,
	HAB_WARNING = 0x69,
	HAB_SUCCESS = 0xf0
};

/* Security Configuration definitions */
enum hab_config {
	HAB_CFG_RETURN = 0x33, /**< Field Return IC */
	HAB_CFG_OPEN = 0xf0, /**< Non-secure IC */
	HAB_CFG_CLOSED = 0xcc /**< Secure IC */
};

/* State definitions */
enum hab_state {
	HAB_STATE_INITIAL = 0x33, /**< Initialising state (transitory) */
	HAB_STATE_CHECK = 0x55, /**< Check state (non-secure) */
	HAB_STATE_NONSECURE = 0x66, /**< Non-secure state */
	HAB_STATE_TRUSTED = 0x99, /**< Trusted state */
	HAB_STATE_SECURE = 0xaa, /**< Secure state */
	HAB_STATE_FAIL_SOFT = 0xcc, /**< Soft fail state */
	HAB_STATE_FAIL_HARD = 0xff, /**< Hard fail state (terminal) */
	HAB_STATE_NONE = 0xf0, /**< No security state machine */
	HAB_STATE_MAX
};

/*Function prototype description*/
typedef enum hab_status hab_rvt_report_event_t(enum hab_status, uint32_t,
		uint8_t* , size_t*);
typedef enum hab_status hab_rvt_report_status_t(enum hab_config *,
		enum hab_state *);
typedef enum hab_status hab_loader_callback_f_t(void**, size_t*, const void*);
typedef enum hab_status hab_rvt_entry_t(void);
typedef enum hab_status hab_rvt_exit_t(void);
typedef void *hab_rvt_authenticate_image_t(uint8_t, ptrdiff_t,
		void **, size_t *, hab_loader_callback_f_t);
typedef void hapi_clock_init_t(void);

#define HAB_RVT_REPORT_EVENT                   (*(uint32_t *)0x000000B4)
#define HAB_RVT_REPORT_STATUS                  (*(uint32_t *)0x000000B8)
#define HAB_RVT_AUTHENTICATE_IMAGE             (*(uint32_t *)0x000000A4)
#define HAB_RVT_ENTRY                          (*(uint32_t *)0x00000098)
#define HAB_RVT_EXIT                           (*(uint32_t *)0x0000009C)

#define HAB_RVT_REPORT_EVENT_NEW               (*(uint32_t *)0x000000B8)
#define HAB_RVT_REPORT_STATUS_NEW              (*(uint32_t *)0x000000BC)
#define HAB_RVT_AUTHENTICATE_IMAGE_NEW         (*(uint32_t *)0x000000A8)
#define HAB_RVT_ENTRY_NEW                      (*(uint32_t *)0x0000009C)
#define HAB_RVT_EXIT_NEW                       (*(uint32_t *)0x000000A0)

#define HAB_CID_ROM 0 /**< ROM Caller ID */
#define HAB_CID_UBOOT 1 /**< UBOOT Caller ID*/
/* ----------- end of HAB API updates ------------*/

#endif
