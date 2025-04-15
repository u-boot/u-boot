// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_http
 *
 * This unit test covers the IPv4 Config2 Protocol, Http Service Binding Protocol,
 * and Http Protocol.
 *
 * An Http HEAD and an Http GET request are sent to the same destination. The test
 * is successful if the HEAD request gets a response with a valid Content-Length header
 * and the subsequent GET request receives the amount of bytes informed by the previous
 * Content-Length header.
 *
 */

#include <efi_selftest.h>
#include <charset.h>
#include <net.h>

static struct efi_boot_services *boottime;

static struct efi_http_protocol *http;
static struct efi_service_binding_protocol *http_service;
static struct efi_ip4_config2_protocol *ip4_config2;
static efi_handle_t http_protocol_handle;

static const efi_guid_t efi_http_guid = EFI_HTTP_PROTOCOL_GUID;
static const efi_guid_t efi_http_service_binding_guid = EFI_HTTP_SERVICE_BINDING_PROTOCOL_GUID;
static const efi_guid_t efi_ip4_config2_guid = EFI_IP4_CONFIG2_PROTOCOL_GUID;
static int callback_done;

/*
 * Setup unit test.
 *
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
	struct efi_http_config_data http_config;
	struct efi_httpv4_access_point ipv4_node;

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
		ret = boottime->open_protocol(*net_handle,
					      &efi_http_service_binding_guid,
					      (void **)&http_service, 0, 0,
					      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS || !http_service)
			continue;
		break; // Get first handle that supports both protocols
	}

	if (!ip4_config2 || !http_service) {
		efi_st_error("Failed to open ipv4 config2 or http service binding protocol\n");
		return EFI_ST_FAILURE;
	}

	http_protocol_handle = NULL;
	ret = http_service->create_child(http_service, &http_protocol_handle);
	if (ret != EFI_SUCCESS || !http_protocol_handle) {
		efi_st_error("Failed to create an http service instance\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->open_protocol(http_protocol_handle, &efi_http_guid,
				      (void **)&http, 0, 0, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS || !http) {
		efi_st_error("Failed to open http protocol\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("HTTP Service Binding: child created successfully\n");

	http_config.http_version = HTTPVERSION11;
	http_config.is_ipv6 = false;
	http_config.access_point.ipv4_node = &ipv4_node;
	ipv4_node.use_default_address = true;

	ret = http->configure(http, &http_config);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to configure http instance\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

void EFIAPI efi_test_http_callback(struct efi_event *event, void *context)
{
	callback_done = 1;
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
	struct efi_http_request_data request_data;
	struct efi_http_message request_message;
	struct efi_http_token request_token;
	struct efi_http_response_data response_data;
	struct efi_http_message response_message;
	struct efi_http_token response_token;
	enum efi_http_status_code status_code;
	void *response_buffer;
	efi_uintn_t len, sum;
	char *url = "http://example.com/";
	u16 url_16[64];
	u16 *tmp;

	/* Setup may have failed */
	if (!ip4_config2 || !http) {
		efi_st_error("Cannot proceed with test after setup failure\n");
		return EFI_ST_FAILURE;
	}

	tmp = url_16;
	utf8_utf16_strcpy(&tmp, url);
	request_data.url = url_16;
	request_data.method = HTTP_METHOD_GET;

	request_message.data.request = &request_data;
	request_message.header_count = 3;
	request_message.body_length = 0;
	request_message.body = NULL;

	/* request token */
	request_token.event = NULL;
	request_token.status = EFI_NOT_READY;
	request_token.message = &request_message;
	callback_done = 0;
	ret = boottime->create_event(EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK,
			efi_test_http_callback,
			NULL,
			&request_token.event);

	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to create request event\n");
		return EFI_ST_FAILURE;
	}

	ret = http->request(http, &request_token);

	if (ret != EFI_SUCCESS) {
		boottime->close_event(request_token.event);
		efi_st_printf("Failed to proceed with the http request\n");
		return EFI_ST_SUCCESS;
	}

	while (!callback_done)
		http->poll(http);

	response_data.status_code = HTTP_STATUS_UNSUPPORTED_STATUS;
	response_message.data.response = &response_data;
	response_message.header_count = 0;
	response_message.headers = NULL;
	response_message.body_length = 0;
	response_message.body = NULL;
	response_token.event = NULL;

	ret = boottime->create_event(EVT_NOTIFY_SIGNAL,
			TPL_CALLBACK,
			efi_test_http_callback,
			NULL,
			&response_token.event);

	if (ret != EFI_SUCCESS) {
		boottime->close_event(request_token.event);
		efi_st_error("Failed to create response event\n");
		return EFI_ST_FAILURE;
	}

	response_token.status = EFI_SUCCESS;
	response_token.message = &response_message;

	callback_done = 0;
	ret = http->response(http, &response_token);

	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed http first response\n");
		goto fail;
	}

	while (!callback_done)
		http->poll(http);

	if (response_message.data.response->status_code != HTTP_STATUS_200_OK) {
		status_code = response_message.data.response->status_code;
		if (status_code == HTTP_STATUS_404_NOT_FOUND) {
			efi_st_error("File not found\n");
		} else {
			efi_st_error("Bad http status %d\n",
				     response_message.data.response->status_code);
		}
		goto fail_free_hdr;
	}

	ret = boottime->allocate_pool(EFI_LOADER_CODE, response_message.body_length,
				      &response_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed allocating response buffer\n");
		goto fail_free_hdr;
	}

	len = response_message.body_length;
	sum = 0;
	while (len) {
		response_message.data.response = NULL;
		response_message.header_count = 0;
		response_message.headers = NULL;
		response_message.body_length = len;
		response_message.body = response_buffer + sum;

		response_token.message = &response_message;
		response_token.status = EFI_NOT_READY;

		callback_done = 0;
		ret = http->response(http, &response_token);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed http second response\n");
			goto fail_free_buf;
		}

		while (!callback_done)
			http->poll(http);

		if (!response_message.body_length)
			break;

		len -= response_message.body_length;
		sum += response_message.body_length;
	}

	if (len)
		goto fail_free_buf;

	boottime->free_pool(response_buffer);
	if (response_message.headers)
		boottime->free_pool(response_message.headers);
	boottime->close_event(request_token.event);
	boottime->close_event(response_token.event);
	efi_st_printf("Efi Http request executed successfully\n");
	return EFI_ST_SUCCESS;

fail_free_buf:
	boottime->free_pool(response_buffer);
fail_free_hdr:
	if (response_message.headers)
		boottime->free_pool(response_message.headers);
fail:
	boottime->close_event(request_token.event);
	boottime->close_event(response_token.event);
	return EFI_ST_FAILURE;
}

/*
 * Tear down unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;
	int exit_status = EFI_ST_SUCCESS;

	if (!http_service || !http_protocol_handle) {
		efi_st_error("No handles to destroy http instance");
		exit_status = EFI_ST_FAILURE;
	} else {
		ret = http_service->destroy_child(http_service, http_protocol_handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to destroy http instance");
			exit_status = EFI_ST_FAILURE;
		}
		efi_st_printf("HTTP Service Binding: child destroyed successfully\n");
	}

	return exit_status;
}

EFI_UNIT_TEST(http) = {
	.name = "http protocol",
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
