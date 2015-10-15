/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <environment.h>
#include <net.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/errno.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

void eth_parse_enetaddr(const char *addr, uchar *enetaddr)
{
	char *end;
	int i;

	for (i = 0; i < 6; ++i) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end + 1 : end;
	}
}

int eth_getenv_enetaddr(char *name, uchar *enetaddr)
{
	eth_parse_enetaddr(getenv(name), enetaddr);
	return is_valid_ethaddr(enetaddr);
}

int eth_setenv_enetaddr(char *name, const uchar *enetaddr)
{
	char buf[20];

	sprintf(buf, "%pM", enetaddr);

	return setenv(name, buf);
}

int eth_getenv_enetaddr_by_index(const char *base_name, int index,
				 uchar *enetaddr)
{
	char enetvar[32];
	sprintf(enetvar, index ? "%s%daddr" : "%saddr", base_name, index);
	return eth_getenv_enetaddr(enetvar, enetaddr);
}

static inline int eth_setenv_enetaddr_by_index(const char *base_name, int index,
				 uchar *enetaddr)
{
	char enetvar[32];
	sprintf(enetvar, index ? "%s%daddr" : "%saddr", base_name, index);
	return eth_setenv_enetaddr(enetvar, enetaddr);
}

static int eth_mac_skip(int index)
{
	char enetvar[15];
	char *skip_state;

	sprintf(enetvar, index ? "eth%dmacskip" : "ethmacskip", index);
	skip_state = getenv(enetvar);
	return skip_state != NULL;
}

static void eth_current_changed(void);

/*
 * CPU and board-specific Ethernet initializations.  Aliased function
 * signals caller to move on
 */
static int __def_eth_init(bd_t *bis)
{
	return -1;
}
int cpu_eth_init(bd_t *bis) __attribute__((weak, alias("__def_eth_init")));
int board_eth_init(bd_t *bis) __attribute__((weak, alias("__def_eth_init")));

static void eth_common_init(void)
{
	bootstage_mark(BOOTSTAGE_ID_NET_ETH_START);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) || defined(CONFIG_PHYLIB)
	miiphy_init();
#endif

#ifdef CONFIG_PHYLIB
	phy_init();
#endif

	/*
	 * If board-specific initialization exists, call it.
	 * If not, call a CPU-specific one
	 */
	if (board_eth_init != __def_eth_init) {
		if (board_eth_init(gd->bd) < 0)
			printf("Board Net Initialization Failed\n");
	} else if (cpu_eth_init != __def_eth_init) {
		if (cpu_eth_init(gd->bd) < 0)
			printf("CPU Net Initialization Failed\n");
	} else {
#ifndef CONFIG_DM_ETH
		printf("Net Initialization Skipped\n");
#endif
	}
}

#ifdef CONFIG_DM_ETH
/**
 * struct eth_device_priv - private structure for each Ethernet device
 *
 * @state: The state of the Ethernet MAC driver (defined by enum eth_state_t)
 */
struct eth_device_priv {
	enum eth_state_t state;
};

/**
 * struct eth_uclass_priv - The structure attached to the uclass itself
 *
 * @current: The Ethernet device that the network functions are using
 */
struct eth_uclass_priv {
	struct udevice *current;
};

/* eth_errno - This stores the most recent failure code from DM functions */
static int eth_errno;

static struct eth_uclass_priv *eth_get_uclass_priv(void)
{
	struct uclass *uc;

	uclass_get(UCLASS_ETH, &uc);
	assert(uc);
	return uc->priv;
}

static void eth_set_current_to_next(void)
{
	struct eth_uclass_priv *uc_priv;

	uc_priv = eth_get_uclass_priv();
	if (uc_priv->current)
		uclass_next_device(&uc_priv->current);
	if (!uc_priv->current)
		uclass_first_device(UCLASS_ETH, &uc_priv->current);
}

/*
 * Typically this will simply return the active device.
 * In the case where the most recent active device was unset, this will attempt
 * to return the first device. If that device doesn't exist or fails to probe,
 * this function will return NULL.
 */
struct udevice *eth_get_dev(void)
{
	struct eth_uclass_priv *uc_priv;

	uc_priv = eth_get_uclass_priv();
	if (!uc_priv->current)
		eth_errno = uclass_first_device(UCLASS_ETH,
				    &uc_priv->current);
	return uc_priv->current;
}

/*
 * Typically this will just store a device pointer.
 * In case it was not probed, we will attempt to do so.
 * dev may be NULL to unset the active device.
 */
static void eth_set_dev(struct udevice *dev)
{
	if (dev && !device_active(dev))
		eth_errno = device_probe(dev);
	eth_get_uclass_priv()->current = dev;
}

/*
 * Find the udevice that either has the name passed in as devname or has an
 * alias named devname.
 */
struct udevice *eth_get_dev_by_name(const char *devname)
{
	int seq = -1;
	char *endp = NULL;
	const char *startp = NULL;
	struct udevice *it;
	struct uclass *uc;
	int len = strlen("eth");

	/* Must be longer than 3 to be an alias */
	if (!strncmp(devname, "eth", len) && strlen(devname) > len) {
		startp = devname + len;
		seq = simple_strtoul(startp, &endp, 10);
	}

	uclass_get(UCLASS_ETH, &uc);
	uclass_foreach_dev(it, uc) {
		/*
		 * We need the seq to be valid, so try to probe it.
		 * If the probe fails, the seq will not match since it will be
		 * -1 instead of what we are looking for.
		 * We don't care about errors from probe here. Either they won't
		 * match an alias or it will match a literal name and we'll pick
		 * up the error when we try to probe again in eth_set_dev().
		 */
		device_probe(it);
		/*
		 * Check for the name or the sequence number to match
		 */
		if (strcmp(it->name, devname) == 0 ||
		    (endp > startp && it->seq == seq))
			return it;
	}

	return NULL;
}

unsigned char *eth_get_ethaddr(void)
{
	struct eth_pdata *pdata;

	if (eth_get_dev()) {
		pdata = eth_get_dev()->platdata;
		return pdata->enetaddr;
	}

	return NULL;
}

/* Set active state without calling start on the driver */
int eth_init_state_only(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current || !device_active(current))
		return -EINVAL;

	priv = current->uclass_priv;
	priv->state = ETH_STATE_ACTIVE;

	return 0;
}

/* Set passive state without calling stop on the driver */
void eth_halt_state_only(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current || !device_active(current))
		return;

	priv = current->uclass_priv;
	priv->state = ETH_STATE_PASSIVE;
}

int eth_get_dev_index(void)
{
	if (eth_get_dev())
		return eth_get_dev()->seq;
	return -1;
}

static int eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev->platdata;
	int ret = 0;

	if (!dev || !device_active(dev))
		return -EINVAL;

	/* seq is valid since the device is active */
	if (eth_get_ops(dev)->write_hwaddr && !eth_mac_skip(dev->seq)) {
		if (!is_valid_ethaddr(pdata->enetaddr)) {
			printf("\nError: %s address %pM illegal value\n",
			       dev->name, pdata->enetaddr);
			return -EINVAL;
		}

		/*
		 * Drivers are allowed to decide not to implement this at
		 * run-time. E.g. Some devices may use it and some may not.
		 */
		ret = eth_get_ops(dev)->write_hwaddr(dev);
		if (ret == -ENOSYS)
			ret = 0;
		if (ret)
			printf("\nWarning: %s failed to set MAC address\n",
			       dev->name);
	}

	return ret;
}

static int on_ethaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	int index;
	int retval;
	struct udevice *dev;

	/* look for an index after "eth" */
	index = simple_strtoul(name + 3, NULL, 10);

	retval = uclass_find_device_by_seq(UCLASS_ETH, index, false, &dev);
	if (!retval) {
		struct eth_pdata *pdata = dev->platdata;
		switch (op) {
		case env_op_create:
		case env_op_overwrite:
			eth_parse_enetaddr(value, pdata->enetaddr);
			break;
		case env_op_delete:
			memset(pdata->enetaddr, 0, 6);
		}
	}

	return 0;
}
U_BOOT_ENV_CALLBACK(ethaddr, on_ethaddr);

int eth_init(void)
{
	struct udevice *current;
	struct udevice *old_current;
	int ret = -ENODEV;

	current = eth_get_dev();
	if (!current) {
		printf("No ethernet found.\n");
		return -ENODEV;
	}

	old_current = current;
	do {
		debug("Trying %s\n", current->name);

		if (device_active(current)) {
			ret = eth_get_ops(current)->start(current);
			if (ret >= 0) {
				struct eth_device_priv *priv =
					current->uclass_priv;

				priv->state = ETH_STATE_ACTIVE;
				return 0;
			}
		} else {
			ret = eth_errno;
		}

		debug("FAIL\n");

		/*
		 * If ethrotate is enabled, this will change "current",
		 * otherwise we will drop out of this while loop immediately
		 */
		eth_try_another(0);
		/* This will ensure the new "current" attempted to probe */
		current = eth_get_dev();
	} while (old_current != current);

	return ret;
}

void eth_halt(void)
{
	struct udevice *current;
	struct eth_device_priv *priv;

	current = eth_get_dev();
	if (!current || !device_active(current))
		return;

	eth_get_ops(current)->stop(current);
	priv = current->uclass_priv;
	priv->state = ETH_STATE_PASSIVE;
}

int eth_is_active(struct udevice *dev)
{
	struct eth_device_priv *priv;

	if (!dev || !device_active(dev))
		return 0;

	priv = dev_get_uclass_priv(dev);
	return priv->state == ETH_STATE_ACTIVE;
}

int eth_send(void *packet, int length)
{
	struct udevice *current;
	int ret;

	current = eth_get_dev();
	if (!current)
		return -ENODEV;

	if (!device_active(current))
		return -EINVAL;

	ret = eth_get_ops(current)->send(current, packet, length);
	if (ret < 0) {
		/* We cannot completely return the error at present */
		debug("%s: send() returned error %d\n", __func__, ret);
	}
	return ret;
}

int eth_rx(void)
{
	struct udevice *current;
	uchar *packet;
	int flags;
	int ret;
	int i;

	current = eth_get_dev();
	if (!current)
		return -ENODEV;

	if (!device_active(current))
		return -EINVAL;

	/* Process up to 32 packets at one time */
	flags = ETH_RECV_CHECK_DEVICE;
	for (i = 0; i < 32; i++) {
		ret = eth_get_ops(current)->recv(current, flags, &packet);
		flags = 0;
		if (ret > 0)
			net_process_received_packet(packet, ret);
		if (ret >= 0 && eth_get_ops(current)->free_pkt)
			eth_get_ops(current)->free_pkt(current, packet, ret);
		if (ret <= 0)
			break;
	}
	if (ret == -EAGAIN)
		ret = 0;
	if (ret < 0) {
		/* We cannot completely return the error at present */
		debug("%s: recv() returned error %d\n", __func__, ret);
	}
	return ret;
}

int eth_initialize(void)
{
	int num_devices = 0;
	struct udevice *dev;

	eth_common_init();

	/*
	 * Devices need to write the hwaddr even if not started so that Linux
	 * will have access to the hwaddr that u-boot stored for the device.
	 * This is accomplished by attempting to probe each device and calling
	 * their write_hwaddr() operation.
	 */
	uclass_first_device(UCLASS_ETH, &dev);
	if (!dev) {
		printf("No ethernet found.\n");
		bootstage_error(BOOTSTAGE_ID_NET_ETH_START);
	} else {
		char *ethprime = getenv("ethprime");
		struct udevice *prime_dev = NULL;

		if (ethprime)
			prime_dev = eth_get_dev_by_name(ethprime);
		if (prime_dev) {
			eth_set_dev(prime_dev);
			eth_current_changed();
		} else {
			eth_set_dev(NULL);
		}

		bootstage_mark(BOOTSTAGE_ID_NET_ETH_INIT);
		do {
			if (num_devices)
				printf(", ");

			printf("eth%d: %s", dev->seq, dev->name);

			if (ethprime && dev == prime_dev)
				printf(" [PRIME]");

			eth_write_hwaddr(dev);

			uclass_next_device(&dev);
			num_devices++;
		} while (dev);

		putc('\n');
	}

	return num_devices;
}

static int eth_post_bind(struct udevice *dev)
{
	if (strchr(dev->name, ' ')) {
		printf("\nError: eth device name \"%s\" has a space!\n",
		       dev->name);
		return -EINVAL;
	}

	return 0;
}

static int eth_pre_unbind(struct udevice *dev)
{
	/* Don't hang onto a pointer that is going away */
	if (dev == eth_get_uclass_priv()->current)
		eth_set_dev(NULL);

	return 0;
}

static int eth_post_probe(struct udevice *dev)
{
	struct eth_device_priv *priv = dev->uclass_priv;
	struct eth_pdata *pdata = dev->platdata;
	unsigned char env_enetaddr[6];

	priv->state = ETH_STATE_INIT;

	/* Check if the device has a MAC address in ROM */
	if (eth_get_ops(dev)->read_rom_hwaddr)
		eth_get_ops(dev)->read_rom_hwaddr(dev);

	eth_getenv_enetaddr_by_index("eth", dev->seq, env_enetaddr);
	if (!is_zero_ethaddr(env_enetaddr)) {
		if (!is_zero_ethaddr(pdata->enetaddr) &&
		    memcmp(pdata->enetaddr, env_enetaddr, 6)) {
			printf("\nWarning: %s MAC addresses don't match:\n",
			       dev->name);
			printf("Address in SROM is         %pM\n",
			       pdata->enetaddr);
			printf("Address in environment is  %pM\n",
			       env_enetaddr);
		}

		/* Override the ROM MAC address */
		memcpy(pdata->enetaddr, env_enetaddr, 6);
	} else if (is_valid_ethaddr(pdata->enetaddr)) {
		eth_setenv_enetaddr_by_index("eth", dev->seq, pdata->enetaddr);
		printf("\nWarning: %s using MAC address from ROM\n",
		       dev->name);
	} else if (is_zero_ethaddr(pdata->enetaddr)) {
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(pdata->enetaddr);
		printf("\nWarning: %s (eth%d) using random MAC address - %pM\n",
		       dev->name, dev->seq, pdata->enetaddr);
#else
		printf("\nError: %s address not set.\n",
		       dev->name);
		return -EINVAL;
#endif
	}

	return 0;
}

static int eth_pre_remove(struct udevice *dev)
{
	eth_get_ops(dev)->stop(dev);

	return 0;
}

UCLASS_DRIVER(eth) = {
	.name		= "eth",
	.id		= UCLASS_ETH,
	.post_bind	= eth_post_bind,
	.pre_unbind	= eth_pre_unbind,
	.post_probe	= eth_post_probe,
	.pre_remove	= eth_pre_remove,
	.priv_auto_alloc_size = sizeof(struct eth_uclass_priv),
	.per_device_auto_alloc_size = sizeof(struct eth_device_priv),
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};
#endif /* #ifdef CONFIG_DM_ETH */

#ifndef CONFIG_DM_ETH

#ifdef CONFIG_API
static struct {
	uchar data[PKTSIZE];
	int length;
} eth_rcv_bufs[PKTBUFSRX];

static unsigned int eth_rcv_current, eth_rcv_last;
#endif

static struct eth_device *eth_devices;
struct eth_device *eth_current;

static void eth_set_current_to_next(void)
{
	eth_current = eth_current->next;
}

static void eth_set_dev(struct eth_device *dev)
{
	eth_current = dev;
}

struct eth_device *eth_get_dev_by_name(const char *devname)
{
	struct eth_device *dev, *target_dev;

	BUG_ON(devname == NULL);

	if (!eth_devices)
		return NULL;

	dev = eth_devices;
	target_dev = NULL;
	do {
		if (strcmp(devname, dev->name) == 0) {
			target_dev = dev;
			break;
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return target_dev;
}

struct eth_device *eth_get_dev_by_index(int index)
{
	struct eth_device *dev, *target_dev;

	if (!eth_devices)
		return NULL;

	dev = eth_devices;
	target_dev = NULL;
	do {
		if (dev->index == index) {
			target_dev = dev;
			break;
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return target_dev;
}

int eth_get_dev_index(void)
{
	if (!eth_current)
		return -1;

	return eth_current->index;
}

static int on_ethaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	int index;
	struct eth_device *dev;

	if (!eth_devices)
		return 0;

	/* look for an index after "eth" */
	index = simple_strtoul(name + 3, NULL, 10);

	dev = eth_devices;
	do {
		if (dev->index == index) {
			switch (op) {
			case env_op_create:
			case env_op_overwrite:
				eth_parse_enetaddr(value, dev->enetaddr);
				break;
			case env_op_delete:
				memset(dev->enetaddr, 0, 6);
			}
		}
	} while (dev != eth_devices);

	return 0;
}
U_BOOT_ENV_CALLBACK(ethaddr, on_ethaddr);

int eth_write_hwaddr(struct eth_device *dev, const char *base_name,
		   int eth_number)
{
	unsigned char env_enetaddr[6];
	int ret = 0;

	eth_getenv_enetaddr_by_index(base_name, eth_number, env_enetaddr);

	if (!is_zero_ethaddr(env_enetaddr)) {
		if (!is_zero_ethaddr(dev->enetaddr) &&
		    memcmp(dev->enetaddr, env_enetaddr, 6)) {
			printf("\nWarning: %s MAC addresses don't match:\n",
			       dev->name);
			printf("Address in SROM is         %pM\n",
			       dev->enetaddr);
			printf("Address in environment is  %pM\n",
			       env_enetaddr);
		}

		memcpy(dev->enetaddr, env_enetaddr, 6);
	} else if (is_valid_ethaddr(dev->enetaddr)) {
		eth_setenv_enetaddr_by_index(base_name, eth_number,
					     dev->enetaddr);
		printf("\nWarning: %s using MAC address from net device\n",
		       dev->name);
	} else if (is_zero_ethaddr(dev->enetaddr)) {
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(dev->enetaddr);
		printf("\nWarning: %s (eth%d) using random MAC address - %pM\n",
		       dev->name, eth_number, dev->enetaddr);
#else
		printf("\nError: %s address not set.\n",
		       dev->name);
		return -EINVAL;
#endif
	}

	if (dev->write_hwaddr && !eth_mac_skip(eth_number)) {
		if (!is_valid_ethaddr(dev->enetaddr)) {
			printf("\nError: %s address %pM illegal value\n",
			       dev->name, dev->enetaddr);
			return -EINVAL;
		}

		ret = dev->write_hwaddr(dev);
		if (ret)
			printf("\nWarning: %s failed to set MAC address\n",
			       dev->name);
	}

	return ret;
}

int eth_register(struct eth_device *dev)
{
	struct eth_device *d;
	static int index;

	assert(strlen(dev->name) < sizeof(dev->name));

	if (!eth_devices) {
		eth_devices = dev;
		eth_current = dev;
		eth_current_changed();
	} else {
		for (d = eth_devices; d->next != eth_devices; d = d->next)
			;
		d->next = dev;
	}

	dev->state = ETH_STATE_INIT;
	dev->next  = eth_devices;
	dev->index = index++;

	return 0;
}

int eth_unregister(struct eth_device *dev)
{
	struct eth_device *cur;

	/* No device */
	if (!eth_devices)
		return -ENODEV;

	for (cur = eth_devices; cur->next != eth_devices && cur->next != dev;
	     cur = cur->next)
		;

	/* Device not found */
	if (cur->next != dev)
		return -ENODEV;

	cur->next = dev->next;

	if (eth_devices == dev)
		eth_devices = dev->next == eth_devices ? NULL : dev->next;

	if (eth_current == dev) {
		eth_current = eth_devices;
		eth_current_changed();
	}

	return 0;
}

int eth_initialize(void)
{
	int num_devices = 0;

	eth_devices = NULL;
	eth_current = NULL;
	eth_common_init();

	if (!eth_devices) {
		puts("No ethernet found.\n");
		bootstage_error(BOOTSTAGE_ID_NET_ETH_START);
	} else {
		struct eth_device *dev = eth_devices;
		char *ethprime = getenv("ethprime");

		bootstage_mark(BOOTSTAGE_ID_NET_ETH_INIT);
		do {
			if (dev->index)
				puts(", ");

			printf("%s", dev->name);

			if (ethprime && strcmp(dev->name, ethprime) == 0) {
				eth_current = dev;
				puts(" [PRIME]");
			}

			if (strchr(dev->name, ' '))
				puts("\nWarning: eth device name has a space!"
					"\n");

			eth_write_hwaddr(dev, "eth", dev->index);

			dev = dev->next;
			num_devices++;
		} while (dev != eth_devices);

		eth_current_changed();
		putc('\n');
	}

	return num_devices;
}

#ifdef CONFIG_MCAST_TFTP
/* Multicast.
 * mcast_addr: multicast ipaddr from which multicast Mac is made
 * join: 1=join, 0=leave.
 */
int eth_mcast_join(struct in_addr mcast_ip, int join)
{
	u8 mcast_mac[6];
	if (!eth_current || !eth_current->mcast)
		return -1;
	mcast_mac[5] = htonl(mcast_ip.s_addr) & 0xff;
	mcast_mac[4] = (htonl(mcast_ip.s_addr)>>8) & 0xff;
	mcast_mac[3] = (htonl(mcast_ip.s_addr)>>16) & 0x7f;
	mcast_mac[2] = 0x5e;
	mcast_mac[1] = 0x0;
	mcast_mac[0] = 0x1;
	return eth_current->mcast(eth_current, mcast_mac, join);
}

/* the 'way' for ethernet-CRC-32. Spliced in from Linux lib/crc32.c
 * and this is the ethernet-crc method needed for TSEC -- and perhaps
 * some other adapter -- hash tables
 */
#define CRCPOLY_LE 0xedb88320
u32 ether_crc(size_t len, unsigned char const *p)
{
	int i;
	u32 crc;
	crc = ~0;
	while (len--) {
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	/* an reverse the bits, cuz of way they arrive -- last-first */
	crc = (crc >> 16) | (crc << 16);
	crc = (crc >> 8 & 0x00ff00ff) | (crc << 8 & 0xff00ff00);
	crc = (crc >> 4 & 0x0f0f0f0f) | (crc << 4 & 0xf0f0f0f0);
	crc = (crc >> 2 & 0x33333333) | (crc << 2 & 0xcccccccc);
	crc = (crc >> 1 & 0x55555555) | (crc << 1 & 0xaaaaaaaa);
	return crc;
}

#endif


int eth_init(void)
{
	struct eth_device *old_current;

	if (!eth_current) {
		puts("No ethernet found.\n");
		return -ENODEV;
	}

	old_current = eth_current;
	do {
		debug("Trying %s\n", eth_current->name);

		if (eth_current->init(eth_current, gd->bd) >= 0) {
			eth_current->state = ETH_STATE_ACTIVE;

			return 0;
		}
		debug("FAIL\n");

		eth_try_another(0);
	} while (old_current != eth_current);

	return -ETIMEDOUT;
}

void eth_halt(void)
{
	if (!eth_current)
		return;

	eth_current->halt(eth_current);

	eth_current->state = ETH_STATE_PASSIVE;
}

int eth_is_active(struct eth_device *dev)
{
	return dev && dev->state == ETH_STATE_ACTIVE;
}

int eth_send(void *packet, int length)
{
	if (!eth_current)
		return -ENODEV;

	return eth_current->send(eth_current, packet, length);
}

int eth_rx(void)
{
	if (!eth_current)
		return -ENODEV;

	return eth_current->recv(eth_current);
}
#endif /* ifndef CONFIG_DM_ETH */

#ifdef CONFIG_API
static void eth_save_packet(void *packet, int length)
{
	char *p = packet;
	int i;

	if ((eth_rcv_last+1) % PKTBUFSRX == eth_rcv_current)
		return;

	if (PKTSIZE < length)
		return;

	for (i = 0; i < length; i++)
		eth_rcv_bufs[eth_rcv_last].data[i] = p[i];

	eth_rcv_bufs[eth_rcv_last].length = length;
	eth_rcv_last = (eth_rcv_last + 1) % PKTBUFSRX;
}

int eth_receive(void *packet, int length)
{
	char *p = packet;
	void *pp = push_packet;
	int i;

	if (eth_rcv_current == eth_rcv_last) {
		push_packet = eth_save_packet;
		eth_rx();
		push_packet = pp;

		if (eth_rcv_current == eth_rcv_last)
			return -1;
	}

	length = min(eth_rcv_bufs[eth_rcv_current].length, length);

	for (i = 0; i < length; i++)
		p[i] = eth_rcv_bufs[eth_rcv_current].data[i];

	eth_rcv_current = (eth_rcv_current + 1) % PKTBUFSRX;
	return length;
}
#endif /* CONFIG_API */

static void eth_current_changed(void)
{
	char *act = getenv("ethact");
	/* update current ethernet name */
	if (eth_get_dev()) {
		if (act == NULL || strcmp(act, eth_get_name()) != 0)
			setenv("ethact", eth_get_name());
	}
	/*
	 * remove the variable completely if there is no active
	 * interface
	 */
	else if (act != NULL)
		setenv("ethact", NULL);
}

void eth_try_another(int first_restart)
{
	static void *first_failed;
	char *ethrotate;

	/*
	 * Do not rotate between network interfaces when
	 * 'ethrotate' variable is set to 'no'.
	 */
	ethrotate = getenv("ethrotate");
	if ((ethrotate != NULL) && (strcmp(ethrotate, "no") == 0))
		return;

	if (!eth_get_dev())
		return;

	if (first_restart)
		first_failed = eth_get_dev();

	eth_set_current_to_next();

	eth_current_changed();

	if (first_failed == eth_get_dev())
		net_restart_wrap = 1;
}

void eth_set_current(void)
{
	static char *act;
	static int  env_changed_id;
	int	env_id;

	env_id = get_env_id();
	if ((act == NULL) || (env_changed_id != env_id)) {
		act = getenv("ethact");
		env_changed_id = env_id;
	}

	if (act == NULL) {
		char *ethprime = getenv("ethprime");
		void *dev = NULL;

		if (ethprime)
			dev = eth_get_dev_by_name(ethprime);
		if (dev)
			eth_set_dev(dev);
		else
			eth_set_dev(NULL);
	} else {
		eth_set_dev(eth_get_dev_by_name(act));
	}

	eth_current_changed();
}

const char *eth_get_name(void)
{
	return eth_get_dev() ? eth_get_dev()->name : "unknown";
}
