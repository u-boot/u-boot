// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple network protocol
 * PXE base code protocol
 *
 * Copyright (c) 2016 Alexander Graf
 *
 * The simple network protocol has the following statuses and services
 * to move between them:
 *
 * Start():	 EfiSimpleNetworkStopped     -> EfiSimpleNetworkStarted
 * Initialize(): EfiSimpleNetworkStarted     -> EfiSimpleNetworkInitialized
 * Shutdown():	 EfiSimpleNetworkInitialized -> EfiSimpleNetworkStarted
 * Stop():	 EfiSimpleNetworkStarted     -> EfiSimpleNetworkStopped
 * Reset():	 EfiSimpleNetworkInitialized -> EfiSimpleNetworkInitialized
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_device_path.h>
#include <efi_loader.h>
#include <env.h>
#include <dm.h>
#include <linux/sizes.h>
#include <malloc.h>
#include <vsprintf.h>
#include <net.h>

#define MAX_EFI_NET_OBJS 10
#define MAX_NUM_DHCP_ENTRIES 10
#define MAX_NUM_DP_ENTRIES 10

const efi_guid_t efi_net_guid = EFI_SIMPLE_NETWORK_PROTOCOL_GUID;
static const efi_guid_t efi_pxe_base_code_protocol_guid =
					EFI_PXE_BASE_CODE_PROTOCOL_GUID;

struct dp_entry {
	struct efi_device_path *net_dp;
	struct udevice *dev;
	bool is_valid;
};

/*
 * The network device path cache. An entry is added when a new bootfile
 * is downloaded from the network. If the bootfile is then loaded as an
 * efi image, the most recent entry corresponding to the device is passed
 * as the device path of the loaded image.
 */
static struct dp_entry dp_cache[MAX_NUM_DP_ENTRIES];
static int next_dp_entry;

#if IS_ENABLED(CONFIG_EFI_HTTP_PROTOCOL)
static struct wget_http_info efi_wget_info = {
	.set_bootdev = false,
	.check_buffer_size = true,
	.silent = true,
};
#endif

struct dhcp_entry {
	struct efi_pxe_packet *dhcp_ack;
	struct udevice *dev;
	bool is_valid;
};

static struct dhcp_entry dhcp_cache[MAX_NUM_DHCP_ENTRIES];
static int next_dhcp_entry;

/**
 * struct efi_net_obj - EFI object representing a network interface
 *
 * @header:			EFI object header
 * @dev:			net udevice
 * @net:			simple network protocol interface
 * @net_mode:			status of the network interface
 * @pxe:			PXE base code protocol interface
 * @pxe_mode:			status of the PXE base code protocol
 * @ip4_config2:		IP4 Config2 protocol interface
 * @http_service_binding:	Http service binding protocol interface
 * @new_tx_packet:		new transmit packet
 * @transmit_buffer:	transmit buffer
 * @receive_buffer:		array of receive buffers
 * @receive_lengths:	array of lengths for received packets
 * @rx_packet_idx:		index of the current receive packet
 * @rx_packet_num:		number of received packets
 * @wait_for_packet:	signaled when a packet has been received
 * @network_timer_event:	event to check for new network packets.
 * @efi_seq_num:		sequence number of the EFI net object.
 */
struct efi_net_obj {
	struct efi_object header;
	struct udevice *dev;
	struct efi_simple_network net;
	struct efi_simple_network_mode net_mode;
	struct efi_pxe_base_code_protocol pxe;
	struct efi_pxe_mode pxe_mode;
#if IS_ENABLED(CONFIG_EFI_IP4_CONFIG2_PROTOCOL)
	struct efi_ip4_config2_protocol ip4_config2;
#endif
#if IS_ENABLED(CONFIG_EFI_HTTP_PROTOCOL)
	struct efi_service_binding_protocol http_service_binding;
#endif
	void *new_tx_packet;
	void *transmit_buffer;
	uchar **receive_buffer;
	size_t *receive_lengths;
	int rx_packet_idx;
	int rx_packet_num;
	struct efi_event *wait_for_packet;
	struct efi_event *network_timer_event;
	int efi_seq_num;
};

static int curr_efi_net_obj;
static struct efi_net_obj *net_objs[MAX_EFI_NET_OBJS];

/**
 * efi_netobj_is_active() - checks if a netobj is active in the efi subsystem
 *
 * @netobj:	pointer to efi_net_obj
 * Return:	true if active
 */
static bool efi_netobj_is_active(struct efi_net_obj *netobj)
{
	if (!netobj || !efi_search_obj(&netobj->header))
		return false;

	return true;
}

/*
 * efi_netobj_from_snp() - get efi_net_obj from simple network protocol
 *
 *
 * @snp:	pointer to the simple network protocol
 * Return:	pointer to efi_net_obj, NULL on error
 */
static struct efi_net_obj *efi_netobj_from_snp(struct efi_simple_network *snp)
{
	int i;

	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && &net_objs[i]->net == snp) {
			// Do not register duplicate devices
			return net_objs[i];
		}
	}
	return NULL;
}

/*
 * efi_net_start() - start the network interface
 *
 * This function implements the Start service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_start(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p", this);
	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	nt = efi_netobj_from_snp(this);

	if (this->mode->state != EFI_NETWORK_STOPPED) {
		ret = EFI_ALREADY_STARTED;
	} else {
		this->int_status = 0;
		nt->wait_for_packet->is_signaled = false;
		this->mode->state = EFI_NETWORK_STARTED;
	}
out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_stop() - stop the network interface
 *
 * This function implements the Stop service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_stop(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p", this);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	nt = efi_netobj_from_snp(this);

	if (this->mode->state == EFI_NETWORK_STOPPED) {
		ret = EFI_NOT_STARTED;
	} else {
		/* Disable hardware and put it into the reset state */
		eth_set_dev(nt->dev);
		env_set("ethact", eth_get_name());
		eth_halt();
		/* Clear cache of packets */
		nt->rx_packet_num = 0;
		this->mode->state = EFI_NETWORK_STOPPED;
	}
out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_initialize() - initialize the network interface
 *
 * This function implements the Initialize service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @extra_rx:	extra receive buffer to be allocated
 * @extra_tx:	extra transmit buffer to be allocated
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_initialize(struct efi_simple_network *this,
					      ulong extra_rx, ulong extra_tx)
{
	int ret;
	efi_status_t r = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p, %lx, %lx", this, extra_rx, extra_tx);

	/* Check parameters */
	if (!this) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}
	nt = efi_netobj_from_snp(this);

	switch (this->mode->state) {
	case EFI_NETWORK_INITIALIZED:
	case EFI_NETWORK_STARTED:
		break;
	default:
		r = EFI_NOT_STARTED;
		goto out;
	}

	/* Setup packet buffers */
	net_init();
	/* Clear cache of packets */
	nt->rx_packet_num = 0;
	/* Set the net device corresponding to the efi net object */
	eth_set_dev(nt->dev);
	env_set("ethact", eth_get_name());
	/* Get hardware ready for send and receive operations */
	ret = eth_start_udev(nt->dev);
	if (ret < 0) {
		eth_halt();
		this->mode->state = EFI_NETWORK_STOPPED;
		r = EFI_DEVICE_ERROR;
		goto out;
	} else {
		this->int_status = 0;
		nt->wait_for_packet->is_signaled = false;
		this->mode->state = EFI_NETWORK_INITIALIZED;
	}
out:
	return EFI_EXIT(r);
}

/*
 * efi_net_reset() - reinitialize the network interface
 *
 * This function implements the Reset service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:			pointer to the protocol instance
 * @extended_verification:	execute exhaustive verification
 * Return:			status code
 */
static efi_status_t EFIAPI efi_net_reset(struct efi_simple_network *this,
					 int extended_verification)
{
	efi_status_t ret;

	EFI_ENTRY("%p, %x", this, extended_verification);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	switch (this->mode->state) {
	case EFI_NETWORK_INITIALIZED:
		break;
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	default:
		ret = EFI_DEVICE_ERROR;
		goto out;
	}

	this->mode->state = EFI_NETWORK_STARTED;
	ret = EFI_CALL(efi_net_initialize(this, 0, 0));
out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_shutdown() - shut down the network interface
 *
 * This function implements the Shutdown service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_shutdown(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p", this);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}
	nt = efi_netobj_from_snp(this);

	switch (this->mode->state) {
	case EFI_NETWORK_INITIALIZED:
		break;
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	default:
		ret = EFI_DEVICE_ERROR;
		goto out;
	}

	eth_set_dev(nt->dev);
	env_set("ethact", eth_get_name());
	eth_halt();

	this->int_status = 0;
	nt->wait_for_packet->is_signaled = false;
	this->mode->state = EFI_NETWORK_STARTED;

out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_receive_filters() - mange multicast receive filters
 *
 * This function implements the ReceiveFilters service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @enable:		bit mask of receive filters to enable
 * @disable:		bit mask of receive filters to disable
 * @reset_mcast_filter:	true resets contents of the filters
 * @mcast_filter_count:	number of hardware MAC addresses in the new filters list
 * @mcast_filter:	list of new filters
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_receive_filters
		(struct efi_simple_network *this, u32 enable, u32 disable,
		 int reset_mcast_filter, ulong mcast_filter_count,
		 struct efi_mac_address *mcast_filter)
{
	EFI_ENTRY("%p, %x, %x, %x, %lx, %p", this, enable, disable,
		  reset_mcast_filter, mcast_filter_count, mcast_filter);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_station_address() - set the hardware MAC address
 *
 * This function implements the StationAddress service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @reset:	if true reset the address to default
 * @new_mac:	new MAC address
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_station_address
		(struct efi_simple_network *this, int reset,
		 struct efi_mac_address *new_mac)
{
	EFI_ENTRY("%p, %x, %p", this, reset, new_mac);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_statistics() - reset or collect statistics of the network interface
 *
 * This function implements the Statistics service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @reset:	if true, the statistics are reset
 * @stat_size:	size of the statistics table
 * @stat_table:	table to receive the statistics
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_statistics(struct efi_simple_network *this,
					      int reset, ulong *stat_size,
					      void *stat_table)
{
	EFI_ENTRY("%p, %x, %p, %p", this, reset, stat_size, stat_table);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_mcastiptomac() - translate multicast IP address to MAC address
 *
 * This function implements the MCastIPtoMAC service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @ipv6:	true if the IP address is an IPv6 address
 * @ip:		IP address
 * @mac:	MAC address
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_mcastiptomac(struct efi_simple_network *this,
						int ipv6,
						struct efi_ip_address *ip,
						struct efi_mac_address *mac)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %x, %p, %p", this, ipv6, ip, mac);

	if (!this || !ip || !mac) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (ipv6) {
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	/* Multi-cast addresses are in the range 224.0.0.0 - 239.255.255.255 */
	if ((ip->ip_addr[0] & 0xf0) != 0xe0) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	};

	switch (this->mode->state) {
	case EFI_NETWORK_INITIALIZED:
	case EFI_NETWORK_STARTED:
		break;
	default:
		ret = EFI_NOT_STARTED;
		goto out;
	}

	memset(mac, 0, sizeof(struct efi_mac_address));

	/*
	 * Copy lower 23 bits of IPv4 multi-cast address
	 * RFC 1112, RFC 7042 2.1.1.
	 */
	mac->mac_addr[0] = 0x01;
	mac->mac_addr[1] = 0x00;
	mac->mac_addr[2] = 0x5E;
	mac->mac_addr[3] = ip->ip_addr[1] & 0x7F;
	mac->mac_addr[4] = ip->ip_addr[2];
	mac->mac_addr[5] = ip->ip_addr[3];
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_nvdata() - read or write NVRAM
 *
 * This function implements the GetStatus service of the Simple Network
 * Protocol. See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @read_write:		true for read, false for write
 * @offset:		offset in NVRAM
 * @buffer_size:	size of buffer
 * @buffer:		buffer
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_nvdata(struct efi_simple_network *this,
					  int read_write, ulong offset,
					  ulong buffer_size, char *buffer)
{
	EFI_ENTRY("%p, %x, %lx, %lx, %p", this, read_write, offset, buffer_size,
		  buffer);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_net_get_status() - get interrupt status
 *
 * This function implements the GetStatus service of the Simple Network
 * Protocol. See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @int_status:		interface status
 * @txbuf:		transmission buffer
 */
static efi_status_t EFIAPI efi_net_get_status(struct efi_simple_network *this,
					      u32 *int_status, void **txbuf)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p, %p, %p", this, int_status, txbuf);

	efi_timer_check();

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	nt = efi_netobj_from_snp(this);

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	if (int_status) {
		*int_status = this->int_status;
		this->int_status = 0;
	}
	if (txbuf)
		*txbuf = nt->new_tx_packet;

	nt->new_tx_packet = NULL;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_transmit() - transmit a packet
 *
 * This function implements the Transmit service of the Simple Network Protocol.
 * See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @header_size:	size of the media header
 * @buffer_size:	size of the buffer to receive the packet
 * @buffer:		buffer to receive the packet
 * @src_addr:		source hardware MAC address
 * @dest_addr:		destination hardware MAC address
 * @protocol:		type of header to build
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_transmit
		(struct efi_simple_network *this, size_t header_size,
		 size_t buffer_size, void *buffer,
		 struct efi_mac_address *src_addr,
		 struct efi_mac_address *dest_addr, u16 *protocol)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p, %lu, %lu, %p, %p, %p, %p", this,
		  (unsigned long)header_size, (unsigned long)buffer_size,
		  buffer, src_addr, dest_addr, protocol);

	efi_timer_check();

	/* Check parameters */
	if (!this || !buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	nt = efi_netobj_from_snp(this);

	/* We do not support jumbo packets */
	if (buffer_size > PKTSIZE_ALIGN) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* At least the IP header has to fit into the buffer */
	if (buffer_size < this->mode->media_header_size) {
		ret = EFI_BUFFER_TOO_SMALL;
		goto out;
	}

	/*
	 * TODO:
	 * Support VLANs. Use net_set_ether() for copying the header. Use a
	 * U_BOOT_ENV_CALLBACK to update the media header size.
	 */
	if (header_size) {
		struct ethernet_hdr *header = buffer;

		if (!dest_addr || !protocol ||
		    header_size != this->mode->media_header_size) {
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}
		if (!src_addr)
			src_addr = &this->mode->current_address;

		memcpy(header->et_dest, dest_addr, ARP_HLEN);
		memcpy(header->et_src, src_addr, ARP_HLEN);
		header->et_protlen = htons(*protocol);
	}

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	eth_set_dev(nt->dev);
	env_set("ethact", eth_get_name());

	/* Ethernet packets always fit, just bounce */
	memcpy(nt->transmit_buffer, buffer, buffer_size);
	net_send_packet(nt->transmit_buffer, buffer_size);

	nt->new_tx_packet = buffer;
	this->int_status |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_receive() - receive a packet from a network interface
 *
 * This function implements the Receive service of the Simple Network Protocol.
 * See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @header_size:	size of the media header
 * @buffer_size:	size of the buffer to receive the packet
 * @buffer:		buffer to receive the packet
 * @src_addr:		source MAC address
 * @dest_addr:		destination MAC address
 * @protocol:		protocol
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_receive
		(struct efi_simple_network *this, size_t *header_size,
		 size_t *buffer_size, void *buffer,
		 struct efi_mac_address *src_addr,
		 struct efi_mac_address *dest_addr, u16 *protocol)
{
	efi_status_t ret = EFI_SUCCESS;
	struct ethernet_hdr *eth_hdr;
	size_t hdr_size = sizeof(struct ethernet_hdr);
	u16 protlen;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p, %p, %p, %p, %p, %p, %p", this, header_size,
		  buffer_size, buffer, src_addr, dest_addr, protocol);

	/* Execute events */
	efi_timer_check();

	/* Check parameters */
	if (!this || !buffer || !buffer_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	nt = efi_netobj_from_snp(this);

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	if (!nt->rx_packet_num) {
		ret = EFI_NOT_READY;
		goto out;
	}
	/* Fill export parameters */
	eth_hdr = (struct ethernet_hdr *)nt->receive_buffer[nt->rx_packet_idx];
	protlen = ntohs(eth_hdr->et_protlen);
	if (protlen == 0x8100) {
		hdr_size += 4;
		protlen = ntohs(*(u16 *)&nt->receive_buffer[nt->rx_packet_idx][hdr_size - 2]);
	}
	if (header_size)
		*header_size = hdr_size;
	if (dest_addr)
		memcpy(dest_addr, eth_hdr->et_dest, ARP_HLEN);
	if (src_addr)
		memcpy(src_addr, eth_hdr->et_src, ARP_HLEN);
	if (protocol)
		*protocol = protlen;
	if (*buffer_size < nt->receive_lengths[nt->rx_packet_idx]) {
		/* Packet doesn't fit, try again with bigger buffer */
		*buffer_size = nt->receive_lengths[nt->rx_packet_idx];
		ret = EFI_BUFFER_TOO_SMALL;
		goto out;
	}
	/* Copy packet */
	memcpy(buffer, nt->receive_buffer[nt->rx_packet_idx],
	       nt->receive_lengths[nt->rx_packet_idx]);
	*buffer_size = nt->receive_lengths[nt->rx_packet_idx];
	nt->rx_packet_idx = (nt->rx_packet_idx + 1) % ETH_PACKETS_BATCH_RECV;
	nt->rx_packet_num--;
	if (nt->rx_packet_num)
		nt->wait_for_packet->is_signaled = true;
	else
		this->int_status &= ~EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_set_dhcp_ack() - take note of a selected DHCP IP address
 *
 * This function is called by dhcp_handler().
 *
 * @pkt:	packet received by dhcp_handler()
 * @len:	length of the packet received
 */
void efi_net_set_dhcp_ack(void *pkt, int len)
{
	struct efi_pxe_packet **dhcp_ack;
	struct udevice *dev;
	int i;

	dhcp_ack = &dhcp_cache[next_dhcp_entry].dhcp_ack;

	/* For now this function gets called only by the current device */
	dev = eth_get_dev();

	int maxsize = sizeof(**dhcp_ack);

	if (!*dhcp_ack) {
		*dhcp_ack = malloc(maxsize);
		if (!*dhcp_ack)
			return;
	}
	memset(*dhcp_ack, 0, maxsize);
	memcpy(*dhcp_ack, pkt, min(len, maxsize));

	dhcp_cache[next_dhcp_entry].is_valid = true;
	dhcp_cache[next_dhcp_entry].dev = dev;
	next_dhcp_entry++;
	next_dhcp_entry %= MAX_NUM_DHCP_ENTRIES;

	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && net_objs[i]->dev == dev) {
			net_objs[i]->pxe_mode.dhcp_ack = **dhcp_ack;
		}
	}
}

/**
 * efi_net_push() - callback for received network packet
 *
 * This function is called when a network packet is received by eth_rx().
 *
 * @pkt:	network packet
 * @len:	length
 */
static void efi_net_push(void *pkt, int len)
{
	int rx_packet_next;
	struct efi_net_obj *nt;

	nt = net_objs[curr_efi_net_obj];
	if (!nt)
		return;

	/* Check that we at least received an Ethernet header */
	if (len < sizeof(struct ethernet_hdr))
		return;

	/* Check that the buffer won't overflow */
	if (len > PKTSIZE_ALIGN)
		return;

	/* Can't store more than pre-alloced buffer */
	if (nt->rx_packet_num >= ETH_PACKETS_BATCH_RECV)
		return;

	rx_packet_next = (nt->rx_packet_idx + nt->rx_packet_num) %
	    ETH_PACKETS_BATCH_RECV;
	memcpy(nt->receive_buffer[rx_packet_next], pkt, len);
	nt->receive_lengths[rx_packet_next] = len;

	nt->rx_packet_num++;
}

/**
 * efi_network_timer_notify() - check if a new network packet has been received
 *
 * This notification function is called in every timer cycle.
 *
 * @event:	the event for which this notification function is registered
 * @context:	event context - not used in this function
 */
static void EFIAPI efi_network_timer_notify(struct efi_event *event,
					    void *context)
{
	struct efi_simple_network *this = (struct efi_simple_network *)context;
	struct efi_net_obj *nt;

	EFI_ENTRY("%p, %p", event, context);

	/*
	 * Some network drivers do not support calling eth_rx() before
	 * initialization.
	 */
	if (!this || this->mode->state != EFI_NETWORK_INITIALIZED)
		goto out;

	nt = efi_netobj_from_snp(this);
	curr_efi_net_obj = nt->efi_seq_num;

	if (!nt->rx_packet_num) {
		eth_set_dev(nt->dev);
		env_set("ethact", eth_get_name());
		push_packet = efi_net_push;
		eth_rx();
		push_packet = NULL;
		if (nt->rx_packet_num) {
			this->int_status |=
				EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
			nt->wait_for_packet->is_signaled = true;
		}
	}
out:
	EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_pxe_base_code_start(
				struct efi_pxe_base_code_protocol *this,
				u8 use_ipv6)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_stop(
				struct efi_pxe_base_code_protocol *this)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_dhcp(
				struct efi_pxe_base_code_protocol *this,
				u8 sort_offers)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_discover(
				struct efi_pxe_base_code_protocol *this,
				u16 type, u16 *layer, u8 bis,
				struct efi_pxe_base_code_discover_info *info)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_mtftp(
				struct efi_pxe_base_code_protocol *this,
				u32 operation, void *buffer_ptr,
				u8 overwrite, efi_uintn_t *buffer_size,
				struct efi_ip_address server_ip, char *filename,
				struct efi_pxe_base_code_mtftp_info *info,
				u8 dont_use_buffer)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_udp_write(
				struct efi_pxe_base_code_protocol *this,
				u16 op_flags, struct efi_ip_address *dest_ip,
				u16 *dest_port,
				struct efi_ip_address *gateway_ip,
				struct efi_ip_address *src_ip, u16 *src_port,
				efi_uintn_t *header_size, void *header_ptr,
				efi_uintn_t *buffer_size, void *buffer_ptr)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_udp_read(
				struct efi_pxe_base_code_protocol *this,
				u16 op_flags, struct efi_ip_address *dest_ip,
				u16 *dest_port, struct efi_ip_address *src_ip,
				u16 *src_port, efi_uintn_t *header_size,
				void *header_ptr, efi_uintn_t *buffer_size,
				void *buffer_ptr)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_set_ip_filter(
				struct efi_pxe_base_code_protocol *this,
				struct efi_pxe_base_code_filter *new_filter)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_arp(
				struct efi_pxe_base_code_protocol *this,
				struct efi_ip_address *ip_addr,
				struct efi_mac_address *mac_addr)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_set_parameters(
				struct efi_pxe_base_code_protocol *this,
				u8 *new_auto_arp, u8 *new_send_guid,
				u8 *new_ttl, u8 *new_tos,
				u8 *new_make_callback)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_set_station_ip(
				struct efi_pxe_base_code_protocol *this,
				struct efi_ip_address *new_station_ip,
				struct efi_ip_address *new_subnet_mask)
{
	return EFI_UNSUPPORTED;
}

static efi_status_t EFIAPI efi_pxe_base_code_set_packets(
				struct efi_pxe_base_code_protocol *this,
				u8 *new_dhcp_discover_valid,
				u8 *new_dhcp_ack_received,
				u8 *new_proxy_offer_received,
				u8 *new_pxe_discover_valid,
				u8 *new_pxe_reply_received,
				u8 *new_pxe_bis_reply_received,
				EFI_PXE_BASE_CODE_PACKET *new_dchp_discover,
				EFI_PXE_BASE_CODE_PACKET *new_dhcp_acc,
				EFI_PXE_BASE_CODE_PACKET *new_proxy_offer,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_discover,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_reply,
				EFI_PXE_BASE_CODE_PACKET *new_pxe_bis_reply)
{
	return EFI_UNSUPPORTED;
}

/**
 * efi_netobj_set_dp() - set device path of a netobj
 *
 * @netobj:	pointer to efi_net_obj
 * @dp:		device path to set, allocated by caller
 * Return:	status code
 */
efi_status_t efi_netobj_set_dp(struct efi_net_obj *netobj, struct efi_device_path *dp)
{
	efi_status_t ret;
	struct efi_handler *phandler;
	struct efi_device_path *new_net_dp;

	if (!efi_netobj_is_active(netobj))
		return EFI_SUCCESS;

	// Create a device path for the netobj
	new_net_dp = dp;
	if (!new_net_dp)
		return EFI_OUT_OF_RESOURCES;

	phandler = NULL;
	efi_search_protocol(&netobj->header, &efi_guid_device_path, &phandler);

	// If the device path protocol is not yet installed, install it
	if (!phandler)
		goto add;

	// If it is already installed, try to update it
	ret = efi_reinstall_protocol_interface(&netobj->header, &efi_guid_device_path,
					       phandler->protocol_interface, new_net_dp);
	if (ret != EFI_SUCCESS)
		return ret;

	return EFI_SUCCESS;
add:
	ret = efi_add_protocol(&netobj->header, &efi_guid_device_path,
			       new_net_dp);
	if (ret != EFI_SUCCESS)
		return ret;

	return EFI_SUCCESS;
}

/**
 * efi_netobj_get_dp() - get device path of a netobj
 *
 * @netobj:	pointer to efi_net_obj
 * Return:	device path, NULL on error
 */
static struct efi_device_path *efi_netobj_get_dp(struct efi_net_obj *netobj)
{
	struct efi_handler *phandler;

	if (!efi_netobj_is_active(netobj))
		return NULL;

	phandler = NULL;
	efi_search_protocol(&netobj->header, &efi_guid_device_path, &phandler);

	if (phandler && phandler->protocol_interface)
		return efi_dp_dup(phandler->protocol_interface);

	return NULL;
}

/**
 * efi_net_do_start() - start the efi network stack
 *
 * This gets called from do_bootefi_exec() each time a payload gets executed.
 *
 * @dev:	net udevice
 * Return:	status code
 */
efi_status_t efi_net_do_start(struct udevice *dev)
{
	efi_status_t r = EFI_SUCCESS;
	struct efi_net_obj *netobj;
	struct efi_device_path *net_dp;
	int i;

	netobj = NULL;
	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && net_objs[i]->dev == dev) {
			netobj = net_objs[i];
			break;
		}
	}

	if (!efi_netobj_is_active(netobj))
		return r;

	efi_net_dp_from_dev(&net_dp, netobj->dev, true);
	// If no dp cache entry applies and there already
	// is a device path installed, continue
	if (!net_dp) {
		if (efi_netobj_get_dp(netobj))
			goto set_addr;
		else
			net_dp = efi_dp_from_eth(netobj->dev);

	}

	if (!net_dp)
		return EFI_OUT_OF_RESOURCES;

	r = efi_netobj_set_dp(netobj, net_dp);
	if (r != EFI_SUCCESS)
		return r;
set_addr:
#ifdef CONFIG_EFI_HTTP_PROTOCOL
	/*
	 * No harm on doing the following. If the PXE handle is present, the client could
	 * find it and try to get its IP address from it. In here the PXE handle is present
	 * but the PXE protocol is not yet implmenented, so we add this in the meantime.
	 */
	efi_net_get_addr((struct efi_ipv4_address *)&netobj->pxe_mode.station_ip,
			 (struct efi_ipv4_address *)&netobj->pxe_mode.subnet_mask, NULL, dev);
#endif

	return r;
}

/**
 * efi_net_register() - register the simple network protocol
 *
 * This gets called from do_bootefi_exec().
 * @dev:	net udevice
 */
efi_status_t efi_net_register(struct udevice *dev)
{
	efi_status_t r;
	int seq_num;
	struct efi_net_obj *netobj;
	void *transmit_buffer = NULL;
	uchar **receive_buffer = NULL;
	size_t *receive_lengths = NULL;
	int i, j;

	if (!dev) {
		/* No network device active, don't expose any */
		return EFI_SUCCESS;
	}

	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && net_objs[i]->dev == dev) {
			// Do not register duplicate devices
			return EFI_SUCCESS;
		}
	}

	seq_num = -1;
	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (!net_objs[i]) {
			seq_num = i;
			break;
		}
	}
	if (seq_num < 0)
		return EFI_OUT_OF_RESOURCES;

	/* We only expose the "active" network device, so one is enough */
	netobj = calloc(1, sizeof(*netobj));
	if (!netobj)
		goto out_of_resources;

	netobj->dev = dev;

	/* Allocate an aligned transmit buffer */
	transmit_buffer = calloc(1, PKTSIZE_ALIGN + PKTALIGN);
	if (!transmit_buffer)
		goto out_of_resources;
	transmit_buffer = (void *)ALIGN((uintptr_t)transmit_buffer, PKTALIGN);
	netobj->transmit_buffer = transmit_buffer;

	/* Allocate a number of receive buffers */
	receive_buffer = calloc(ETH_PACKETS_BATCH_RECV,
				sizeof(*receive_buffer));
	if (!receive_buffer)
		goto out_of_resources;
	for (i = 0; i < ETH_PACKETS_BATCH_RECV; i++) {
		receive_buffer[i] = malloc(PKTSIZE_ALIGN);
		if (!receive_buffer[i])
			goto out_of_resources;
	}
	netobj->receive_buffer = receive_buffer;

	receive_lengths = calloc(ETH_PACKETS_BATCH_RECV,
				 sizeof(*receive_lengths));
	if (!receive_lengths)
		goto out_of_resources;
	netobj->receive_lengths = receive_lengths;

	/* Hook net up to the device list */
	efi_add_handle(&netobj->header);

	/* Fill in object data */
	r = efi_add_protocol(&netobj->header, &efi_net_guid,
			     &netobj->net);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;

	r = efi_add_protocol(&netobj->header, &efi_pxe_base_code_protocol_guid,
			     &netobj->pxe);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
	netobj->net.revision = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
	netobj->net.start = efi_net_start;
	netobj->net.stop = efi_net_stop;
	netobj->net.initialize = efi_net_initialize;
	netobj->net.reset = efi_net_reset;
	netobj->net.shutdown = efi_net_shutdown;
	netobj->net.receive_filters = efi_net_receive_filters;
	netobj->net.station_address = efi_net_station_address;
	netobj->net.statistics = efi_net_statistics;
	netobj->net.mcastiptomac = efi_net_mcastiptomac;
	netobj->net.nvdata = efi_net_nvdata;
	netobj->net.get_status = efi_net_get_status;
	netobj->net.transmit = efi_net_transmit;
	netobj->net.receive = efi_net_receive;
	netobj->net.mode = &netobj->net_mode;
	netobj->net_mode.state = EFI_NETWORK_STOPPED;
	if (dev_get_plat(dev))
		memcpy(netobj->net_mode.current_address.mac_addr,
		       ((struct eth_pdata *)dev_get_plat(dev))->enetaddr, 6);
	netobj->net_mode.hwaddr_size = ARP_HLEN;
	netobj->net_mode.media_header_size = ETHER_HDR_SIZE;
	netobj->net_mode.max_packet_size = PKTSIZE;
	netobj->net_mode.if_type = ARP_ETHER;

	netobj->pxe.revision = EFI_PXE_BASE_CODE_PROTOCOL_REVISION;
	netobj->pxe.start = efi_pxe_base_code_start;
	netobj->pxe.stop = efi_pxe_base_code_stop;
	netobj->pxe.dhcp = efi_pxe_base_code_dhcp;
	netobj->pxe.discover = efi_pxe_base_code_discover;
	netobj->pxe.mtftp = efi_pxe_base_code_mtftp;
	netobj->pxe.udp_write = efi_pxe_base_code_udp_write;
	netobj->pxe.udp_read = efi_pxe_base_code_udp_read;
	netobj->pxe.set_ip_filter = efi_pxe_base_code_set_ip_filter;
	netobj->pxe.arp = efi_pxe_base_code_arp;
	netobj->pxe.set_parameters = efi_pxe_base_code_set_parameters;
	netobj->pxe.set_station_ip = efi_pxe_base_code_set_station_ip;
	netobj->pxe.set_packets = efi_pxe_base_code_set_packets;
	netobj->pxe.mode = &netobj->pxe_mode;

	/*
	 * Scan dhcp entries for one corresponding
	 * to this udevice, from newest to oldest
	 */
	i = (next_dhcp_entry + MAX_NUM_DHCP_ENTRIES - 1) % MAX_NUM_DHCP_ENTRIES;
	for (j = 0; dhcp_cache[i].is_valid && j < MAX_NUM_DHCP_ENTRIES;
	     i = (i + MAX_NUM_DHCP_ENTRIES - 1) % MAX_NUM_DHCP_ENTRIES, j++) {
		if (dev == dhcp_cache[i].dev) {
			netobj->pxe_mode.dhcp_ack = *dhcp_cache[i].dhcp_ack;
			break;
		}
	}

	/*
	 * Create WaitForPacket event.
	 */
	r = efi_create_event(EVT_NOTIFY_WAIT, TPL_CALLBACK,
			     efi_network_timer_notify, NULL, NULL,
			     &netobj->wait_for_packet);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	netobj->net.wait_for_packet = netobj->wait_for_packet;
	/*
	 * Create a timer event.
	 *
	 * The notification function is used to check if a new network packet
	 * has been received.
	 *
	 * iPXE is running at TPL_CALLBACK most of the time. Use a higher TPL.
	 */
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY,
			     efi_network_timer_notify, &netobj->net, NULL,
			     &netobj->network_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	/* Network is time critical, create event in every timer cycle */
	r = efi_set_timer(netobj->network_timer_event, EFI_TIMER_PERIODIC, 0);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to set network timer\n");
		return r;
	}

#if IS_ENABLED(CONFIG_EFI_IP4_CONFIG2_PROTOCOL)
	r = efi_ipconfig_register(&netobj->header, &netobj->ip4_config2);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
#endif

#ifdef CONFIG_EFI_HTTP_PROTOCOL
	r = efi_http_register(&netobj->header, &netobj->http_service_binding);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
#endif
	netobj->efi_seq_num = seq_num;
	net_objs[seq_num] = netobj;
	return EFI_SUCCESS;
failure_to_add_protocol:
	printf("ERROR: Failure to add protocol\n");
	return r;
out_of_resources:
	free(netobj);
	netobj = NULL;
	free(transmit_buffer);
	if (receive_buffer)
		for (i = 0; i < ETH_PACKETS_BATCH_RECV; i++)
			free(receive_buffer[i]);
	free(receive_buffer);
	free(receive_lengths);
	printf("ERROR: Out of memory\n");
	return EFI_OUT_OF_RESOURCES;
}

/**
 * efi_net_new_dp() - update device path associated to a net udevice
 *
 * This gets called to update the device path when a new boot
 * file is downloaded
 *
 * @dev:	dev to set the device path from
 * @server:	remote server address
 * @udev:	net udevice
 * Return:	status code
 */
efi_status_t efi_net_new_dp(const char *dev, const char *server, struct udevice *udev)
{
	efi_status_t ret;
	struct efi_net_obj *netobj;
	struct efi_device_path *old_net_dp, *new_net_dp;
	struct efi_device_path **dp;
	int i;

	dp = &dp_cache[next_dp_entry].net_dp;

	dp_cache[next_dp_entry].dev = udev;
	dp_cache[next_dp_entry].is_valid = true;
	next_dp_entry++;
	next_dp_entry %= MAX_NUM_DP_ENTRIES;

	old_net_dp = *dp;
	new_net_dp = NULL;
	if (!strcmp(dev, "Net"))
		new_net_dp = efi_dp_from_eth(udev);
	else if (!strcmp(dev, "Http"))
		new_net_dp = efi_dp_from_http(server, udev);
	if (!new_net_dp)
		return EFI_OUT_OF_RESOURCES;

	*dp = new_net_dp;
	// Free the old cache entry
	efi_free_pool(old_net_dp);

	netobj = NULL;
	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && net_objs[i]->dev == udev) {
			netobj = net_objs[i];
			break;
		}
	}
	if (!netobj)
		return EFI_SUCCESS;

	new_net_dp = efi_dp_dup(*dp);
	if (!new_net_dp)
		return EFI_OUT_OF_RESOURCES;
	ret = efi_netobj_set_dp(netobj, new_net_dp);
	if (ret != EFI_SUCCESS)
		efi_free_pool(new_net_dp);

	return ret;
}

/**
 * efi_net_dp_from_dev() - get device path associated to a net udevice
 *
 * Produce a copy of the current device path
 *
 * @dp:		copy of the current device path
 * @udev:	net udevice
 * @cache_only:	get device path from cache only
 */
void efi_net_dp_from_dev(struct efi_device_path **dp, struct udevice *udev, bool cache_only)
{
	int i, j;

	if (!dp)
		return;

	*dp = NULL;

	if (cache_only)
		goto cache;

	// If a netobj matches:
	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && net_objs[i]->dev == udev) {
			*dp = efi_netobj_get_dp(net_objs[i]);
			if (*dp)
				return;
		}
	}
cache:
	// Search in the cache
	i = (next_dp_entry + MAX_NUM_DP_ENTRIES - 1) % MAX_NUM_DP_ENTRIES;
	for (j = 0; dp_cache[i].is_valid && j < MAX_NUM_DP_ENTRIES;
		i = (i + MAX_NUM_DP_ENTRIES - 1) % MAX_NUM_DP_ENTRIES, j++) {
		if (dp_cache[i].dev == udev) {
			*dp = efi_dp_dup(dp_cache[i].net_dp);
			return;
		}
	}
}

/**
 * efi_net_get_addr() - get IP address information
 *
 * Copy the current IP address, mask, and gateway into the
 * efi_ipv4_address structs pointed to by ip, mask and gw,
 * respectively.
 *
 * @ip:		pointer to an efi_ipv4_address struct to
 *		be filled with the current IP address
 * @mask:	pointer to an efi_ipv4_address struct to
 *		be filled with the current network mask
 * @gw:		pointer to an efi_ipv4_address struct to be
 *		filled with the current network gateway
 * @dev:	udevice
 */
void efi_net_get_addr(struct efi_ipv4_address *ip,
		      struct efi_ipv4_address *mask,
		      struct efi_ipv4_address *gw,
		      struct udevice *dev)
{
	if (!dev)
		dev = eth_get_dev();
#ifdef CONFIG_NET_LWIP
	char ipstr[] = "ipaddr\0\0";
	char maskstr[] = "netmask\0\0";
	char gwstr[] = "gatewayip\0\0";
	int idx;
	struct in_addr tmp;
	char *env;

	idx = dev_seq(dev);

	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return;
	}

	if (idx) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	}

	env = env_get(ipstr);
	if (env && ip) {
		tmp = string_to_ip(env);
		memcpy(ip, &tmp, sizeof(tmp));
	}

	env = env_get(maskstr);
	if (env && mask) {
		tmp = string_to_ip(env);
		memcpy(mask, &tmp, sizeof(tmp));
	}
	env = env_get(gwstr);
	if (env && gw) {
		tmp = string_to_ip(env);
		memcpy(gw, &tmp, sizeof(tmp));
	}
#else
	if (ip)
		memcpy(ip, &net_ip, sizeof(net_ip));
	if (mask)
		memcpy(mask, &net_netmask, sizeof(net_netmask));
#endif
}

/**
 * efi_net_set_addr() - set IP address information
 *
 * Set the current IP address, mask, and gateway to the
 * efi_ipv4_address structs pointed to by ip, mask and gw,
 * respectively.
 *
 * @ip:		pointer to new IP address
 * @mask:	pointer to new network mask to set
 * @gw:		pointer to new network gateway
 * @dev:	udevice
 */
void efi_net_set_addr(struct efi_ipv4_address *ip,
		      struct efi_ipv4_address *mask,
		      struct efi_ipv4_address *gw,
		      struct udevice *dev)
{
	if (!dev)
		dev = eth_get_dev();
#ifdef CONFIG_NET_LWIP
	char ipstr[] = "ipaddr\0\0";
	char maskstr[] = "netmask\0\0";
	char gwstr[] = "gatewayip\0\0";
	int idx;
	struct in_addr *addr;
	char tmp[46];

	idx = dev_seq(dev);

	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return;
	}

	if (idx) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	}

	if (ip) {
		addr = (struct in_addr *)ip;
		ip_to_string(*addr, tmp);
		env_set(ipstr, tmp);
	}

	if (mask) {
		addr = (struct in_addr *)mask;
		ip_to_string(*addr, tmp);
		env_set(maskstr, tmp);
	}

	if (gw) {
		addr = (struct in_addr *)gw;
		ip_to_string(*addr, tmp);
		env_set(gwstr, tmp);
	}
#else
	if (ip)
		memcpy(&net_ip, ip, sizeof(*ip));
	if (mask)
		memcpy(&net_netmask, mask, sizeof(*mask));
#endif
}

#if IS_ENABLED(CONFIG_EFI_HTTP_PROTOCOL)
/**
 * efi_net_set_buffer() - allocate a buffer of min 64K
 *
 * @buffer:	allocated buffer
 * @size:	desired buffer size
 * Return:	status code
 */
static efi_status_t efi_net_set_buffer(void **buffer, size_t size)
{
	efi_status_t ret = EFI_SUCCESS;

	if (size < SZ_64K)
		size = SZ_64K;

	*buffer = efi_alloc(size);
	if (!*buffer)
		ret = EFI_OUT_OF_RESOURCES;

	efi_wget_info.buffer_size = (ulong)size;

	return ret;
}

/**
 * efi_net_parse_headers() - parse HTTP headers
 *
 * Parses the raw buffer efi_wget_info.headers into an array headers
 * of efi structs http_headers. The array should be at least
 * MAX_HTTP_HEADERS long.
 *
 * @num_headers:	number of headers
 * @headers:		caller provided array of struct http_headers
 */
void efi_net_parse_headers(ulong *num_headers, struct http_header *headers)
{
	if (!num_headers || !headers)
		return;

	// Populate info with http headers.
	*num_headers = 0;
	const uchar *line_start = efi_wget_info.headers;
	const uchar *line_end;
	ulong count;
	struct http_header *current_header;
	const uchar *separator;
	size_t name_length, value_length;

	// Skip the first line (request or status line)
	line_end = strstr(line_start, "\r\n");

	if (line_end)
		line_start = line_end + 2;

	while ((line_end = strstr(line_start, "\r\n")) != NULL) {
		count = *num_headers;
		if (line_start == line_end || count >= MAX_HTTP_HEADERS)
			break;
		current_header = headers + count;
		separator = strchr(line_start, ':');
		if (separator) {
			name_length = separator - line_start;
			++separator;
			while (*separator == ' ')
				++separator;
			value_length = line_end - separator;
			if (name_length < MAX_HTTP_HEADER_NAME &&
			    value_length < MAX_HTTP_HEADER_VALUE) {
				strncpy(current_header->name, line_start, name_length);
				current_header->name[name_length] = '\0';
				strncpy(current_header->value, separator, value_length);
				current_header->value[value_length] = '\0';
				(*num_headers)++;
			}
		}
		line_start = line_end + 2;
	}
}

/**
 * efi_net_do_request() - issue an HTTP request using wget
 *
 * @url:		url
 * @method:		HTTP method
 * @buffer:		data buffer
 * @status_code:	HTTP status code
 * @file_size:		file size in bytes
 * @headers_buffer:	headers buffer
 * @parent:		service binding protocol
 * Return:		status code
 */
efi_status_t efi_net_do_request(u8 *url, enum efi_http_method method, void **buffer,
				u32 *status_code, ulong *file_size, char *headers_buffer,
				struct efi_service_binding_protocol *parent)
{
	efi_status_t ret = EFI_SUCCESS;
	int wget_ret;
	static bool last_head;
	struct udevice *dev;
	int i;

	if (!buffer || !file_size || !parent)
		return EFI_ABORTED;

	efi_wget_info.method = (enum wget_http_method)method;
	efi_wget_info.headers = headers_buffer;

	// Set corresponding udevice
	dev = NULL;
	for (i = 0; i < MAX_EFI_NET_OBJS; i++) {
		if (net_objs[i] && &net_objs[i]->http_service_binding == parent)
			dev = net_objs[i]->dev;
	}
	if (!dev)
		return EFI_ABORTED;

	switch (method) {
	case HTTP_METHOD_GET:
		ret = efi_net_set_buffer(buffer, last_head ? (size_t)efi_wget_info.hdr_cont_len : 0);
		if (ret != EFI_SUCCESS)
			goto out;
		eth_set_dev(dev);
		env_set("ethact", eth_get_name());
		wget_ret = wget_request((ulong)*buffer, url, &efi_wget_info);
		if ((ulong)efi_wget_info.hdr_cont_len > efi_wget_info.buffer_size) {
			// Try again with updated buffer size
			efi_free_pool(*buffer);
			ret = efi_net_set_buffer(buffer, (size_t)efi_wget_info.hdr_cont_len);
			if (ret != EFI_SUCCESS)
				goto out;
			eth_set_dev(dev);
			env_set("ethact", eth_get_name());
			if (wget_request((ulong)*buffer, url, &efi_wget_info)) {
				efi_free_pool(*buffer);
				ret = EFI_DEVICE_ERROR;
				goto out;
			}
		} else if (wget_ret) {
			efi_free_pool(*buffer);
			ret = EFI_DEVICE_ERROR;
			goto out;
		}
		// Pass the actual number of received bytes to the application
		*file_size = efi_wget_info.file_size;
		*status_code = efi_wget_info.status_code;
		last_head = false;
		break;
	case HTTP_METHOD_HEAD:
		ret = efi_net_set_buffer(buffer, 0);
		if (ret != EFI_SUCCESS)
			goto out;
		eth_set_dev(dev);
		env_set("ethact", eth_get_name());
		wget_request((ulong)*buffer, url, &efi_wget_info);
		*file_size = 0;
		*status_code = efi_wget_info.status_code;
		last_head = true;
		break;
	default:
		ret = EFI_UNSUPPORTED;
		break;
	}

out:
	return ret;
}
#endif
