// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 Duncan Hare, all rights reserved.
 */

/*
 * General Desription:
 *
 * TCP support for the wget command, for fast file downloading.
 *
 * HTTP/TCP Receiver:
 *
 *      Prerequisites:  - own ethernet address
 *                      - own IP address
 *                      - Server IP address
 *                      - Server with TCP
 *                      - TCP application (eg wget)
 *      Next Step       HTTPS?
 */
#include <command.h>
#include <console.h>
#include <env_internal.h>
#include <errno.h>
#include <net.h>
#include <net/tcp.h>

/*
 * The start sequence number increment for the two sequently created
 * connections within the same timer tick. This number must be:
 *  - prime (to increase the time before the same number will be generated)
 *  - larger than typical MTU (to avoid similar numbers for two sequently
 *    created connections)
 */
#define TCP_START_SEQ_INC	2153	/* just large prime number */

#define TCP_SEND_RETRY		3
#define TCP_SEND_TIMEOUT	2000UL
#define TCP_RX_INACTIVE_TIMEOUT	30000UL
#if PKTBUFSRX != 0
  #define TCP_RCV_WND_SIZE	(PKTBUFSRX * TCP_MSS)
#else
  #define TCP_RCV_WND_SIZE	(4 * TCP_MSS)
#endif

#define TCP_PACKET_OK		0
#define TCP_PACKET_DROP		1

static struct tcp_stream tcp_stream;

static int (*tcp_stream_on_create)(struct tcp_stream *tcp);

/*
 * TCP lengths are stored as a rounded up number of 32 bit words.
 * Add 3 to length round up, rounded, then divided into the
 * length in 32 bit words.
 */
#define LEN_B_TO_DW(x) ((x) >> 2)
#define ROUND_TCPHDR_LEN(x) (LEN_B_TO_DW((x) + 3))
#define ROUND_TCPHDR_BYTES(x) (((x) + 3) & ~3)
#define SHIFT_TO_TCPHDRLEN_FIELD(x) ((x) << 4)
#define GET_TCP_HDR_LEN_IN_BYTES(x) ((x) >> 2)

#define RANDOM_PORT_START 1024
#define RANDOM_PORT_RANGE 0x4000

/**
 * random_port() - make port a little random (1024-17407)
 *
 * Return: random port number from 1024 to 17407
 *
 * This keeps the math somewhat trivial to compute, and seems to work with
 * all supported protocols/clients/servers
 */
static uint random_port(void)
{
	return RANDOM_PORT_START + (get_timer(0) % RANDOM_PORT_RANGE);
}

static inline s32 tcp_seq_cmp(u32 a, u32 b)
{
	return (s32)(a - b);
}

static inline u32 tcp_get_start_seq(void)
{
	static u32	tcp_seq_inc;
	u32		tcp_seq;

	tcp_seq = (get_timer(0) & 0xffffffff) + tcp_seq_inc;
	tcp_seq_inc += TCP_START_SEQ_INC;

	return tcp_seq;
}

static inline ulong msec_to_ticks(ulong msec)
{
	return msec * CONFIG_SYS_HZ / 1000;
}

/**
 * tcp_stream_get_state() - get TCP stream state
 * @tcp: tcp stream
 *
 * Return: TCP stream state
 */
enum tcp_state tcp_stream_get_state(struct tcp_stream *tcp)
{
	return tcp->state;
}

/**
 * tcp_stream_set_state() - set TCP stream state
 * @tcp: tcp stream
 * @new_state: new TCP state
 */
static void tcp_stream_set_state(struct tcp_stream *tcp,
				 enum tcp_state new_state)
{
	tcp->state = new_state;
}

/**
 * tcp_stream_get_status() - get TCP stream status
 * @tcp: tcp stream
 *
 * Return: TCP stream status
 */
enum tcp_status tcp_stream_get_status(struct tcp_stream *tcp)
{
	return tcp->status;
}

/**
 * tcp_stream_set_status() - set TCP stream state
 * @tcp: tcp stream
 * @new_satus: new TCP stream status
 */
static void tcp_stream_set_status(struct tcp_stream *tcp,
				  enum tcp_status new_status)
{
	tcp->status = new_status;
}

void tcp_stream_restart_rx_timer(struct tcp_stream *tcp)
{
	tcp->time_last_rx = get_timer(0);
}

static void tcp_stream_init(struct tcp_stream *tcp,
			    struct in_addr rhost, u16 rport, u16 lport)
{
	memset(tcp, 0, sizeof(struct tcp_stream));
	tcp->rhost.s_addr = rhost.s_addr;
	tcp->rport = rport;
	tcp->lport = lport;
	tcp->state = TCP_CLOSED;
	tcp->lost.len = TCP_OPT_LEN_2;
	tcp->rcv_wnd = TCP_RCV_WND_SIZE;
	tcp->max_retry_count = TCP_SEND_RETRY;
	tcp->initial_timeout = TCP_SEND_TIMEOUT;
	tcp->rx_inactiv_timeout = TCP_RX_INACTIVE_TIMEOUT;
	tcp_stream_restart_rx_timer(tcp);
}

static void tcp_stream_destroy(struct tcp_stream *tcp)
{
	if (tcp->on_closed)
		tcp->on_closed(tcp);
	memset(tcp, 0, sizeof(struct tcp_stream));
}

void tcp_init(void)
{
	static int initialized;
	struct tcp_stream *tcp = &tcp_stream;

	tcp_stream_on_create = NULL;
	if (!initialized) {
		initialized = 1;
		memset(tcp, 0, sizeof(struct tcp_stream));
	}

	tcp_stream_set_state(tcp, TCP_CLOSED);
	tcp_stream_set_status(tcp, TCP_ERR_RST);
	tcp_stream_destroy(tcp);
}

void tcp_stream_set_on_create_handler(int (*on_create)(struct tcp_stream *))
{
	tcp_stream_on_create = on_create;
}

static struct tcp_stream *tcp_stream_add(struct in_addr rhost,
					 u16 rport, u16 lport)
{
	struct tcp_stream *tcp = &tcp_stream;

	if (!tcp_stream_on_create ||
	    tcp->state != TCP_CLOSED)
		return NULL;

	tcp_stream_init(tcp, rhost, rport, lport);
	if (!tcp_stream_on_create(tcp))
		return NULL;

	return tcp;
}

struct tcp_stream *tcp_stream_get(int is_new, struct in_addr rhost,
				  u16 rport, u16 lport)
{
	struct tcp_stream *tcp = &tcp_stream;

	if (tcp->rhost.s_addr == rhost.s_addr &&
	    tcp->rport == rport &&
	    tcp->lport == lport)
		return tcp;

	return is_new ? tcp_stream_add(rhost, rport, lport) : NULL;
}

void tcp_stream_put(struct tcp_stream *tcp)
{
	if (tcp->state == TCP_CLOSED)
		tcp_stream_destroy(tcp);
}

u32 tcp_stream_rx_offs(struct tcp_stream *tcp)
{
	u32 ret;

	switch (tcp->state) {
	case TCP_CLOSED:
	case TCP_SYN_SENT:
	case TCP_SYN_RECEIVED:
		return 0;
	default:
		break;
	}

	ret = tcp->rcv_nxt - tcp->irs - 1;
	if (tcp->fin_rx && tcp->rcv_nxt == tcp->fin_rx_seq)
		ret--;

	return ret;
}

u32 tcp_stream_tx_offs(struct tcp_stream *tcp)
{
	u32 ret;

	switch (tcp->state) {
	case TCP_CLOSED:
	case TCP_SYN_SENT:
	case TCP_SYN_RECEIVED:
		return 0;
	default:
		break;
	}

	ret = tcp->snd_una - tcp->iss - 1;
	if (tcp->fin_tx && tcp->snd_una == tcp->fin_tx_seq + 1)
		ret--;

	return ret;
}

static void tcp_stream_set_time_handler(struct tcp_stream *tcp, ulong msec,
					void (*handler)(struct tcp_stream *))
{
	if (!msec) {
		tcp->time_handler = NULL;
		return;
	}

	tcp->time_handler = handler;
	tcp->time_start = get_timer(0);
	tcp->time_delta = msec_to_ticks(msec);
}

static void tcp_send_packet(struct tcp_stream *tcp, u8 action,
			    u32 tcp_seq_num, u32 tcp_ack_num, u32 tx_len)
{
	tcp->tx_packets++;
	net_send_tcp_packet(tx_len, tcp->rhost, tcp->rport,
			    tcp->lport, action, tcp_seq_num,
			    tcp_ack_num);
}

static void tcp_send_repeat(struct tcp_stream *tcp)
{
	uchar *ptr;
	u32 tcp_opts_size;
	int ret;

	if (!tcp->retry_cnt) {
		puts("\nTCP: send retry counter exceeded\n");
		tcp_send_packet(tcp, TCP_RST, tcp->retry_seq_num,
				tcp->rcv_nxt, 0);
		tcp_stream_set_status(tcp, TCP_ERR_TOUT);
		tcp_stream_set_state(tcp, TCP_CLOSED);
		tcp_stream_destroy(tcp);
		return;
	}
	tcp->retry_cnt--;
	tcp->retry_timeout += tcp->initial_timeout;

	if (tcp->retry_tx_len > 0) {
		tcp_opts_size = ROUND_TCPHDR_BYTES(TCP_TSOPT_SIZE +
						   tcp->lost.len);
		ptr = net_tx_packet + net_eth_hdr_size() +
			IP_TCP_HDR_SIZE + tcp_opts_size;

		if (tcp->retry_tx_len > TCP_MSS - tcp_opts_size)
			tcp->retry_tx_len = TCP_MSS - tcp_opts_size;

		/* refill packet data */
		ret = tcp->tx(tcp, tcp->retry_tx_offs, ptr, tcp->retry_tx_len);
		if (ret < 0) {
			puts("\nTCP: send failure\n");
			tcp_send_packet(tcp, TCP_RST, tcp->retry_seq_num,
					tcp->rcv_nxt, 0);
			tcp_stream_set_status(tcp, TCP_ERR_IO);
			tcp_stream_set_state(tcp, TCP_CLOSED);
			tcp_stream_destroy(tcp);
			return;
		}
	}
	tcp_send_packet(tcp, tcp->retry_action, tcp->retry_seq_num,
			tcp->rcv_nxt, tcp->retry_tx_len);

	tcp_stream_set_time_handler(tcp, tcp->retry_timeout, tcp_send_repeat);
}

static void tcp_send_packet_with_retry(struct tcp_stream *tcp, u8 action,
				       u32 tcp_seq_num, u32 tx_len, u32 tx_offs)
{
	tcp->retry_cnt = tcp->max_retry_count;
	tcp->retry_timeout = tcp->initial_timeout;
	tcp->retry_action = action;
	tcp->retry_seq_num = tcp_seq_num;
	tcp->retry_tx_len = tx_len;
	tcp->retry_tx_offs = tx_offs;

	tcp_send_packet(tcp, action, tcp_seq_num, tcp->rcv_nxt, tx_len);
	tcp_stream_set_time_handler(tcp, tcp->retry_timeout, tcp_send_repeat);
}

static inline u8 tcp_stream_fin_needed(struct tcp_stream *tcp, u32 tcp_seq_num)
{
	return (tcp->fin_tx && (tcp_seq_num == tcp->fin_tx_seq)) ? TCP_FIN : 0;
}

static void tcp_steam_tx_try(struct tcp_stream *tcp)
{
	uchar *ptr;
	int tx_len;
	u32 tx_offs, tcp_opts_size;

	if (tcp->state != TCP_ESTABLISHED ||
	    tcp->time_handler ||
	    !tcp->tx)
		return;

	tcp_opts_size = ROUND_TCPHDR_BYTES(TCP_TSOPT_SIZE + tcp->lost.len);
	tx_len = TCP_MSS - tcp_opts_size;
	if (tcp->fin_tx) {
		/* do not try to send beyonds FIN packet limits */
		if (tcp_seq_cmp(tcp->snd_una, tcp->fin_tx_seq) >= 0)
			return;

		tx_len = tcp->fin_tx_seq - tcp->snd_una;
		if (tx_len > TCP_MSS - tcp_opts_size)
			tx_len = TCP_MSS - tcp_opts_size;
	}

	tx_offs = tcp_stream_tx_offs(tcp);
	ptr = net_tx_packet + net_eth_hdr_size() +
		IP_TCP_HDR_SIZE + tcp_opts_size;

	/* fill packet data and adjust size */
	tx_len = tcp->tx(tcp, tx_offs, ptr, tx_len);
	if (tx_len < 0) {
		puts("\nTCP: send failure\n");
		tcp_send_packet(tcp, TCP_RST, tcp->retry_seq_num,
				tcp->rcv_nxt, 0);
		tcp_stream_set_status(tcp, TCP_ERR_IO);
		tcp_stream_set_state(tcp, TCP_CLOSED);
		tcp_stream_destroy(tcp);
		return;
	}
	if (!tx_len)
		return;

	if (tcp_seq_cmp(tcp->snd_una + tx_len, tcp->snd_nxt) > 0)
		tcp->snd_nxt = tcp->snd_una + tx_len;

	tcp_send_packet_with_retry(tcp, TCP_ACK | TCP_PUSH,
				   tcp->snd_una, tx_len, tx_offs);
}

static void tcp_stream_poll(struct tcp_stream *tcp, ulong time)
{
	ulong	delta;
	void	(*handler)(struct tcp_stream *tcp);

	if (tcp->state == TCP_CLOSED)
		return;

	/* handle rx inactivity timeout */
	delta = msec_to_ticks(tcp->rx_inactiv_timeout);
	if (time - tcp->time_last_rx >= delta) {
		puts("\nTCP: rx inactivity timeout exceeded\n");
		tcp_stream_reset(tcp);
		tcp_stream_set_status(tcp, TCP_ERR_TOUT);
		tcp_stream_destroy(tcp);
		return;
	}

	/* handle retransmit timeout */
	if (tcp->time_handler &&
	    time - tcp->time_start >= tcp->time_delta) {
		handler = tcp->time_handler;
		tcp->time_handler = NULL;
		handler(tcp);
	}

	tcp_steam_tx_try(tcp);
}

void tcp_streams_poll(void)
{
	ulong			time;
	struct tcp_stream	*tcp;

	time = get_timer(0);
	tcp = &tcp_stream;
	tcp_stream_poll(tcp, time);
}

/**
 * tcp_set_pseudo_header() - set TCP pseudo header
 * @pkt: the packet
 * @src: source IP address
 * @dest: destinaion IP address
 * @tcp_len: tcp length
 * @pkt_len: packet length
 *
 * Return: the checksum of the packet
 */
u16 tcp_set_pseudo_header(uchar *pkt, struct in_addr src, struct in_addr dest,
			  int tcp_len, int pkt_len)
{
	union tcp_build_pkt *b = (union tcp_build_pkt *)pkt;
	int checksum_len;

	/*
	 * Pseudo header
	 *
	 * Zero the byte after the last byte so that the header checksum
	 * will always work.
	 */
	pkt[pkt_len] = 0;

	net_copy_ip((void *)&b->ph.p_src, &src);
	net_copy_ip((void *)&b->ph.p_dst, &dest);
	b->ph.rsvd = 0;
	b->ph.p	= IPPROTO_TCP;
	b->ph.len = htons(tcp_len);
	checksum_len = tcp_len + PSEUDO_HDR_SIZE;

	debug_cond(DEBUG_DEV_PKT,
		   "TCP Pesudo  Header  (to=%pI4, from=%pI4, Len=%d)\n",
		   &b->ph.p_dst, &b->ph.p_src, checksum_len);

	return compute_ip_checksum(pkt + PSEUDO_PAD_SIZE, checksum_len);
}

/**
 * net_set_ack_options() - set TCP options in acknowledge packets
 * @tcp: tcp stream
 * @b: the packet
 *
 * Return: TCP header length
 */
int net_set_ack_options(struct tcp_stream *tcp, union tcp_build_pkt *b)
{
	b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));

	b->sack.t_opt.kind = TCP_O_TS;
	b->sack.t_opt.len = TCP_OPT_LEN_A;
	b->sack.t_opt.t_snd = htons(tcp->loc_timestamp);
	b->sack.t_opt.t_rcv = tcp->rmt_timestamp;
	b->sack.sack_v.kind = TCP_1_NOP;
	b->sack.sack_v.len = 0;

	if (IS_ENABLED(CONFIG_PROT_TCP_SACK)) {
		if (tcp->lost.len > TCP_OPT_LEN_2) {
			debug_cond(DEBUG_DEV_PKT, "TCP ack opt lost.len %x\n",
				   tcp->lost.len);
			b->sack.sack_v.len = tcp->lost.len;
			b->sack.sack_v.kind = TCP_V_SACK;
			b->sack.sack_v.hill[0].l = htonl(tcp->lost.hill[0].l);
			b->sack.sack_v.hill[0].r = htonl(tcp->lost.hill[0].r);

			/*
			 * These SACK structures are initialized with NOPs to
			 * provide TCP header alignment padding. There are 4
			 * SACK structures used for both header padding and
			 * internally.
			 */
			b->sack.sack_v.hill[1].l = htonl(tcp->lost.hill[1].l);
			b->sack.sack_v.hill[1].r = htonl(tcp->lost.hill[1].r);
			b->sack.sack_v.hill[2].l = htonl(tcp->lost.hill[2].l);
			b->sack.sack_v.hill[2].r = htonl(tcp->lost.hill[2].r);
			b->sack.sack_v.hill[3].l = TCP_O_NOP;
			b->sack.sack_v.hill[3].r = TCP_O_NOP;
		}

		b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(ROUND_TCPHDR_LEN(TCP_HDR_SIZE +
										 TCP_TSOPT_SIZE +
										 tcp->lost.len));
	} else {
		b->sack.sack_v.kind = 0;
		b->sack.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(ROUND_TCPHDR_LEN(TCP_HDR_SIZE +
										 TCP_TSOPT_SIZE));
	}

	/*
	 * This returns the actual rounded up length of the
	 * TCP header to add to the total packet length
	 */
	return GET_TCP_HDR_LEN_IN_BYTES(b->sack.hdr.tcp_hlen);
}

/**
 * net_set_syn_options() - set TCP options in SYN packets
 * @tcp: tcp stream
 * @b: the packet
 */
void net_set_syn_options(struct tcp_stream *tcp, union tcp_build_pkt *b)
{
	if (IS_ENABLED(CONFIG_PROT_TCP_SACK))
		tcp->lost.len = 0;

	b->ip.hdr.tcp_hlen = 0xa0;

	b->ip.mss.kind = TCP_O_MSS;
	b->ip.mss.len = TCP_OPT_LEN_4;
	b->ip.mss.mss = htons(TCP_MSS);
	b->ip.scale.kind = TCP_O_SCL;
	b->ip.scale.scale = TCP_SCALE;
	b->ip.scale.len = TCP_OPT_LEN_3;
	if (IS_ENABLED(CONFIG_PROT_TCP_SACK)) {
		b->ip.sack_p.kind = TCP_P_SACK;
		b->ip.sack_p.len = TCP_OPT_LEN_2;
	} else {
		b->ip.sack_p.kind = TCP_1_NOP;
		b->ip.sack_p.len = TCP_1_NOP;
	}
	b->ip.t_opt.kind = TCP_O_TS;
	b->ip.t_opt.len = TCP_OPT_LEN_A;
	tcp->loc_timestamp = get_ticks();
	tcp->rmt_timestamp = 0;
	b->ip.t_opt.t_snd = 0;
	b->ip.t_opt.t_rcv = 0;
	b->ip.end = TCP_O_END;
}

const char *tcpflags_to_str(char tcpflags, char *buf, int size)
{
	int i;
	static const struct {
		int		bit;
		const char	*name;
	} desc[] = {{TCP_RST, "RST"}, {TCP_SYN, "SYN"}, {TCP_PUSH, "PSH"},
		    {TCP_FIN, "FIN"}, {TCP_ACK, "ACK"}};

	*buf = '\0';
	for (i = 0; i < ARRAY_SIZE(desc); i++) {
		if (!(tcpflags & desc[i].bit))
			continue;

		if (*buf)
			strlcat(buf, ",", size);
		strlcat(buf, desc[i].name, size);
	}

	return buf;
}

int tcp_set_tcp_header(struct tcp_stream *tcp, uchar *pkt, int payload_len,
		       u8 action, u32 tcp_seq_num, u32 tcp_ack_num)
{
	union tcp_build_pkt *b = (union tcp_build_pkt *)pkt;
	char buf[24];
	int pkt_hdr_len;
	int pkt_len;
	int tcp_len;

	/*
	 * Header: 5 32 bit words. 4 bits TCP header Length,
	 *         4 bits reserved options
	 */
	b->ip.hdr.tcp_flags = action;
	b->ip.hdr.tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));

	switch (action) {
	case TCP_SYN:
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:%s (%pI4, %pI4, s=%u, a=%u)\n",
			   tcpflags_to_str(action, buf, sizeof(buf)),
			   &tcp->rhost, &net_ip, tcp_seq_num, tcp_ack_num);
		net_set_syn_options(tcp, b);
		pkt_hdr_len = IP_TCP_O_SIZE;
		break;
	case TCP_RST | TCP_ACK:
	case TCP_RST:
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:%s (%pI4, %pI4, s=%u, a=%u)\n",
			   tcpflags_to_str(action, buf, sizeof(buf)),
			   &tcp->rhost, &net_ip, tcp_seq_num, tcp_ack_num);
		pkt_hdr_len = IP_TCP_HDR_SIZE;
		break;
	default:
		pkt_hdr_len = IP_HDR_SIZE + net_set_ack_options(tcp, b);
		debug_cond(DEBUG_DEV_PKT,
			   "TCP Hdr:%s (%pI4, %pI4, s=%u, a=%u)\n",
			   tcpflags_to_str(action, buf, sizeof(buf)),
			   &tcp->rhost, &net_ip, tcp_seq_num, tcp_ack_num);
		break;
	}

	pkt_len	= pkt_hdr_len + payload_len;
	tcp_len	= pkt_len - IP_HDR_SIZE;

	tcp->rcv_nxt = tcp_ack_num;
	/* TCP Header */
	b->ip.hdr.tcp_ack = htonl(tcp->rcv_nxt);
	b->ip.hdr.tcp_src = htons(tcp->lport);
	b->ip.hdr.tcp_dst = htons(tcp->rport);
	b->ip.hdr.tcp_seq = htonl(tcp_seq_num);

	/*
	 * TCP window size - TCP header variable tcp_win.
	 * Change tcp_win only if you have an understanding of network
	 * overrun, congestion, TCP segment sizes, TCP windows, TCP scale,
	 * queuing theory  and packet buffering. If there are too few buffers,
	 * there will be data loss, recovery may work or the sending TCP,
	 * the server, could abort the stream transmission.
	 * MSS is governed by maximum Ethernet frame length.
	 * The number of buffers is governed by the desire to have a queue of
	 * full buffers to be processed at the destination to maximize
	 * throughput. Temporary memory use for the boot phase on modern
	 * SOCs is may not be considered a constraint to buffer space, if
	 * it is, then the u-boot tftp or nfs kernel netboot should be
	 * considered.
	 */
	b->ip.hdr.tcp_win = htons(tcp->rcv_wnd >> TCP_SCALE);

	b->ip.hdr.tcp_xsum = 0;
	b->ip.hdr.tcp_ugr = 0;

	b->ip.hdr.tcp_xsum = tcp_set_pseudo_header(pkt, net_ip, tcp->rhost,
						   tcp_len, pkt_len);

	net_set_ip_header((uchar *)&b->ip, tcp->rhost, net_ip,
			  pkt_len, IPPROTO_TCP);

	return pkt_hdr_len;
}

static void tcp_update_rcv_nxt(struct tcp_stream *tcp)
{
	if (tcp_seq_cmp(tcp->rcv_nxt, tcp->lost.hill[0].l) >= 0) {
		tcp->rcv_nxt = tcp->lost.hill[0].r;

		memmove(&tcp->lost.hill[0], &tcp->lost.hill[1],
			(TCP_SACK_HILLS - 1) * sizeof(struct sack_edges));

		tcp->lost.len -= TCP_OPT_LEN_8;
		tcp->lost.hill[TCP_SACK_HILLS - 1].l = TCP_O_NOP;
		tcp->lost.hill[TCP_SACK_HILLS - 1].r = TCP_O_NOP;
	}
}

/**
 * tcp_hole() - Selective Acknowledgment (Essential for fast stream transfer)
 * @tcp: tcp stream
 * @tcp_seq_num: TCP sequence start number
 * @len: the length of sequence numbers
 */
void tcp_hole(struct tcp_stream *tcp, u32 tcp_seq_num, u32 len)
{
	int i, j, cnt, cnt_move;

	cnt = (tcp->lost.len - TCP_OPT_LEN_2) / TCP_OPT_LEN_8;
	for (i = 0; i < cnt; i++) {
		if (tcp_seq_cmp(tcp->lost.hill[i].r, tcp_seq_num) < 0)
			continue;
		if (tcp_seq_cmp(tcp->lost.hill[i].l, tcp_seq_num + len) > 0)
			break;

		if (tcp_seq_cmp(tcp->lost.hill[i].l, tcp_seq_num) > 0)
			tcp->lost.hill[i].l = tcp_seq_num;
		if (tcp_seq_cmp(tcp->lost.hill[i].l, tcp_seq_num) < 0) {
			len += tcp_seq_num - tcp->lost.hill[i].l;
			tcp_seq_num = tcp->lost.hill[i].l;
		}
		if (tcp_seq_cmp(tcp->lost.hill[i].r, tcp_seq_num + len) >= 0) {
			tcp_update_rcv_nxt(tcp);
			return;
		}

		/* check overlapping with next hills */
		cnt_move = 0;
		tcp->lost.hill[i].r = tcp_seq_num + len;
		for (j = i + 1; j < cnt; j++) {
			if (tcp_seq_cmp(tcp->lost.hill[j].l, tcp->lost.hill[i].r) > 0)
				break;

			tcp->lost.hill[i].r = tcp->lost.hill[j].r;
			cnt_move++;
		}

		if (cnt_move > 0) {
			if (cnt > i + cnt_move + 1)
				memmove(&tcp->lost.hill[i + 1],
					&tcp->lost.hill[i + cnt_move + 1],
					cnt_move * sizeof(struct sack_edges));

			cnt -= cnt_move;
			tcp->lost.len = TCP_OPT_LEN_2 + cnt * TCP_OPT_LEN_8;
			for (j = cnt; j < TCP_SACK_HILLS; j++) {
				tcp->lost.hill[j].l = TCP_O_NOP;
				tcp->lost.hill[j].r = TCP_O_NOP;
			}
		}

		tcp_update_rcv_nxt(tcp);
		return;
	}

	if (i == TCP_SACK_HILLS) {
		tcp_update_rcv_nxt(tcp);
		return;
	}

	if (cnt < TCP_SACK_HILLS) {
		cnt_move = cnt - i;
		cnt++;
	} else {
		cnt = TCP_SACK_HILLS;
		cnt_move = TCP_SACK_HILLS - i;
	}

	if (cnt_move > 0)
		memmove(&tcp->lost.hill[i + 1],
			&tcp->lost.hill[i],
			cnt_move * sizeof(struct sack_edges));

	tcp->lost.hill[i].l = tcp_seq_num;
	tcp->lost.hill[i].r = tcp_seq_num + len;
	tcp->lost.len = TCP_OPT_LEN_2 + cnt * TCP_OPT_LEN_8;

	tcp_update_rcv_nxt(tcp);
};

/**
 * tcp_parse_options() - parsing TCP options
 * @tcp: tcp stream
 * @o: pointer to the option field.
 * @o_len: length of the option field.
 */
void tcp_parse_options(struct tcp_stream *tcp, uchar *o, int o_len)
{
	struct tcp_t_opt  *tsopt;
	struct tcp_scale  *wsopt;
	uchar *p = o;

	/*
	 * NOPs are options with a zero length, and thus are special.
	 * All other options have length fields.
	 */
	for (p = o; p < (o + o_len); ) {
		if (!p[1])
			return; /* Finished processing options */

		switch (p[0]) {
		case TCP_O_END:
			return;
		case TCP_O_MSS:
		case TCP_P_SACK:
		case TCP_V_SACK:
			break;
		case TCP_O_SCL:
			wsopt = (struct tcp_scale *)p;
			tcp->rmt_win_scale = wsopt->scale;
			break;
		case TCP_O_TS:
			tsopt = (struct tcp_t_opt *)p;
			tcp->rmt_timestamp = tsopt->t_snd;
			break;
		}

		/* Process optional NOPs */
		if (p[0] == TCP_1_NOP)
			p++;
		else
			p += p[1];
	}
}

static int tcp_seg_in_wnd(struct tcp_stream *tcp,
			  u32 tcp_seq_num, int payload_len)
{
	if (!payload_len && !tcp->rcv_wnd) {
		if (tcp_seq_num == tcp->rcv_nxt)
			return 1;
	}
	if (!payload_len && tcp->rcv_wnd > 0) {
		if (tcp_seq_cmp(tcp->rcv_nxt, tcp_seq_num) <= 0 &&
		    tcp_seq_cmp(tcp_seq_num, tcp->rcv_nxt + tcp->rcv_wnd) < 0)
			return 1;
	}
	if (payload_len > 0 && tcp->rcv_wnd > 0) {
		if (tcp_seq_cmp(tcp->rcv_nxt, tcp_seq_num) <= 0 &&
		    tcp_seq_cmp(tcp_seq_num, tcp->rcv_nxt + tcp->rcv_wnd) < 0)
			return 1;
		tcp_seq_num += payload_len - 1;
		if (tcp_seq_cmp(tcp->rcv_nxt, tcp_seq_num) <= 0 &&
		    tcp_seq_cmp(tcp_seq_num, tcp->rcv_nxt + tcp->rcv_wnd) < 0)
			return 1;
	}

	return 0;
}

static int tcp_rx_check_ack_num(struct tcp_stream *tcp, u32 tcp_seq_num,
				u32 tcp_ack_num, u32 tcp_win_size)
{
	u32 old_offs, new_offs;
	u8 action;

	switch (tcp->state) {
	case TCP_SYN_RECEIVED:
		if (tcp_seq_cmp(tcp->snd_una, tcp_ack_num) >= 0 ||
		    tcp_seq_cmp(tcp_ack_num, tcp->snd_nxt) > 0) {
			// segment acknowledgment is not acceptable
			tcp_send_packet(tcp, TCP_RST, tcp_ack_num, 0, 0);
			return TCP_PACKET_DROP;
		}

		tcp_stream_set_state(tcp, TCP_ESTABLISHED);
		tcp->snd_wnd = tcp_win_size;
		tcp->snd_wl1 = tcp_seq_num;
		tcp->snd_wl2 = tcp_ack_num;

		if (tcp->on_established)
			tcp->on_established(tcp);

		fallthrough;

	case TCP_ESTABLISHED:
	case TCP_FIN_WAIT_1:
	case TCP_FIN_WAIT_2:
	case TCP_CLOSE_WAIT:
	case TCP_CLOSING:
		if (tcp_seq_cmp(tcp_ack_num, tcp->snd_nxt) > 0) {
			// ACK acks something not yet sent
			action = tcp_stream_fin_needed(tcp, tcp->snd_una) | TCP_ACK;
			tcp_send_packet(tcp, action, tcp->snd_una, tcp->rcv_nxt, 0);
			return TCP_PACKET_DROP;
		}

		if (tcp_seq_cmp(tcp->snd_una, tcp_ack_num) < 0) {
			old_offs = tcp_stream_tx_offs(tcp);
			tcp->snd_una = tcp_ack_num;
			new_offs = tcp_stream_tx_offs(tcp);
			if (tcp->time_handler &&
			    tcp_seq_cmp(tcp->snd_una, tcp->retry_seq_num) > 0) {
				tcp_stream_set_time_handler(tcp, 0, NULL);
			}
			if (tcp->on_snd_una_update &&
			    old_offs != new_offs)
				tcp->on_snd_una_update(tcp, new_offs);
		}

		if (tcp_seq_cmp(tcp->snd_una, tcp_ack_num) <= 0) {
			if (tcp_seq_cmp(tcp->snd_wl1, tcp_seq_num) < 0 ||
			    (tcp->snd_wl1 == tcp_seq_num &&
			     tcp_seq_cmp(tcp->snd_wl2, tcp_seq_num) <= 0)) {
				tcp->snd_wnd = tcp_win_size;
				tcp->snd_wl1 = tcp_seq_num;
				tcp->snd_wl2 = tcp_ack_num;
			}
		}

		if (tcp->state == TCP_FIN_WAIT_1) {
			if (tcp->snd_una == tcp->snd_nxt)
				tcp_stream_set_state(tcp, TCP_FIN_WAIT_2);
		}

		if (tcp->state == TCP_CLOSING) {
			if (tcp->snd_una == tcp->snd_nxt)
				tcp_stream_set_state(tcp, TCP_CLOSED);
		}
		return TCP_PACKET_OK;

	case TCP_LAST_ACK:
		if (tcp_ack_num == tcp->snd_nxt)
			tcp_stream_set_state(tcp, TCP_CLOSED);
		return TCP_PACKET_OK;

	default:
		return TCP_PACKET_DROP;
	}
}

static int tcp_rx_user_data(struct tcp_stream *tcp, u32 tcp_seq_num,
			    char *buf, int len)
{
	int tmp_len;
	u32 buf_offs, old_offs, new_offs;
	u8 action;

	if (!len)
		return TCP_PACKET_OK;

	switch (tcp->state) {
	case TCP_ESTABLISHED:
	case TCP_FIN_WAIT_1:
	case TCP_FIN_WAIT_2:
		break;
	default:
		return TCP_PACKET_DROP;
	}

	tmp_len = len;
	old_offs = tcp_stream_rx_offs(tcp);
	buf_offs = tcp_seq_num - tcp->irs - 1;
	if (tcp->rx) {
		tmp_len = tcp->rx(tcp, buf_offs, buf, len);
		if (tmp_len < 0) {
			puts("\nTCP: receive failure\n");
			tcp_send_packet(tcp, TCP_RST, tcp->snd_una,
					tcp->rcv_nxt, 0);
			tcp_stream_set_status(tcp, TCP_ERR_IO);
			tcp_stream_set_state(tcp, TCP_CLOSED);
			tcp_stream_destroy(tcp);
			return TCP_PACKET_DROP;
		}
	}
	if (tmp_len)
		tcp_hole(tcp, tcp_seq_num, tmp_len);

	new_offs = tcp_stream_rx_offs(tcp);
	if (tcp->on_rcv_nxt_update && old_offs != new_offs)
		tcp->on_rcv_nxt_update(tcp, new_offs);

	action = tcp_stream_fin_needed(tcp, tcp->snd_una) | TCP_ACK;
	tcp_send_packet(tcp, action, tcp->snd_una, tcp->rcv_nxt, 0);

	return TCP_PACKET_OK;
}

void tcp_rx_state_machine(struct tcp_stream *tcp,
			  union tcp_build_pkt *b, unsigned int pkt_len)
{
	int tcp_len = pkt_len - IP_HDR_SIZE;
	u32 tcp_seq_num, tcp_ack_num, tcp_win_size;
	int tcp_hdr_len, payload_len;
	u8  tcp_flags, action;

	tcp_hdr_len = GET_TCP_HDR_LEN_IN_BYTES(b->ip.hdr.tcp_hlen);
	payload_len = tcp_len - tcp_hdr_len;

	if (tcp_hdr_len > TCP_HDR_SIZE)
		tcp_parse_options(tcp, (uchar *)b + IP_TCP_HDR_SIZE,
				  tcp_hdr_len - TCP_HDR_SIZE);
	/*
	 * Incoming sequence and ack numbers are server's view of the numbers.
	 * The app must swap the numbers when responding.
	 */
	tcp_seq_num = ntohl(b->ip.hdr.tcp_seq);
	tcp_ack_num = ntohl(b->ip.hdr.tcp_ack);
	tcp_win_size = ntohs(b->ip.hdr.tcp_win) << tcp->rmt_win_scale;

	tcp_flags = b->ip.hdr.tcp_flags;

//	printf("pkt: seq=%d, ack=%d, flags=%x, len=%d\n",
//		tcp_seq_num - tcp->irs, tcp_ack_num - tcp->iss, tcp_flags, pkt_len);
//	printf("tcp: rcv_nxt=%d, snd_una=%d, snd_nxt=%d\n\n",
//		tcp->rcv_nxt - tcp->irs, tcp->snd_una - tcp->iss, tcp->snd_nxt - tcp->iss);

	switch (tcp->state) {
	case TCP_CLOSED:
		if (tcp_flags & TCP_RST)
			return;

		if (tcp_flags & TCP_ACK) {
			tcp_send_packet(tcp, TCP_RST, tcp_ack_num, 0, 0);
			return;
		}

		if (!(tcp_flags & TCP_SYN))
			return;

		tcp->irs = tcp_seq_num;
		tcp->rcv_nxt = tcp->irs + 1;

		tcp->iss = tcp_get_start_seq();
		tcp->snd_una = tcp->iss;
		tcp->snd_nxt = tcp->iss + 1;
		tcp->snd_wnd = tcp_win_size;

		tcp_stream_restart_rx_timer(tcp);

		tcp_stream_set_state(tcp, TCP_SYN_RECEIVED);
		tcp_send_packet_with_retry(tcp, TCP_SYN | TCP_ACK,
					   tcp->iss, 0, 0);
		return;

	case TCP_SYN_SENT:
		if (!(tcp_flags & TCP_ACK))
			return;

		if (tcp_seq_cmp(tcp_ack_num, tcp->iss) <= 0 ||
		    tcp_seq_cmp(tcp_ack_num, tcp->snd_nxt) > 0) {
			if (!(tcp_flags & TCP_RST))
				tcp_send_packet(tcp, TCP_RST, tcp_ack_num, 0, 0);
			return;
		}

		if (tcp_flags & TCP_RST) {
			tcp_stream_set_status(tcp, TCP_ERR_RST);
			tcp_stream_set_state(tcp, TCP_CLOSED);
			return;
		}

		if (!(tcp_flags & TCP_SYN))
			return;

		/* stop retransmit of SYN */
		tcp_stream_set_time_handler(tcp, 0, NULL);

		tcp->irs = tcp_seq_num;
		tcp->rcv_nxt = tcp->irs + 1;
		tcp->snd_una = tcp_ack_num;

		tcp_stream_restart_rx_timer(tcp);

		/* our SYN has been ACKed */
		tcp_stream_set_state(tcp, TCP_ESTABLISHED);

		if (tcp->on_established)
			tcp->on_established(tcp);

		action = tcp_stream_fin_needed(tcp, tcp->snd_una) | TCP_ACK;
		tcp_send_packet(tcp, action, tcp->snd_una, tcp->rcv_nxt, 0);
		tcp_rx_user_data(tcp, tcp_seq_num,
				 ((char *)b) + pkt_len - payload_len,
				 payload_len);
		return;

	case TCP_SYN_RECEIVED:
	case TCP_ESTABLISHED:
	case TCP_FIN_WAIT_1:
	case TCP_FIN_WAIT_2:
	case TCP_CLOSE_WAIT:
	case TCP_CLOSING:
	case TCP_LAST_ACK:
		if (!tcp_seg_in_wnd(tcp, tcp_seq_num, payload_len)) {
			if (tcp_flags & TCP_RST)
				return;
			action = tcp_stream_fin_needed(tcp, tcp->snd_una) | TCP_ACK;
			tcp_send_packet(tcp, action, tcp->snd_una, tcp->rcv_nxt, 0);
			return;
		}

		tcp_stream_restart_rx_timer(tcp);

		if (tcp_flags & TCP_RST) {
			tcp_stream_set_status(tcp, TCP_ERR_RST);
			tcp_stream_set_state(tcp, TCP_CLOSED);
			return;
		}

		if (tcp_flags & TCP_SYN) {
			tcp_send_packet(tcp, TCP_RST, tcp_ack_num, 0, 0);
			tcp_stream_set_status(tcp, TCP_ERR_RST);
			tcp_stream_set_state(tcp, TCP_CLOSED);
			return;
		}

		if (!(tcp_flags & TCP_ACK))
			return;

		if (tcp_rx_check_ack_num(tcp, tcp_seq_num, tcp_ack_num,
					 tcp_win_size) == TCP_PACKET_DROP) {
			return;
		}

		if (tcp_rx_user_data(tcp, tcp_seq_num,
				     ((char *)b) + pkt_len - payload_len,
				     payload_len) == TCP_PACKET_DROP) {
			return;
		}

		if (tcp_flags & TCP_FIN) {
			tcp->fin_rx = 1;
			tcp->fin_rx_seq = tcp_seq_num + payload_len + 1;
			tcp_hole(tcp, tcp_seq_num + payload_len, 1);
			action = tcp_stream_fin_needed(tcp, tcp->snd_una) | TCP_ACK;
			tcp_send_packet(tcp, action, tcp->snd_una, tcp->rcv_nxt, 0);
		}

		if (tcp->fin_rx &&
		    tcp->fin_rx_seq == tcp->rcv_nxt) {
			/* all rx data were processed */
			switch (tcp->state) {
			case TCP_ESTABLISHED:
				tcp_stream_set_state(tcp, TCP_LAST_ACK);
				tcp_send_packet_with_retry(tcp, TCP_ACK | TCP_FIN,
							   tcp->snd_nxt, 0, 0);
				tcp->snd_nxt++;
				break;

			case TCP_FIN_WAIT_1:
				if (tcp_ack_num == tcp->snd_nxt)
					tcp_stream_set_state(tcp, TCP_CLOSED);
				else
					tcp_stream_set_state(tcp, TCP_CLOSING);
				break;

			case TCP_FIN_WAIT_2:
				tcp_stream_set_state(tcp, TCP_CLOSED);
				break;

			default:
				break;
			}
		}

		if (tcp->state == TCP_FIN_WAIT_1 &&
		    tcp_stream_fin_needed(tcp, tcp->snd_una)) {
			/* all tx data were acknowledged */
			tcp_send_packet_with_retry(tcp, TCP_ACK | TCP_FIN,
						   tcp->snd_una, 0, 0);
		}
	}
}

/**
 * rxhand_tcp_f() - process receiving data and call data handler.
 * @b: the packet
 * @pkt_len: the length of packet.
 */
void rxhand_tcp_f(union tcp_build_pkt *b, unsigned int pkt_len)
{
	int tcp_len = pkt_len - IP_HDR_SIZE;
	u16 tcp_rx_xsum = b->ip.hdr.ip_sum;
	struct tcp_stream *tcp;
	struct in_addr src;

	/* Verify IP header */
	debug_cond(DEBUG_DEV_PKT,
		   "TCP RX in RX Sum (to=%pI4, from=%pI4, len=%d)\n",
		   &b->ip.hdr.ip_src, &b->ip.hdr.ip_dst, pkt_len);

	/*
	 * src IP address will be destroyed by TCP checksum verification
	 * algorithm (see tcp_set_pseudo_header()), so remember it before
	 * it was garbaged.
	 */
	src.s_addr = b->ip.hdr.ip_src.s_addr;

	b->ip.hdr.ip_dst = net_ip;
	b->ip.hdr.ip_sum = 0;
	if (tcp_rx_xsum != compute_ip_checksum(b, IP_HDR_SIZE)) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP RX IP xSum Error (%pI4, =%pI4, len=%d)\n",
			   &net_ip, &src, pkt_len);
		return;
	}

	/* Build pseudo header and verify TCP header */
	tcp_rx_xsum = b->ip.hdr.tcp_xsum;
	b->ip.hdr.tcp_xsum = 0;
	if (tcp_rx_xsum != tcp_set_pseudo_header((uchar *)b, b->ip.hdr.ip_src,
						 b->ip.hdr.ip_dst, tcp_len,
						 pkt_len)) {
		debug_cond(DEBUG_DEV_PKT,
			   "TCP RX TCP xSum Error (%pI4, %pI4, len=%d)\n",
			   &net_ip, &src, tcp_len);
		return;
	}

	tcp = tcp_stream_get(b->ip.hdr.tcp_flags & TCP_SYN,
			     src,
			     ntohs(b->ip.hdr.tcp_src),
			     ntohs(b->ip.hdr.tcp_dst));
	if (!tcp)
		return;

	tcp->rx_packets++;
	tcp_rx_state_machine(tcp, b, pkt_len);
	tcp_stream_put(tcp);
}

struct tcp_stream *tcp_stream_connect(struct in_addr rhost, u16 rport)
{
	struct tcp_stream *tcp;

	tcp = tcp_stream_add(rhost, rport, random_port());
	if (!tcp)
		return NULL;

	tcp->iss = tcp_get_start_seq();
	tcp->snd_una = tcp->iss;
	tcp->snd_nxt = tcp->iss + 1;

	tcp_stream_set_state(tcp, TCP_SYN_SENT);
	tcp_send_packet_with_retry(tcp, TCP_SYN, tcp->snd_una, 0, 0);

	return tcp;
}

void tcp_stream_reset(struct tcp_stream *tcp)
{
	if (tcp->state == TCP_CLOSED)
		return;

	tcp_stream_set_time_handler(tcp, 0, NULL);
	tcp_send_packet(tcp, TCP_RST, tcp->snd_una, 0, 0);
	tcp_stream_set_status(tcp, TCP_ERR_RST);
	tcp_stream_set_state(tcp, TCP_CLOSED);
}

void tcp_stream_close(struct tcp_stream *tcp)
{
	switch (tcp->state) {
	case TCP_SYN_SENT:
		tcp_stream_reset(tcp);
		break;
	case TCP_SYN_RECEIVED:
	case TCP_ESTABLISHED:
		tcp->fin_tx = 1;
		tcp->fin_tx_seq = tcp->snd_nxt;
		if (tcp_stream_fin_needed(tcp, tcp->snd_una)) {
			/* all tx data were acknowledged */
			tcp_send_packet_with_retry(tcp, TCP_ACK | TCP_FIN,
						   tcp->snd_una, 0, 0);
		}
		tcp_stream_set_state(tcp, TCP_FIN_WAIT_1);
		tcp->snd_nxt++;
		break;
	default:
		break;
	}
}
