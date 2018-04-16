/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Leif Lindholm
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>

const efi_guid_t efi_guid_device_path_utilities_protocol =
		EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID;

static efi_uintn_t EFIAPI get_device_path_size(
	const struct efi_device_path *device_path)
{
	efi_uintn_t sz = 0;

	EFI_ENTRY("%p", device_path);
	/* size includes the END node: */
	if (device_path)
		sz = efi_dp_size(device_path) + sizeof(struct efi_device_path);
	return EFI_EXIT(sz);
}

static struct efi_device_path * EFIAPI duplicate_device_path(
	const struct efi_device_path *device_path)
{
	EFI_ENTRY("%p", device_path);
	return EFI_EXIT(efi_dp_dup(device_path));
}

static struct efi_device_path * EFIAPI append_device_path(
	const struct efi_device_path *src1,
	const struct efi_device_path *src2)
{
	EFI_ENTRY("%p, %p", src1, src2);
	return EFI_EXIT(efi_dp_append(src1, src2));
}

static struct efi_device_path * EFIAPI append_device_node(
	const struct efi_device_path *device_path,
	const struct efi_device_path *device_node)
{
	EFI_ENTRY("%p, %p", device_path, device_node);
	return EFI_EXIT(efi_dp_append_node(device_path, device_node));
}

static struct efi_device_path * EFIAPI append_device_path_instance(
	const struct efi_device_path *device_path,
	const struct efi_device_path *device_path_instance)
{
	EFI_ENTRY("%p, %p", device_path, device_path_instance);
	return EFI_EXIT(NULL);
}

static struct efi_device_path * EFIAPI get_next_device_path_instance(
	struct efi_device_path **device_path_instance,
	efi_uintn_t *device_path_instance_size)
{
	EFI_ENTRY("%p, %p", device_path_instance, device_path_instance_size);
	return EFI_EXIT(NULL);
}

static bool EFIAPI is_device_path_multi_instance(
	const struct efi_device_path *device_path)
{
	EFI_ENTRY("%p", device_path);
	return EFI_EXIT(false);
}

/*
 * Create device node.
 *
 * This function implements the CreateDeviceNode service of the device path
 * utilities protocol.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification
 * for details.
 *
 * @node_type		node type
 * @node_sub_type	node sub type
 * @node_length		node length
 * @return		device path node
 */
static struct efi_device_path * EFIAPI create_device_node(
	uint8_t node_type, uint8_t node_sub_type, uint16_t node_length)
{
	EFI_ENTRY("%u, %u, %u", node_type, node_sub_type, node_length);
	return EFI_EXIT(efi_dp_create_device_node(node_type, node_sub_type,
			node_length));
}

const struct efi_device_path_utilities_protocol efi_device_path_utilities = {
	.get_device_path_size = get_device_path_size,
	.duplicate_device_path = duplicate_device_path,
	.append_device_path = append_device_path,
	.append_device_node = append_device_node,
	.append_device_path_instance = append_device_path_instance,
	.get_next_device_path_instance = get_next_device_path_instance,
	.is_device_path_multi_instance = is_device_path_multi_instance,
	.create_device_node = create_device_node,
};
