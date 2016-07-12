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

struct efi_net_obj {
	/* Generic EFI object parent class data */
	struct efi_object parent;
	/* EFI Interface callback struct for network */
	struct efi_simple_network net;
	struct efi_simple_network_mode net_mode;
	/* Device path to the network adapter */
	struct efi_device_path_file_path dp[2];
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

	/* XXX Do we care? */

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_station_address(
		struct efi_simple_network *this, int reset,
		struct efi_mac_address *new_mac)
{
	EFI_ENTRY("%p, %x, %p", this, reset, new_mac);

	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_net_statistics(struct efi_simple_network *this,
					      int reset, ulong *stat_size,
					      void *stat_table)
{
	EFI_ENTRY("%p, %x, %p, %p", this, reset, stat_size, stat_table);

	return EFI_EXIT(EFI_INVALID_PARAMETER);
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

	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

static efi_status_t EFIAPI efi_net_get_status(struct efi_simple_network *this,
					      u32 *int_status, void **txbuf)
{
	EFI_ENTRY("%p, %p, %p", this, int_status, txbuf);

	/* We send packets synchronously, so nothing is outstanding */
	if (int_status)
		*int_status = 0;
	if (txbuf)
		*txbuf = new_tx_packet;

	new_tx_packet = NULL;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t EFIAPI efi_net_transmit(struct efi_simple_network *this,
		ulong header_size, ulong buffer_size, void *buffer,
		struct efi_mac_address *src_addr,
		struct efi_mac_address *dest_addr, u16 *protocol)
{
	EFI_ENTRY("%p, %lx, %lx, %p, %p, %p, %p", this, header_size,
		  buffer_size, buffer, src_addr, dest_addr, protocol);

	if (header_size) {
		/* We would need to create the header if header_size != 0 */
		return EFI_EXIT(EFI_INVALID_PARAMETER);
	}

	net_send_packet(buffer, buffer_size);
	new_tx_packet = buffer;

	return EFI_EXIT(EFI_SUCCESS);
}

static void efi_net_push(void *pkt, int len)
{
	new_rx_packet = true;
}

static efi_status_t EFIAPI efi_net_receive(struct efi_simple_network *this,
		ulong *header_size, ulong *buffer_size, void *buffer,
		struct efi_mac_address *src_addr,
		struct efi_mac_address *dest_addr, u16 *protocol)
{
	EFI_ENTRY("%p, %p, %p, %p, %p, %p, %p", this, header_size,
		  buffer_size, buffer, src_addr, dest_addr, protocol);

	push_packet = efi_net_push;
	eth_rx();
	push_packet = NULL;

	if (!new_rx_packet)
		return EFI_EXIT(EFI_NOT_READY);

	if (*buffer_size < net_rx_packet_len) {
		/* Packet doesn't fit, try again with bigger buf */
		*buffer_size = net_rx_packet_len;
		return EFI_EXIT(EFI_BUFFER_TOO_SMALL);
	}

	memcpy(buffer, net_rx_packet, net_rx_packet_len);
	*buffer_size = net_rx_packet_len;
	new_rx_packet = false;

	return EFI_EXIT(EFI_SUCCESS);
}

static efi_status_t efi_net_open_dp(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_simple_network *net = handle;
	struct efi_net_obj *netobj = container_of(net, struct efi_net_obj, net);

	*protocol_interface = netobj->dp;

	return EFI_SUCCESS;
}

static efi_status_t efi_net_open_pxe(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	struct efi_simple_network *net = handle;
	struct efi_net_obj *netobj = container_of(net, struct efi_net_obj, net);

	*protocol_interface = &netobj->pxe;

	return EFI_SUCCESS;
}

void efi_net_set_dhcp_ack(void *pkt, int len)
{
	int maxsize = sizeof(*dhcp_ack);

	if (!dhcp_ack)
		dhcp_ack = malloc(maxsize);

	memcpy(dhcp_ack, pkt, min(len, maxsize));
}

/* This gets called from do_bootefi_exec(). */
int efi_net_register(void **handle)
{
	struct efi_net_obj *netobj;
	struct efi_device_path_file_path dp_net = {
		.dp.type = DEVICE_PATH_TYPE_MEDIA_DEVICE,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_FILE_PATH,
		.dp.length = sizeof(dp_net),
		.str = { 'N', 'e', 't' },
	};
	struct efi_device_path_file_path dp_end = {
		.dp.type = DEVICE_PATH_TYPE_END,
		.dp.sub_type = DEVICE_PATH_SUB_TYPE_END,
		.dp.length = sizeof(dp_end),
	};

	if (!eth_get_dev()) {
		/* No eth device active, don't expose any */
		return 0;
	}

	/* We only expose the "active" eth device, so one is enough */
	netobj = calloc(1, sizeof(*netobj));

	/* Fill in object data */
	netobj->parent.protocols[0].guid = &efi_net_guid;
	netobj->parent.protocols[0].open = efi_return_handle;
	netobj->parent.protocols[1].guid = &efi_guid_device_path;
	netobj->parent.protocols[1].open = efi_net_open_dp;
	netobj->parent.protocols[2].guid = &efi_pxe_guid;
	netobj->parent.protocols[2].open = efi_net_open_pxe;
	netobj->parent.handle = &netobj->net;
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
	netobj->dp[0] = dp_net;
	netobj->dp[1] = dp_end;
	memcpy(netobj->net_mode.current_address.mac_addr, eth_get_ethaddr(), 6);
	netobj->net_mode.max_packet_size = PKTSIZE;

	netobj->pxe.mode = &netobj->pxe_mode;
	if (dhcp_ack)
		netobj->pxe_mode.dhcp_ack = *dhcp_ack;

	/* Hook net up to the device list */
	list_add_tail(&netobj->parent.link, &efi_obj_list);

	if (handle)
		*handle = &netobj->net;

	return 0;
}
