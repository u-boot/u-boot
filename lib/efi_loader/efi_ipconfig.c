// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Implementation of EFI_IP4_CONFIG2_PROTOCOL
 *
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>

static const efi_guid_t efi_ip4_config2_guid = EFI_IP4_CONFIG2_PROTOCOL_GUID;

struct efi_ip4_config2_manual_address current_http_ip;
static enum efi_ip4_config2_policy current_policy;
static char current_mac_addr[32];

/* EFI_IP4_CONFIG2_PROTOCOL */

/*
 * efi_ip4_config2_set_data() -  Set the configuration for the EFI IPv4 network
 * stack running on the communication device
 *
 * This function implements EFI_IP4_CONFIG2_PROTOCOL.SetData()
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @data_type:		the type of data to set
 * @data_size:		size of the buffer pointed to by data in bytes
 * @data:		the data buffer to set
 * Return:		status code
 */
static efi_status_t EFIAPI efi_ip4_config2_set_data(struct efi_ip4_config2_protocol *this,
						    enum efi_ip4_config2_data_type data_type,
						    efi_uintn_t data_size,
						    void *data)
{
	EFI_ENTRY("%p, %d, %zu, %p", this, data_type, data_size, data);
	efi_status_t ret = EFI_SUCCESS;

	if (!this || (data && !data_size) || (!data && data_size))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	switch (data_type) {
	case EFI_IP4_CONFIG2_DATA_TYPE_INTERFACEINFO:
		return EFI_EXIT(EFI_WRITE_PROTECTED);
	case EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS:
		if (current_policy != EFI_IP4_CONFIG2_POLICY_STATIC)
			return EFI_EXIT(EFI_WRITE_PROTECTED);
		if (!data_size && !data) {
			memset((void *)&current_http_ip, 0,
			       sizeof(current_http_ip));
			return EFI_EXIT(EFI_SUCCESS);
		}
		if (data && data_size == sizeof(struct efi_ip4_config2_manual_address)) {
			memcpy((void *)&current_http_ip, data,
			       sizeof(struct efi_ip4_config2_manual_address));
			efi_net_set_addr(&current_http_ip.address,
					 &current_http_ip.subnet_mask, NULL, NULL);
			return EFI_EXIT(EFI_SUCCESS);
		}
		return EFI_EXIT(EFI_BAD_BUFFER_SIZE);
	case EFI_IP4_CONFIG2_DATA_TYPE_POLICY:
		if (data && data_size == sizeof(enum efi_ip4_config2_policy)) {
			current_policy = *(enum efi_ip4_config2_policy *)data;
			return EFI_EXIT(EFI_SUCCESS);
		}
		return EFI_EXIT(EFI_BAD_BUFFER_SIZE);

	default:
		return EFI_EXIT(EFI_UNSUPPORTED);
	}

	return EFI_EXIT(ret);
}

/*
 * efi_ip4_config2_get_data() -  Get the configuration for the EFI IPv4 network
 * stack running on the communication device
 *
 * This function implements EFI_IP4_CONFIG2_PROTOCOL.GetData()
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @data_type:		the type of data to get
 * @data_size:		size
 * @data:		the data buffer
 * Return:		status code
 */
static efi_status_t EFIAPI efi_ip4_config2_get_data(struct efi_ip4_config2_protocol *this,
						    enum efi_ip4_config2_data_type data_type,
						    efi_uintn_t *data_size,
						    void *data)
{
	EFI_ENTRY("%p, %d, %p, %p", this, data_type, data_size, data);

	efi_status_t ret = EFI_SUCCESS;
	struct efi_ip4_config2_interface_info *info;
	int tmp;

	if (!this || !data_size)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (*data_size && !data)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	tmp = sizeof(struct efi_ip4_config2_interface_info) + sizeof(struct efi_ip4_route_table);

	switch (data_type) {
	case EFI_IP4_CONFIG2_DATA_TYPE_INTERFACEINFO:
		if (*data_size < tmp) {
			*data_size = tmp;
			return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
		}

		info = (struct efi_ip4_config2_interface_info *)data;
		memset(info, 0, sizeof(*info));

		info->hw_address_size = 6;
		memcpy(info->hw_address.mac_addr, current_mac_addr, 6);
		// Set the route table size

		info->route_table_size = 0;
		break;
	case EFI_IP4_CONFIG2_DATA_TYPE_MANUAL_ADDRESS:
		if (*data_size < sizeof(struct efi_ip4_config2_manual_address)) {
			*data_size = sizeof(struct efi_ip4_config2_manual_address);
			return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
		}

		efi_net_get_addr(&current_http_ip.address, &current_http_ip.subnet_mask, NULL, NULL);
		memcpy(data, (void *)&current_http_ip,
		       sizeof(struct efi_ip4_config2_manual_address));

		break;
	default:
		return EFI_EXIT(EFI_NOT_FOUND);
	}
	return EFI_EXIT(ret);
}

/*
 * efi_ip4_config2_register_notify() -  Register an event that is to be signaled whenever
 * a configuration process on the specified configuration
 * data is done
 *
 * This function implements EFI_IP4_CONFIG2_PROTOCOL.RegisterDataNotify()
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @data_type:		the type of data to register the event for
 * @event:		the event to register
 * Return:		status code
 */
static efi_status_t EFIAPI efi_ip4_config2_register_notify(struct efi_ip4_config2_protocol *this,
							   enum efi_ip4_config2_data_type data_type,
							   struct efi_event *event)
{
	EFI_ENTRY("%p, %d, %p", this, data_type, event);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_ip4_config2_unregister_notify() -  Remove a previously registered eventfor
 * the specified configuration data
 *
 * This function implements EFI_IP4_CONFIG2_PROTOCOL.UnregisterDataNotify()
 * See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @data_type:		the type of data to remove the event for
 * @event:		the event to unregister
 * Return:		status code
 */
static efi_status_t EFIAPI efi_ip4_config2_unregister_notify(struct efi_ip4_config2_protocol *this,
							     enum efi_ip4_config2_data_type data_type,
							     struct efi_event *event)
{
	EFI_ENTRY("%p, %d, %p", this, data_type, event);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_ipconfig_register() - register the ip4_config2 protocol
 *
 */
efi_status_t efi_ipconfig_register(const efi_handle_t handle,
				   struct efi_ip4_config2_protocol *ip4config)
{
	efi_status_t r = EFI_SUCCESS;

	r = efi_add_protocol(handle, &efi_ip4_config2_guid,
			     ip4config);
	if (r != EFI_SUCCESS) {
		log_err("ERROR: Failure to add protocol\n");
		return r;
	}

	memcpy(current_mac_addr, eth_get_ethaddr(), 6);

	ip4config->set_data = efi_ip4_config2_set_data;
	ip4config->get_data = efi_ip4_config2_get_data;
	ip4config->register_data_notify = efi_ip4_config2_register_notify;
	ip4config->unregister_data_notify = efi_ip4_config2_unregister_notify;

	return EFI_SUCCESS;
}
