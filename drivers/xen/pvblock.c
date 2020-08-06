// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2007-2008 Samuel Thibault.
 * (C) Copyright 2020 EPAM Systems Inc.
 */
#include <blk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <part.h>

#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/xen/system.h>

#include <linux/compat.h>

#include <xen/events.h>
#include <xen/gnttab.h>
#include <xen/hvm.h>
#include <xen/xenbus.h>

#include <xen/interface/io/ring.h>
#include <xen/interface/io/blkif.h>
#include <xen/interface/io/protocols.h>

#define DRV_NAME	"pvblock"
#define DRV_NAME_BLK	"pvblock_blk"

#define O_RDONLY	00
#define O_RDWR		02

struct blkfront_info {
	u64 sectors;
	unsigned int sector_size;
	int mode;
	int info;
	int barrier;
	int flush;
};

/**
 * struct blkfront_dev - Struct representing blkfront device
 * @dom: Domain id
 * @ring: Front_ring structure
 * @ring_ref: The grant reference, allowing us to grant access
 *	      to the ring to the other end/domain
 * @evtchn: Event channel used to signal ring events
 * @handle: Events handle
 * @nodename: Device XenStore path in format "device/vbd/" + @devid
 * @backend: Backend XenStore path
 * @info: Private data
 * @devid: Device id
 */
struct blkfront_dev {
	domid_t dom;

	struct blkif_front_ring ring;
	grant_ref_t ring_ref;
	evtchn_port_t evtchn;
	blkif_vdev_t handle;

	char *nodename;
	char *backend;
	struct blkfront_info info;
	unsigned int devid;
};

struct blkfront_platdata {
	unsigned int devid;
};

static void free_blkfront(struct blkfront_dev *dev)
{
	mask_evtchn(dev->evtchn);
	free(dev->backend);

	gnttab_end_access(dev->ring_ref);
	free(dev->ring.sring);

	unbind_evtchn(dev->evtchn);

	free(dev->nodename);
	free(dev);
}

static void blkfront_handler(evtchn_port_t port, struct pt_regs *regs,
			     void *data)
{
	printf("%s [Port %d] - event received\n", __func__, port);
}

static int init_blkfront(unsigned int devid, struct blkfront_dev *dev)
{
	xenbus_transaction_t xbt;
	char *err = NULL;
	char *message = NULL;
	struct blkif_sring *s;
	int retry = 0;
	char *msg = NULL;
	char *c;
	char nodename[32];
	char path[ARRAY_SIZE(nodename) + strlen("/backend-id") + 1];

	sprintf(nodename, "device/vbd/%d", devid);

	memset(dev, 0, sizeof(*dev));
	dev->nodename = strdup(nodename);
	dev->devid = devid;

	snprintf(path, sizeof(path), "%s/backend-id", nodename);
	dev->dom = xenbus_read_integer(path);
	evtchn_alloc_unbound(dev->dom, blkfront_handler, dev, &dev->evtchn);

	s = (struct blkif_sring *)memalign(PAGE_SIZE, PAGE_SIZE);
	if (!s) {
		printf("Failed to allocate shared ring\n");
		goto error;
	}

	SHARED_RING_INIT(s);
	FRONT_RING_INIT(&dev->ring, s, PAGE_SIZE);

	dev->ring_ref = gnttab_grant_access(dev->dom, virt_to_pfn(s), 0);

again:
	err = xenbus_transaction_start(&xbt);
	if (err) {
		printf("starting transaction\n");
		free(err);
	}

	err = xenbus_printf(xbt, nodename, "ring-ref", "%u", dev->ring_ref);
	if (err) {
		message = "writing ring-ref";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, nodename, "event-channel", "%u", dev->evtchn);
	if (err) {
		message = "writing event-channel";
		goto abort_transaction;
	}
	err = xenbus_printf(xbt, nodename, "protocol", "%s",
			    XEN_IO_PROTO_ABI_NATIVE);
	if (err) {
		message = "writing protocol";
		goto abort_transaction;
	}

	snprintf(path, sizeof(path), "%s/state", nodename);
	err = xenbus_switch_state(xbt, path, XenbusStateConnected);
	if (err) {
		message = "switching state";
		goto abort_transaction;
	}

	err = xenbus_transaction_end(xbt, 0, &retry);
	free(err);
	if (retry) {
		goto again;
		printf("completing transaction\n");
	}

	goto done;

abort_transaction:
	free(err);
	err = xenbus_transaction_end(xbt, 1, &retry);
	printf("Abort transaction %s\n", message);
	goto error;

done:
	snprintf(path, sizeof(path), "%s/backend", nodename);
	msg = xenbus_read(XBT_NIL, path, &dev->backend);
	if (msg) {
		printf("Error %s when reading the backend path %s\n",
		       msg, path);
		goto error;
	}

	dev->handle = strtoul(strrchr(nodename, '/') + 1, NULL, 0);

	{
		XenbusState state;
		char path[strlen(dev->backend) +
			strlen("/feature-flush-cache") + 1];

		snprintf(path, sizeof(path), "%s/mode", dev->backend);
		msg = xenbus_read(XBT_NIL, path, &c);
		if (msg) {
			printf("Error %s when reading the mode\n", msg);
			goto error;
		}
		if (*c == 'w')
			dev->info.mode = O_RDWR;
		else
			dev->info.mode = O_RDONLY;
		free(c);

		snprintf(path, sizeof(path), "%s/state", dev->backend);

		msg = NULL;
		state = xenbus_read_integer(path);
		while (!msg && state < XenbusStateConnected)
			msg = xenbus_wait_for_state_change(path, &state);
		if (msg || state != XenbusStateConnected) {
			printf("backend not available, state=%d\n", state);
			goto error;
		}

		snprintf(path, sizeof(path), "%s/info", dev->backend);
		dev->info.info = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/sectors", dev->backend);
		/*
		 * FIXME: read_integer returns an int, so disk size
		 * limited to 1TB for now
		 */
		dev->info.sectors = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/sector-size", dev->backend);
		dev->info.sector_size = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/feature-barrier",
			 dev->backend);
		dev->info.barrier = xenbus_read_integer(path);

		snprintf(path, sizeof(path), "%s/feature-flush-cache",
			 dev->backend);
		dev->info.flush = xenbus_read_integer(path);
	}
	unmask_evtchn(dev->evtchn);

	debug("%llu sectors of %u bytes\n",
	      dev->info.sectors, dev->info.sector_size);

	return 0;

error:
	free(msg);
	free(err);
	free_blkfront(dev);
	return -ENODEV;
}

static void shutdown_blkfront(struct blkfront_dev *dev)
{
	char *err = NULL, *err2;
	XenbusState state;

	char path[strlen(dev->backend) + strlen("/state") + 1];
	char nodename[strlen(dev->nodename) + strlen("/event-channel") + 1];

	debug("Close " DRV_NAME ", device ID %d\n", dev->devid);

	snprintf(path, sizeof(path), "%s/state", dev->backend);
	snprintf(nodename, sizeof(nodename), "%s/state", dev->nodename);

	if ((err = xenbus_switch_state(XBT_NIL, nodename,
				       XenbusStateClosing)) != NULL) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateClosing, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (!err && state < XenbusStateClosing)
		err = xenbus_wait_for_state_change(path, &state);
	free(err);

	if ((err = xenbus_switch_state(XBT_NIL, nodename,
				       XenbusStateClosed)) != NULL) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateClosed, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (state < XenbusStateClosed) {
		err = xenbus_wait_for_state_change(path, &state);
		free(err);
	}

	if ((err = xenbus_switch_state(XBT_NIL, nodename,
				       XenbusStateInitialising)) != NULL) {
		printf("%s: error changing state to %d: %s\n", __func__,
		       XenbusStateInitialising, err);
		goto close;
	}

	state = xenbus_read_integer(path);
	while (!err &&
	       (state < XenbusStateInitWait || state >= XenbusStateClosed))
		err = xenbus_wait_for_state_change(path, &state);

close:
	free(err);

	snprintf(nodename, sizeof(nodename), "%s/ring-ref", dev->nodename);
	err2 = xenbus_rm(XBT_NIL, nodename);
	free(err2);
	snprintf(nodename, sizeof(nodename), "%s/event-channel", dev->nodename);
	err2 = xenbus_rm(XBT_NIL, nodename);
	free(err2);

	if (!err)
		free_blkfront(dev);
}

ulong pvblock_blk_read(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
{
	return 0;
}

ulong pvblock_blk_write(struct udevice *udev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
{
	return 0;
}

static int pvblock_blk_bind(struct udevice *udev)
{
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	int devnum;

	desc->if_type = IF_TYPE_PVBLOCK;
	/*
	 * Initialize the devnum to -ENODEV. This is to make sure that
	 * blk_next_free_devnum() works as expected, since the default
	 * value 0 is a valid devnum.
	 */
	desc->devnum = -ENODEV;
	devnum = blk_next_free_devnum(IF_TYPE_PVBLOCK);
	if (devnum < 0)
		return devnum;
	desc->devnum = devnum;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = udev;

	strncpy(desc->vendor, "Xen", sizeof(desc->vendor));
	strncpy(desc->revision, "1", sizeof(desc->revision));
	strncpy(desc->product, "Virtual disk", sizeof(desc->product));

	return 0;
}

static int pvblock_blk_probe(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);
	struct blkfront_platdata *platdata = dev_get_platdata(udev);
	struct blk_desc *desc = dev_get_uclass_platdata(udev);
	int ret, devid;

	devid = platdata->devid;
	free(platdata);

	ret = init_blkfront(devid, blk_dev);
	if (ret < 0)
		return ret;

	desc->blksz = blk_dev->info.sector_size;
	desc->lba = blk_dev->info.sectors;
	desc->log2blksz = LOG2(blk_dev->info.sector_size);

	return 0;
}

static int pvblock_blk_remove(struct udevice *udev)
{
	struct blkfront_dev *blk_dev = dev_get_priv(udev);

	shutdown_blkfront(blk_dev);
	return 0;
}

static const struct blk_ops pvblock_blk_ops = {
	.read	= pvblock_blk_read,
	.write	= pvblock_blk_write,
};

U_BOOT_DRIVER(pvblock_blk) = {
	.name			= DRV_NAME_BLK,
	.id			= UCLASS_BLK,
	.ops			= &pvblock_blk_ops,
	.bind			= pvblock_blk_bind,
	.probe			= pvblock_blk_probe,
	.remove			= pvblock_blk_remove,
	.priv_auto_alloc_size	= sizeof(struct blkfront_dev),
	.flags			= DM_FLAG_OS_PREPARE,
};

/*******************************************************************************
 * Para-virtual block device class
 *******************************************************************************/

typedef int (*enum_vbd_callback)(struct udevice *parent, unsigned int devid);

static int on_new_vbd(struct udevice *parent, unsigned int devid)
{
	struct driver_info info;
	struct udevice *udev;
	struct blkfront_platdata *platdata;
	int ret;

	debug("New " DRV_NAME_BLK ", device ID %d\n", devid);

	platdata = malloc(sizeof(struct blkfront_platdata));
	if (!platdata) {
		printf("Failed to allocate platform data\n");
		return -ENOMEM;
	}

	platdata->devid = devid;

	info.name = DRV_NAME_BLK;
	info.platdata = platdata;

	ret = device_bind_by_name(parent, false, &info, &udev);
	if (ret < 0) {
		printf("Failed to bind " DRV_NAME_BLK " to device with ID %d, ret: %d\n",
		       devid, ret);
		free(platdata);
	}
	return ret;
}

static int xenbus_enumerate_vbd(struct udevice *udev, enum_vbd_callback clb)
{
	char **dirs, *msg;
	int i, ret;

	msg = xenbus_ls(XBT_NIL, "device/vbd", &dirs);
	if (msg) {
		printf("Failed to read device/vbd directory: %s\n", msg);
		free(msg);
		return -ENODEV;
	}

	for (i = 0; dirs[i]; i++) {
		int devid;

		sscanf(dirs[i], "%d", &devid);
		ret = clb(udev, devid);
		if (ret < 0)
			goto fail;

		free(dirs[i]);
	}
	ret = 0;

fail:
	for (; dirs[i]; i++)
		free(dirs[i]);
	free(dirs);
	return ret;
}

void pvblock_init(void)
{
	struct driver_info info;
	struct udevice *udev;
	struct uclass *uc;
	int ret;

	/*
	 * At this point Xen drivers have already initialized,
	 * so we can instantiate the class driver and enumerate
	 * virtual block devices.
	 */
	info.name = DRV_NAME;
	ret = device_bind_by_name(gd->dm_root, false, &info, &udev);
	if (ret < 0)
		printf("Failed to bind " DRV_NAME ", ret: %d\n", ret);

	/* Bootstrap virtual block devices class driver */
	ret = uclass_get(UCLASS_PVBLOCK, &uc);
	if (ret)
		return;
	uclass_foreach_dev_probe(UCLASS_PVBLOCK, udev);
}

static int pvblock_probe(struct udevice *udev)
{
	struct uclass *uc;
	int ret;

	if (xenbus_enumerate_vbd(udev, on_new_vbd) < 0)
		return -ENODEV;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_probe(UCLASS_BLK, udev) {
		if (_ret)
			return _ret;
	};
	return 0;
}

U_BOOT_DRIVER(pvblock_drv) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
	.probe		= pvblock_probe,
};

UCLASS_DRIVER(pvblock) = {
	.name		= DRV_NAME,
	.id		= UCLASS_PVBLOCK,
};
