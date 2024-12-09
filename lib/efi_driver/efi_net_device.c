// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  EFI network driver
 *
 */

#include <dm.h>
#include <efi_driver.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/tag.h>
#include <dm/uclass-internal.h>

/**
 * efi_net_bind_drv() - TODO
 *
 * @this:	driver binding protocol
 * @handle:	handle
 * @interface:	block io protocol
 * Return:	status code
 */
static efi_status_t efi_net_bind_drv(
			struct efi_driver_binding_extended_protocol *this,
			efi_handle_t handle, void *interface)
{

	EFI_PRINT("%s: handle %p, interface %p\n", __func__, handle, interface);


	return EFI_UNSUPPORTED;
}

/**
 * efi_net_init_drv() - initialize network device driver
 *
 * @this:	extended driver binding protocol
 * Return:	status code
 */
static efi_status_t
efi_net_init_drv(struct efi_driver_binding_extended_protocol *this)
{
	int ret;
	struct udevice *dev;
	struct event event;

	for (uclass_find_first_device(UCLASS_ETH, &dev); dev;
	     uclass_find_next_device(&dev)) {
		if (dev_get_flags(dev) & DM_FLAG_ACTIVATED) {
			memcpy(&event.data, &dev, sizeof(dev));
			ret = efi_net_register(NULL, &event);
			if (ret) {
				log_err("Failed registering %s in EFI net\n", dev->name);
				return EFI_OUT_OF_RESOURCES;
			}
		}

	}

	ret = event_register("efi_net register", EVT_DM_POST_PROBE,
			     efi_net_register, this);
	if (ret) {
		log_err("Event registration for efi_net register failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	ret = event_register("efi_net unregister", EVT_DM_PRE_REMOVE,
			     efi_net_unregister, this);
	if (ret) {
		log_err("Event registration for efi_net unregister failed\n");
		return EFI_OUT_OF_RESOURCES;
	}

	return EFI_SUCCESS;
}

/* EFI driver operators */
static const struct efi_driver_ops driver_ops = {
	.protocol	= &efi_net_guid,
	.init		= efi_net_init_drv,
	.bind		= efi_net_bind_drv,
};

/* Identify as EFI driver */
U_BOOT_DRIVER(efi_net) = {
	.name		= "efi_net",
	.id		= UCLASS_EFI_LOADER,
	.ops		= &driver_ops,
};
