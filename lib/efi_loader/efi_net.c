/*
 *  EFI application network access support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <inttypes.h>
#include <lcd.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

static const efi_guid_t efi_net_guid = EFI_SIMPLE_NETWORK_GUID;
static const efi_guid_t efi_pxe_guid = EFI_PXE_GUID;
static struct efi_pxe_packet *dhcp_ack;
static bool new_rx_packet;
static void *new_tx_packet;
/*
 * The notification function of this event is called in every timer cycle
 * to check if a new network packet has been received.
 */
static struct efi_event *network_timer_event;
/*
 * This event is signaled when a packet has been received.
 */
static struct efi_event *wait_for_packet;

struct efi_net_obj {
	/* Generic EFI object parent class data */
	struct efi_object parent;
	/* EFI Interface callback struct for network */
	struct efi_simple_network net;
	struct efi_simple_network_mode net_mode;
	/* PXE struct to transmit dhcp data */
	struct efi_pxe pxe;
	struct efi_pxe_mode pxe_mode;
};

static efi_status_t EFIAPI efi_net_start(struct efi_simple_network *this)
{
	EFI_ENTRY("%p", this);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_stop(struct efi_simple_network *this)
{
	EFI_ENTRY("%p", this);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_initialize(struct efi_simple_network *this,
					      ulong extra_rx, ulong extra_tx)
{
	EFI_ENTRY("%p, %lx, %lx", this, extra_rx, extra_tx);

	eth_init();

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_reset(struct efi_simple_network *this,
					 int extended_verification)
{
	EFI_ENTRY("%p, %x", this, extended_verification);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_shutdown(struct efi_simple_network *this)
{
	EFI_ENTRY("%p", this);

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_receive_filters(
		struct efi_simple_network *this, u32 enable, u32 disable,
		int reset_mcast_filter, ulong mcast_filter_count,
		struct efi_mac_address *mcast_filter)
{
	EFI_ENTRY("%p, %x, %x, %x, %lx, %p", this, enable, disable,
		  reset_mcast_filter, mcast_filter_count, mcast_filter);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_net_station_address(
		struct efi_simple_network *this, int reset,
		struct efi_mac_address *new_mac)
{
	EFI_ENTRY("%p, %x, %p", this, reset, new_mac);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_net_statistics(struct efi_simple_network *this,
					      int reset, ulong *stat_size,
					      void *stat_table)
{
	EFI_ENTRY("%p, %x, %p, %p", this, reset, stat_size, stat_table);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_net_mcastiptomac(struct efi_simple_network *this,
						int ipv6,
						struct efi_ip_address *ip,
						struct efi_mac_address *mac)
{
	EFI_ENTRY("%p, %x, %p, %p", this, ipv6, ip, mac);

	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_net_nvdata(struct efi_simple_network *this,
					  int read_write, ulong offset,
					  ulong buffer_size, char *buffer)
{
	EFI_ENTRY("%p, %x, %lx, %lx, %p", this, read_write, offset, buffer_size,
		  buffer);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

static efi_status_t EFIAPI efi_net_get_status(struct efi_simple_network *this,
					      u32 *int_status, void **txbuf)
{
	EFI_ENTRY("%p, %p, %p", this, int_status, txbuf);

	efi_timer_check();

	if (int_status) {
		/* We send packets synchronously, so nothing is outstanding */
		*int_status = EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
		if (new_rx_packet)
			*int_status |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
	}
	if (txbuf)
		*txbuf = new_tx_packet;

	new_tx_packet = NULL;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_transmit(struct efi_simple_network *this,
		size_t header_size, size_t buffer_size, void *buffer,
		struct efi_mac_address *src_addr,
		struct efi_mac_address *dest_addr, u16 *protocol)
{
	EFI_ENTRY("%p, %lu, %lu, %p, %p, %p, %p", this,
		  (unsigned long)header_size, (unsigned long)buffer_size,
		  buffer, src_addr, dest_addr, protocol);

	efi_timer_check();

	if (header_size) {
		/* We would need to create the header if header_size != 0 */
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	}

#ifdef CONFIG_EFI_LOADER_BOUNCE_BUFFER
	/* Ethernet packets always fit, just bounce */
	memcpy(efi_bounce_buffer, buffer, buffer_size);
	net_send_packet(efi_bounce_buffer, buffer_size);
#else
	net_send_packet(buffer, buffer_size);
#endif

	new_tx_packet = buffer;

	return EFI_EXIT(EFI_SUCCESS);
}

static void efi_net_push(void *pkt, int len)
{
	new_rx_packet = true;
	wait_for_packet->is_signaled = true;
}

/*
 * Receive a packet from a network interface.
 *
 * This function implements the Receive service of the Simple Network Protocol.
 * See the UEFI spec for details.
 *
 * @this	the instance of the Simple Network Protocol
 * @header_size	size of the media header
 * @buffer_size	size of the buffer to receive the packet
 * @buffer	buffer to receive the packet
 * @src_addr	source MAC address
 * @dest_addr	destination MAC address
 * @protocol	protocol
 * @return	status code
 */
static efi_status_t EFIAPI efi_net_receive(struct efi_simple_network *this,
		size_t *header_size, size_t *buffer_size, void *buffer,
		struct efi_mac_address *src_addr,
		struct efi_mac_address *dest_addr, u16 *protocol)
{
	struct ethernet_hdr *eth_hdr;
	size_t hdr_size = sizeof(struct ethernet_hdr);
	u16 protlen;

	EFI_ENTRY("%p, %p, %p, %p, %p, %p, %p", this, header_size,
		  buffer_size, buffer, src_addr, dest_addr, protocol);

	efi_timer_check();

	if (!new_rx_packet)
		return EFI_EXIT(EFI_NOT_READY);
	/* Check that we at least received an Ethernet header */
	if (net_rx_packet_len < sizeof(struct ethernet_hdr)) {
		new_rx_packet = false;
		return EFI_EXIT(EFI_NOT_READY);
	}
	/* Fill export parameters */
	eth_hdr = (struct ethernet_hdr *)net_rx_packet;
	protlen = ntohs(eth_hdr->et_protlen);
	if (protlen == 0x8100) {
		hdr_size += 4;
		protlen = ntohs(*(u16 *)&net_rx_packet[hdr_size - 2]);
	}
	if (header_size)
		*header_size = hdr_size;
	if (dest_addr)
		memcpy(dest_addr, eth_hdr->et_dest, ARP_HLEN);
	if (src_addr)
		memcpy(src_addr, eth_hdr->et_src, ARP_HLEN);
	if (protocol)
		*protocol = protlen;
	if (*buffer_size < net_rx_packet_len) {
		/* Packet doesn't fit, try again with bigger buf */
		*buffer_size = net_rx_packet_len;
		return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
	}
	/* Copy packet */
	memcpy(buffer, net_rx_packet, net_rx_packet_len);
	*buffer_size = net_rx_packet_len;
	new_rx_packet = false;

	return EFI_EXIT(EFI_SUCCESS);
}

void efi_net_set_dhcp_ack(void *pkt, int len)
{
	int maxsize = sizeof(*dhcp_ack);

	if (!dhcp_ack)
		dhcp_ack = malloc(maxsize);

	memcpy(dhcp_ack, pkt, min(len, maxsize));
}

/*
 * Check if a new network packet has been received.
 *
 * This notification function is called in every timer cycle.
 *
 * @event	the event for which this notification function is registered
 * @context	event context - not used in this function
 */
static void EFIAPI efi_network_timer_notify(struct efi_event *event,
					    void *context)
{
	EFI_ENTRY("%p, %p", event, context);

	if (!new_rx_packet) {
		push_packet = efi_net_push;
		eth_rx();
		push_packet = NULL;
	}
	EFI_EXIT(EFI_SUCCESS);
}

/* This gets called from do_bootefi_exec(). */
int efi_net_register(void)
{
	struct efi_net_obj *netobj;
	efi_status_t r;

	if (!eth_get_dev()) {
		/* No eth device active, don't expose any */
		return 0;
	}

	/* We only expose the "active" eth device, so one is enough */
	netobj = calloc(1, sizeof(*netobj));

	/* Fill in object data */
	netobj->parent.protocols[0].guid = &efi_net_guid;
	netobj->parent.protocols[0].protocol_interface = &netobj->net;
	netobj->parent.protocols[1].guid = &efi_guid_device_path;
	netobj->parent.protocols[1].protocol_interface =
		efi_dp_from_eth();
	netobj->parent.protocols[2].guid = &efi_pxe_guid;
	netobj->parent.protocols[2].protocol_interface = &netobj->pxe;
	netobj->parent.handle = &netobj->net;
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
	netobj->net_mode.state = EFI_NETWORK_STARTED;
	memcpy(netobj->net_mode.current_address.mac_addr, eth_get_ethaddr(), 6);
	netobj->net_mode.hwaddr_size = ARP_HLEN;
	netobj->net_mode.max_packet_size = PKTSIZE;

	netobj->pxe.mode = &netobj->pxe_mode;
	if (dhcp_ack)
		netobj->pxe_mode.dhcp_ack = *dhcp_ack;

	/* Hook net up to the device list */
	list_add_tail(&netobj->parent.link, &efi_obj_list);

	/*
	 * Create WaitForPacket event.
	 */
	r = efi_create_event(EVT_NOTIFY_WAIT, TPL_CALLBACK,
			     efi_network_timer_notify, NULL,
			     &wait_for_packet);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	netobj->net.wait_for_packet = wait_for_packet;
	/*
	 * Create a timer event.
	 *
	 * The notification function is used to check if a new network packet
	 * has been received.
	 */
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			     efi_network_timer_notify, NULL,
			     &network_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	/* Network is time critical, create event in every timer cyle */
	r = efi_set_timer(network_timer_event, EFI_TIMER_PERIODIC, 0);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to set network timer\n");
		return r;
	}

	return 0;
}
