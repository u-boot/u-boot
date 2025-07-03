// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * An HTTP driver
 *
 * HTTP_PROTOCOL
 * HTTP_SERVICE_BINDING_PROTOCOL
 * IP4_CONFIG2_PROTOCOL
 */

#define LOG_CATEGORY LOGC_EFI

#include <charset.h>
#include <efi_loader.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>

static const efi_guid_t efi_http_service_binding_guid = EFI_HTTP_SERVICE_BINDING_PROTOCOL_GUID;
static const efi_guid_t efi_http_guid = EFI_HTTP_PROTOCOL_GUID;

/**
 * struct efi_http_instance - EFI object representing an HTTP protocol instance
 *
 * @http:			EFI_HTTP_PROTOCOL interface
 * @handle:			handle to efi object
 * @configured:			configuration status
 * @http_load_addr:		data buffer
 * @file_size:			size of data
 * @current_offset:		offset in data buffer
 * @status_code:		HTTP status code
 * @num_headers:		number of received headers
 * @headers:			array of headers
 * @headers_buffer:		raw buffer with headers
 */
struct efi_http_instance {
	struct efi_http_protocol http;
	efi_handle_t handle;
	struct efi_service_binding_protocol *parent;
	bool configured;
	void *http_load_addr;
	ulong file_size;
	ulong current_offset;
	u32 status_code;
	ulong num_headers;
	struct http_header headers[MAX_HTTP_HEADERS];
	char headers_buffer[MAX_HTTP_HEADERS_SIZE];
};

static int num_instances;

/*
 * efi_u32_to_httpstatus() - convert u32 to status
 *
 */
enum efi_http_status_code efi_u32_to_httpstatus(u32 status);

/*
 * efi_http_send_data() - sends data to client
 *
 *
 * @client_buffer:		client buffer to send data to
 * @client_buffer_size:		size of the client buffer
 * @inst:			HTTP instance for which to send data
 *
 * Return:	status code
 */
static efi_status_t efi_http_send_data(void *client_buffer,
				       efi_uintn_t *client_buffer_size,
				       struct efi_http_instance *inst)
{
	efi_status_t ret = EFI_SUCCESS;
	ulong total_size, transfer_size;
	uchar *ptr;

	// Amount of data left;
	total_size = inst->file_size;
	transfer_size = total_size - inst->current_offset;
	debug("efi_http: sending data to client, total size %lu\n", total_size);
	// Amount of data the client is willing to receive
	if (transfer_size > *client_buffer_size)
		transfer_size = *client_buffer_size;
	else
		*client_buffer_size = transfer_size;
	debug("efi_http: transfer size %lu\n", transfer_size);
	if (!transfer_size) // Ok, only headers
		goto out;

	if (!client_buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	// Send data
	ptr = (uchar *)inst->http_load_addr + inst->current_offset;
	memcpy(client_buffer, ptr, transfer_size);

	inst->current_offset += transfer_size;

	// Whole file served, clean the buffer:
	if (inst->current_offset == inst->file_size) {
		efi_free_pool(inst->http_load_addr);
		inst->http_load_addr = NULL;
		inst->current_offset = 0;
		inst->file_size = 0;
	}

out:
	return ret;
}

/* EFI_HTTP_PROTOCOL */

/*
 * efi_http_get_mode_data() - Gets the current operational status.
 *
 * This function implements EFI_HTTP_PROTOCOL.GetModeData().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @data:	pointer to the buffer for operational parameters
 *		of this HTTP instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_get_mode_data(struct efi_http_protocol *this,
						  struct efi_http_config_data *data)
{
	EFI_ENTRY("%p, %p", this, data);

	efi_status_t ret = EFI_UNSUPPORTED;

	return EFI_EXIT(ret);
}

/*
 * efi_http_configure() - Initializes operational status for this
 * EFI HTTP instance.
 *
 * This function implements EFI_HTTP_PROTOCOL.Configure().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @data:	pointer to the buffer for operational parameters of
 *		this HTTP instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_configure(struct efi_http_protocol *this,
					      struct efi_http_config_data *data)
{
	EFI_ENTRY("%p, %p", this, data);

	efi_status_t ret = EFI_SUCCESS;
	enum efi_http_version http_version;
	struct efi_httpv4_access_point *ipv4_node;
	struct efi_http_instance *http_instance;

	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	http_instance = (struct efi_http_instance *)this;

	if (!data) {
		efi_free_pool(http_instance->http_load_addr);
		http_instance->http_load_addr = NULL;
		http_instance->current_offset = 0;
		http_instance->configured = false;

		goto out;
	}

	if (http_instance->configured) {
		ret = EFI_ALREADY_STARTED;
		goto out;
	}

	http_version = data->http_version;
	ipv4_node = data->access_point.ipv4_node;

	if ((http_version != HTTPVERSION10 &&
	    http_version != HTTPVERSION11) ||
	    data->is_ipv6 || !ipv4_node) { /* Only support ipv4 */
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	if (!ipv4_node->use_default_address) {
		efi_net_set_addr((struct efi_ipv4_address *)&ipv4_node->local_address,
				 (struct efi_ipv4_address *)&ipv4_node->local_subnet, NULL, NULL);
	}

	http_instance->current_offset = 0;
	http_instance->configured = true;

out:
	return EFI_EXIT(ret);
}

/*
 * efi_http_request() - Queues an HTTP request to this HTTP instance
 *
 * This function implements EFI_HTTP_PROTOCOL.Request().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @token:	pointer to storage containing HTTP request token
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_request(struct efi_http_protocol *this,
					    struct efi_http_token *token)
{
	EFI_ENTRY("%p, %p", this, token);

	efi_status_t ret = EFI_SUCCESS;
	u8 *tmp;
	u8 url_8[1024];
	u16 *url_16;
	enum efi_http_method current_method;
	struct efi_http_instance *http_instance;

	if (!token || !this || !token->message ||
	    !token->message->data.request) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	http_instance = (struct efi_http_instance *)this;

	if (!http_instance->configured) {
		ret = EFI_NOT_STARTED;
		goto out;
	}

	current_method = token->message->data.request->method;
	url_16 = token->message->data.request->url;

	/* Parse URL. It comes in UCS-2 encoding and follows RFC3986 */
	tmp = url_8;
	utf16_utf8_strncpy((char **)&tmp, url_16, 1024);

	ret = efi_net_do_request(url_8, current_method, &http_instance->http_load_addr,
				 &http_instance->status_code, &http_instance->file_size,
				 http_instance->headers_buffer, http_instance->parent);
	if (ret != EFI_SUCCESS)
		goto out;

	// We have a successful request
	efi_net_parse_headers(&http_instance->num_headers, http_instance->headers);
	http_instance->current_offset = 0;
	token->status = EFI_SUCCESS;
	goto out_signal;

out_signal:
	efi_signal_event(token->event);
out:
	return EFI_EXIT(ret);
}

/*
 * efi_http_cancel() - Abort an asynchronous HTTP request or response token
 *
 * This function implements EFI_HTTP_PROTOCOL.Cancel().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @token:	pointer to storage containing HTTP request token
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_cancel(struct efi_http_protocol *this,
					   struct efi_http_token *token)
{
	EFI_ENTRY("%p, %p", this, token);

	efi_status_t ret = EFI_UNSUPPORTED;

	return EFI_EXIT(ret);
}

/*
 * efi_http_response() -  Queues an HTTP response to this HTTP instance
 *
 * This function implements EFI_HTTP_PROTOCOL.Response().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @token:	pointer to storage containing HTTP request token
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_response(struct efi_http_protocol *this,
					     struct efi_http_token *token)
{
	EFI_ENTRY("%p, %p", this, token);

	efi_status_t ret = EFI_SUCCESS;
	struct efi_http_instance *http_instance;
	struct efi_http_header **client_headers;
	struct efi_http_response_data *response;

	if (!token || !this || !token->message) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	http_instance = (struct efi_http_instance *)this;

	// Set HTTP status code
	if (token->message->data.response) { // TODO extra check, see spec.
		response = token->message->data.response;
		response->status_code = efi_u32_to_httpstatus(http_instance->status_code);
	}

	client_headers = &token->message->headers;

	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA,
				(http_instance->num_headers) * sizeof(struct efi_http_header),
				(void **)client_headers); // This is deallocated by the client.
	if (ret != EFI_SUCCESS)
		goto out_bad_signal;

	// Send headers
	token->message->header_count = http_instance->num_headers;
	for (int i = 0; i < http_instance->num_headers; i++) {
		(*client_headers)[i].field_name = http_instance->headers[i].name;
		(*client_headers)[i].field_value = http_instance->headers[i].value;
	}

	ret = efi_http_send_data(token->message->body, &token->message->body_length, http_instance);
	if (ret != EFI_SUCCESS)
		goto out_bad_signal;

	token->status = EFI_SUCCESS;
	goto out_signal;

out_bad_signal:
	token->status = EFI_ABORTED;
out_signal:
	efi_signal_event(token->event);
out:
	return EFI_EXIT(ret);
}

/*
 * efi_http_poll() -  Polls for incoming data packets and processes outgoing data packets
 *
 * This function implements EFI_HTTP_PROTOCOL.Poll().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @token:	pointer to storage containing HTTP request token
 * Return:	status code
 */
static efi_status_t EFIAPI efi_http_poll(struct efi_http_protocol *this)
{
	EFI_ENTRY("%p", this);

	efi_status_t ret = EFI_UNSUPPORTED;

	return EFI_EXIT(ret);
}

/* EFI_HTTP_SERVICE_BINDING_PROTOCOL */

/*
 * efi_http_service_binding_create_child() -  Creates a child handle
 * and installs a protocol
 *
 * This function implements EFI_HTTP_SERVICE_BINDING.CreateChild().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @child_handle:	pointer to child handle
 * Return:		status code
 */
static efi_status_t EFIAPI efi_http_service_binding_create_child(
			struct efi_service_binding_protocol *this,
			efi_handle_t *child_handle)
{
	EFI_ENTRY("%p, %p", this, child_handle);

	efi_status_t ret = EFI_SUCCESS;
	struct efi_http_instance *new_instance;

	if (!child_handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	new_instance = calloc(1, sizeof(struct efi_http_instance));
	if (!new_instance) {
		ret = EFI_OUT_OF_RESOURCES;
		goto failure_to_add_protocol;
	}

	if (*child_handle) {
		new_instance->handle = *child_handle;
		goto install;
	}

	new_instance->handle = calloc(1, sizeof(struct efi_object));
	if (!new_instance->handle) {
		efi_free_pool((void *)new_instance);
		ret = EFI_OUT_OF_RESOURCES;
		goto failure_to_add_protocol;
	}

	new_instance->parent = this;
	efi_add_handle(new_instance->handle);
	*child_handle = new_instance->handle;

install:
	ret = efi_add_protocol(new_instance->handle, &efi_http_guid,
			       &new_instance->http);
	if (ret != EFI_SUCCESS)
		goto failure_to_add_protocol;

	new_instance->http.get_mode_data = efi_http_get_mode_data;
	new_instance->http.configure = efi_http_configure;
	new_instance->http.request = efi_http_request;
	new_instance->http.cancel = efi_http_cancel;
	new_instance->http.response = efi_http_response;
	new_instance->http.poll = efi_http_poll;
	++num_instances;

	return EFI_EXIT(EFI_SUCCESS);

failure_to_add_protocol:
	return EFI_EXIT(ret);
}

/*
 * efi_http_service_binding_destroy_child() -  Destroys a child handle with
 * a protocol installed on it
 *
 * This function implements EFI_HTTP_SERVICE_BINDING.DestroyChild().
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @child_handle:	child handle
 * Return:		status code
 */
static efi_status_t EFIAPI efi_http_service_binding_destroy_child(
			struct efi_service_binding_protocol *this,
			efi_handle_t child_handle)
{
	EFI_ENTRY("%p, %p", this, child_handle);
	efi_status_t ret = EFI_SUCCESS;
	struct efi_http_instance *http_instance;
	struct efi_handler *phandler;

	if (num_instances == 0)
		return EFI_EXIT(EFI_NOT_FOUND);

	if (!child_handle)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	efi_search_protocol(child_handle, &efi_http_guid, &phandler);

	if (!phandler)
		return EFI_EXIT(EFI_UNSUPPORTED);

	ret = efi_delete_handle(child_handle);
	if (ret != EFI_SUCCESS)
		return EFI_EXIT(ret);

	http_instance = phandler->protocol_interface;
	efi_free_pool(http_instance->http_load_addr);
	http_instance->http_load_addr = NULL;

	free(phandler->protocol_interface);

	num_instances--;

	return EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_http_register() - register the http protocol
 *
 */
efi_status_t efi_http_register(const efi_handle_t handle,
			       struct efi_service_binding_protocol *http_service_binding)
{
	efi_status_t r = EFI_SUCCESS;

	r = efi_add_protocol(handle, &efi_http_service_binding_guid,
			     http_service_binding);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;

	http_service_binding->create_child = efi_http_service_binding_create_child;
	http_service_binding->destroy_child = efi_http_service_binding_destroy_child;

	return EFI_SUCCESS;
failure_to_add_protocol:
	return r;
}

enum efi_http_status_code efi_u32_to_httpstatus(u32 status)
{
	switch (status) {
	case 100: return HTTP_STATUS_100_CONTINUE;
	case 101: return HTTP_STATUS_101_SWITCHING_PROTOCOLS;
	case 200: return HTTP_STATUS_200_OK;
	case 201: return HTTP_STATUS_201_CREATED;
	case 202: return HTTP_STATUS_202_ACCEPTED;
	case 203: return HTTP_STATUS_203_NON_AUTHORITATIVE_INFORMATION;
	case 204: return HTTP_STATUS_204_NO_CONTENT;
	case 205: return HTTP_STATUS_205_RESET_CONTENT;
	case 206: return HTTP_STATUS_206_PARTIAL_CONTENT;
	case 300: return HTTP_STATUS_300_MULTIPLE_CHOICES;
	case 301: return HTTP_STATUS_301_MOVED_PERMANENTLY;
	case 302: return HTTP_STATUS_302_FOUND;
	case 303: return HTTP_STATUS_303_SEE_OTHER;
	case 304: return HTTP_STATUS_304_NOT_MODIFIED;
	case 305: return HTTP_STATUS_305_USE_PROXY;
	case 307: return HTTP_STATUS_307_TEMPORARY_REDIRECT;
	case 400: return HTTP_STATUS_400_BAD_REQUEST;
	case 401: return HTTP_STATUS_401_UNAUTHORIZED;
	case 402: return HTTP_STATUS_402_PAYMENT_REQUIRED;
	case 403: return HTTP_STATUS_403_FORBIDDEN;
	case 404: return HTTP_STATUS_404_NOT_FOUND;
	case 405: return HTTP_STATUS_405_METHOD_NOT_ALLOWED;
	case 406: return HTTP_STATUS_406_NOT_ACCEPTABLE;
	case 407: return HTTP_STATUS_407_PROXY_AUTHENTICATION_REQUIRED;
	case 408: return HTTP_STATUS_408_REQUEST_TIME_OUT;
	case 409: return HTTP_STATUS_409_CONFLICT;
	case 410: return HTTP_STATUS_410_GONE;
	case 411: return HTTP_STATUS_411_LENGTH_REQUIRED;
	case 412: return HTTP_STATUS_412_PRECONDITION_FAILED;
	case 413: return HTTP_STATUS_413_REQUEST_ENTITY_TOO_LARGE;
	case 414: return HTTP_STATUS_414_REQUEST_URI_TOO_LARGE;
	case 415: return HTTP_STATUS_415_UNSUPPORTED_MEDIA_TYPE;
	case 416: return HTTP_STATUS_416_REQUESTED_RANGE_NOT_SATISFIED;
	case 417: return HTTP_STATUS_417_EXPECTATION_FAILED;
	case 500: return HTTP_STATUS_500_INTERNAL_SERVER_ERROR;
	case 501: return HTTP_STATUS_501_NOT_IMPLEMENTED;
	case 502: return HTTP_STATUS_502_BAD_GATEWAY;
	case 503: return HTTP_STATUS_503_SERVICE_UNAVAILABLE;
	case 504: return HTTP_STATUS_504_GATEWAY_TIME_OUT;
	case 505: return HTTP_STATUS_505_HTTP_VERSION_NOT_SUPPORTED;
	case 308: return HTTP_STATUS_308_PERMANENT_REDIRECT;
	default: return HTTP_STATUS_UNSUPPORTED_STATUS;
	}
}
