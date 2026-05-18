// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Masahiro Yamada <yamada.masahiro@socionext.com>
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <dm.h>
#include <sysreset.h>
#include <asm/system.h>
#include <linux/errno.h>
#include <linux/psci.h>
#include <asm/psci.h>

static int qcom_psci_sysreset_get_status(struct udevice *dev, char *buf, int size)
{
	return -EOPNOTSUPP;
}

static int qcom_psci_sysreset_request_arg(struct udevice *dev, int argc,
					  char * const argv[])
{
	if (!strncasecmp(argv[1], "-edl", 4)) {
		/* Supported in qcs9100, qcs8300, sc7280, qcs615 */
		if (psci_features(ARM_PSCI_1_1_FN64_SYSTEM_RESET2) ==
							ARM_PSCI_RET_SUCCESS) {
			psci_system_reset2(0, 1);
			return -EINPROGRESS;
		}
		printf("PSCI SYSTEM_RESET2 not supported\n");
	}

	return -EPROTONOSUPPORT;
}

static struct sysreset_ops qcom_psci_sysreset_ops = {
	.request_arg = qcom_psci_sysreset_request_arg,
	.get_status = qcom_psci_sysreset_get_status,
};

U_BOOT_DRIVER(qcom_psci_sysreset) = {
	.name = "qcom_psci-sysreset",
	.id = UCLASS_SYSRESET,
	.ops = &qcom_psci_sysreset_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
