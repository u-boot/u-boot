// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_devicepath
 *
 * Copyright (c) 2020 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the EFI_TCG2_PROTOCOL
 */

#include <efi_selftest.h>
#include <efi_tcg2.h>

static struct efi_boot_services *boottime;
static const efi_guid_t guid_tcg2 = EFI_TCG2_PROTOCOL_GUID;

/**
 * efi_st_tcg2_setup() - setup test
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	status code
 */
static int efi_st_tcg2_setup(const efi_handle_t img_handle,
			     const struct efi_system_table *systable)
{
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

/**
 * efi_st_tcg2_execute() - execute test
 *
 * Call the GetCapability service of the EFI_TCG2_PROTOCOL.
 *
 * Return:	status code
 */
static int efi_st_tcg2_execute(void)
{
	struct efi_tcg2_protocol *tcg2;
	struct efi_tcg2_boot_service_capability capability;
	efi_status_t ret;

	ret = boottime->locate_protocol(&guid_tcg2, NULL, (void **)&tcg2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("TCG2 protocol is not available.\n");
		return EFI_ST_FAILURE;
	}
	capability.size = sizeof(struct efi_tcg2_boot_service_capability) - 1;
	ret = tcg2->get_capability(tcg2, &capability);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("tcg2->get_capability on small buffer failed\n");
		return EFI_ST_FAILURE;
	}
	capability.size = sizeof(struct efi_tcg2_boot_service_capability);
	ret = tcg2->get_capability(tcg2, &capability);
	if (ret != EFI_SUCCESS) {
		efi_st_error("tcg2->get_capability failed\n");
		return EFI_ST_FAILURE;
	}
	if (!capability.tpm_present_flag) {
		efi_st_error("TPM not present\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("TPM supports 0x%.8x event logs\n",
		      capability.supported_event_logs);
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(tcg2) = {
	.name = "tcg2",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = efi_st_tcg2_execute,
	.setup = efi_st_tcg2_setup,
};
