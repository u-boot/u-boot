// SPDX-License-Identifier: GPL-2.0+
/*
 * Start the TPM during init, so that the OS does not have to do it.
 *
 * Copyright (c) 2026 Siemens AG
 */

#define LOG_CATEGORY UCLASS_TPM

#include <dm.h>
#include <errno.h>
#include <event.h>
#include <log.h>
#include <tpm_api.h>

int tpm_autostart_init(void)
{
	struct udevice *dev;
	int ret;
	u32 rc;

	ret = uclass_first_device_err(UCLASS_TPM, &dev);
	if (ret) {
		/* a board may share one binary between variants without a TPM */
		if (ret == -ENODEV)
			return 0;

		log_warning("Cannot init TPM (err=%dE)\n", ret);
		return ret;
	}

	rc = tpm_auto_start(dev);
	if (rc) {
		log_warning("TPM startup failed (rc=%#x)\n", rc);
		return -EIO;
	}

	return 0;
}

static int tpm_evt_autostart(void)
{
	/* errors are reported above and must not stop the boot */
	tpm_autostart_init();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, tpm_evt_autostart);
