// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_ipconfig
 *
 * This unit test covers the IPv4 Config2 Protocol.
 *
 */

#include <efi_selftest.h>
#include <charset.h>
#include <net.h>

static struct efi_boot_services *boottime;

static struct efi_ip4_config2_protocol *ip4_config2;
static const efi_guid_t efi_ip4_config2_guid = EFI_IP4_CONFIG2_PROTOCOL_GUID;

/*
 * Setup unit test.
 *
 * Open IPv4 Config2 protocol
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;
	efi_handle_t *net_handle;
	efi_uintn_t num_handles;
	efi_handle_t *handles;

	boottime = systable->boottime;

	num_handles = 0;
	boottime->locate_handle_buffer(BY_PROTOCOL, &efi_ip4_config2_guid,
				       NULL, &num_handles, &handles);

	if (!num_handles) {
		efi_st_error("Failed to locate ipv4 config2 protocol\n");
		return EFI_ST_FAILURE;
	}

	for (net_handle = handles; num_handles--; net_handle++) {
		ret = boottime->open_protocol(*net_handle, &efi_ip4_config2_guid,
					      (void **)&ip4_config2, 0, 0,
					      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS || !ip4_config2)
			continue;
		break; // Get first handle that supports ipv4
	}

	if (!ip4_config2) {
		efi_st_error("Failed to open ipv4 config2 protocol\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	enum efi_ip4_config2_policy policy;
	efi_uintn_t data_size;
	struct efi_ip4_config2_manual_address manual_address;
	struct efi_ip4_config2_manual_address orig_address;
	u8 netmask[] = {255, 255, 255, 0};
	u8 ip[] = {10, 0, 0, 1};

	/* Setup may have failed */
	if (!ip4_config2) {
		efi_st_error("Setup failure, cannot proceed with test\n");
		return EFI_ST_FAILURE;
	}

	/* Set policy to static */
	policy = EFI_IP4_CONFIG2_POLICY_STATIC;
	ret = ip4_config2->set_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_POLICY,
			      sizeof(policy), (void *)&policy);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to set policy\n");
		return EFI_ST_FAILURE;
	}

	/* Save original ip address and netmask */
	data_size = sizeof(manual_address);
	ret = ip4_config2->get_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS,
			      &data_size, &orig_address);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to save original ip address and netmask\n");
		return EFI_ST_FAILURE;
	}

	/* Set static ip and netmask */
	memcpy(&manual_address.address, ip,
	       sizeof(struct efi_ipv4_address));
	memcpy(&manual_address.subnet_mask, netmask,
	       sizeof(struct efi_ipv4_address));
	ret = ip4_config2->set_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS,
			      sizeof(manual_address), &manual_address);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to get ip address and netmask\n");
		return EFI_ST_FAILURE;
	}

	/* Try to set interface info, this should fail */
	ret = ip4_config2->set_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_INTERFACEINFO, 0, NULL);
	if (ret == EFI_SUCCESS) {
		efi_st_error("Interface info is read-only\n");
		return EFI_ST_FAILURE;
	}

	/* Get ip address and netmask and check that they match with the previously set ones */
	data_size = sizeof(manual_address);
	ret = ip4_config2->get_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS,
			      &data_size, &manual_address);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to get ip address and netmask\n");
		return EFI_ST_FAILURE;
	}
	if (memcmp(ip, &manual_address.address,
		   sizeof(struct efi_ipv4_address)) ||
	    memcmp(netmask, &manual_address.subnet_mask,
		   sizeof(struct efi_ipv4_address))) {
		efi_st_error("Ip address mismatch\n");
		return EFI_ST_FAILURE;
	}

	/* Restore original ip address and netmask */
	ret = ip4_config2->set_data(ip4_config2, EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS,
			      sizeof(orig_address), &orig_address);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to restore original ip address and netmask\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Efi ipconfig test execute succeeded\n");
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	int exit_status = EFI_ST_SUCCESS;

	return exit_status;
}

EFI_UNIT_TEST(ipconfig) = {
	.name = "IPv4 config2 protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
#ifdef CONFIG_SANDBOX
	/*
	 * Running this test on the sandbox requires setting environment
	 * variable ethact to a network interface connected to a DHCP server and
	 * ethrotate to 'no'.
	 */
	.on_request = true,
#endif
};
