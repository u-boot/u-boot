/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020, Linaro Limited
 */

#ifndef __SANDBOX_SCMI_TEST_H
#define __SANDBOX_SCMI_TEST_H

struct udevice;
struct sandbox_scmi_agent;
struct sandbox_scmi_service;

/**
 * struct sandbox_scmi_agent - Simulated SCMI service seen by SCMI agent
 * @idx:	Identifier for the SCMI agent, its index
 */
struct sandbox_scmi_agent {
	uint idx;
};

/**
 * struct sandbox_scmi_service - Reference to simutaed SCMI agents/services
 * @agent:		Pointer to SCMI sandbox agent pointers array
 * @agent_count:	Number of emulated agents exposed in array @agent.
 */
struct sandbox_scmi_service {
	struct sandbox_scmi_agent **agent;
	size_t agent_count;
};

#ifdef CONFIG_SCMI_FIRMWARE
/**
 * sandbox_scmi_service_context - Get the simulated SCMI services context
 * @return:	Reference to backend simulated resources state
 */
struct sandbox_scmi_service *sandbox_scmi_service_ctx(void);
#else
static inline struct sandbox_scmi_service *sandbox_scmi_service_ctx(void)
{
	return NULL;
}
#endif /* CONFIG_SCMI_FIRMWARE */
#endif /* __SANDBOX_SCMI_TEST_H */
