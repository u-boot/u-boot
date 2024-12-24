// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  EFI network driver
 *
 */

#include <dm.h>
#include <efi_driver.h>
#include <malloc.h>
#include <net.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/tag.h>
#include <dm/uclass-internal.h>

#define DEFAULT_EFI_NET_BUFFER_SIZE 1024

int efi_net_start(struct udevice *dev)
{
	struct efi_netdev_plat *plat;
	efi_status_t ret;

	plat = dev_get_plat(dev);
	if (!plat || !plat->snp)
		return -1;

	ret = plat->snp->start(plat->snp);
	if (ret != EFI_SUCCESS)
		return -1;
	ret = plat->snp->initialize(plat->snp, 0, 0);
	if (ret != EFI_SUCCESS)
		return -1;

	return 0;
}

int efi_net_send(struct udevice *dev, void *packet, int length)
{
	struct efi_netdev_plat *plat;
	efi_status_t ret;

	plat = dev_get_plat(dev);
	if (!plat || !plat->snp)
		return -1;

	ret = plat->snp->transmit(plat->snp, 0, length, packet, NULL, NULL, NULL);
	if (ret != EFI_SUCCESS)
		return -1;

	return 0;
}

int efi_net_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct efi_netdev_plat *plat;
	efi_status_t ret;
	size_t buffer_size;

	plat = dev_get_plat(dev);
	if (!plat || !plat->snp)
		return -1;

	if (plat->buffer)
		free(plat->buffer);

	buffer_size = DEFAULT_EFI_NET_BUFFER_SIZE;
	plat->buffer = calloc(1, buffer_size);
	ret = plat->snp->receive(plat->snp, NULL, &buffer_size, plat->buffer, NULL, NULL, NULL);

	if (ret == EFI_BUFFER_TOO_SMALL) {
		free(plat->buffer);
		plat->buffer = calloc(1, buffer_size);
		ret = plat->snp->receive(plat->snp, NULL, &buffer_size, plat->buffer, NULL, NULL, NULL);
	}

	if (ret != EFI_SUCCESS || ret != EFI_NOT_READY)
		return -1;

	*packetp = plat->buffer;
	return buffer_size;
}

void efi_net_stop(struct udevice *dev)
{
	struct efi_netdev_plat *plat;

	plat = dev_get_plat(dev);
	if (!plat || !plat->snp)
		return;

	plat->snp->stop(plat->snp);
}

/**
 * efi_netdev_create() - create a net udevice for a handle
 *
 * @handle:	handle
 * @interface:	simple network protocol
 * Return:	status code
 */
static efi_status_t
efi_netdev_create(efi_handle_t handle, void *interface)
{
	struct udevice *dev = NULL, *parent = dm_root();
	efi_status_t ret;
	char *name;
	struct efi_netdev_plat *plat;
	static int devnum = 0;

	name = calloc(1, 18); /* strlen("efinet#2147483648") + 1 */
	if (!name)
		return EFI_OUT_OF_RESOURCES;
	sprintf(name, "efinet#%d", devnum);
	devnum++;

	/* Create driver model udevice for the EFI block io device */
	if (eth_create_device(parent, "efi_netdev", name, &dev)) {
		ret = EFI_OUT_OF_RESOURCES;
		free(name);
		goto err;
	}

	plat = dev_get_plat(dev);
	plat->handle = handle;
	plat->snp = interface;

	if (efi_link_dev(handle, dev)) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	if (device_probe(dev)) {
		ret = EFI_DEVICE_ERROR;
		goto err;
	}
	EFI_PRINT("%s: net udevice '%s' created\n", __func__, dev->name);

	return EFI_SUCCESS;

err:
	efi_unlink_dev(handle);
	if (dev)
		device_unbind(dev);

	return ret;
}

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

	efi_status_t ret;

	if (!handle->dev) {
		ret = efi_netdev_create(handle, interface);
		if (ret != EFI_SUCCESS)
			return ret;
	}

	return EFI_SUCCESS;
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

/* Net device driver operators */
static const struct eth_ops efi_eth_ops = {
	.start	= efi_net_start,
	.send	= efi_net_send,
	.recv	= efi_net_recv,
	.stop	= efi_net_stop,
};

/* Identify as net device driver */
U_BOOT_DRIVER(efi_netdev) = {
	.name		= "efi_netdev",
	.id		= UCLASS_ETH,
	.ops		= &efi_eth_ops,
	.plat_auto	= sizeof(struct efi_netdev_plat),
};

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
