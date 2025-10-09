// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Lucien Jheng <lucienzx159@gmail.com>
 */

#include <command.h>
#include <env.h>
#include <errno.h>
#include <linux/types.h>
#include <log.h>

int request_firmware_into_buf_via_script(void *buf, size_t max_size,
					 const char *script_name,
					 size_t *retsize)
{
	char env_addr[CONFIG_SYS_CBSIZE] = { 0 };
	char env_size[CONFIG_SYS_CBSIZE] = { 0 };
	char *args[2] = { "run", (char *)script_name };
	int ret, repeatable;
	ulong addr, size;

	if (!buf || !script_name || !max_size)
		return -EINVAL;

	/* Run the firmware loading script */
	ret = cmd_process(0, 2, args, &repeatable, NULL);
	if (ret) {
		log_err("Firmware loading script '%s' not defined or failed.\n",
			script_name);
		return -EINVAL;
	}

	/* Prefix the FW loader variables with the script prefix */
	snprintf(env_addr, sizeof(env_addr), "%s_addr", script_name);
	snprintf(env_size, sizeof(env_size), "%s_size", script_name);

	/* Find out where the firmware got loaded and how long it is */
	addr = env_get_hex(env_addr, 0);
	size = env_get_hex(env_size, 0);

	/* Clear the variables set by the firmware loading script */
	env_set(env_addr, NULL);
	env_set(env_size, NULL);

	if (!addr || !size) {
		log_err("Firmware address (0x%lx) or size (0x%lx) are invalid.\n",
			addr, size);
		return -EINVAL;
	}

	if (size > max_size) {
		log_err("Loaded firmware size 0x%lx exceeded maximum allowed size 0x%zx.\n",
			size, max_size);
		return -E2BIG;
	}

	if (retsize)
		*retsize = size;

	memcpy(buf, (void *)addr, size);

	return 0;
}
