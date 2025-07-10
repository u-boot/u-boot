/* SPDX-License-Identifier: GPL-2.0 */
/*
 * TCP Support with SACK for file transfer.
 *
 * Copyright 2017 Duncan Hare, All rights reserved.
 */

#define TCP_ACTIVITY 127		/* Number of packets received   */
					/* before console progress mark */
/**
 * struct ip_tcp_hdr - IP and TCP header
 * @ip_hl_v: header length and version
 * @ip_tos: type of service
 * @ip_len: total length
 * @ip_id: identification
 * @ip_off: fragment offset field
 * @ip_ttl: time to live
 * @ip_p: protocol
 * @ip_sum: checksum
 * @ip_src: Source IP address
 * @ip_dst: Destination IP address
 * @tcp_src: TCP source port
 * @tcp_dst: TCP destination port
 * @tcp_seq: TCP sequence number
 * @tcp_ack: TCP Acknowledgment number
 * @tcp_hlen: 4 bits TCP header Length/4, 4 bits reserved, 2 more bits reserved
 * @tcp_flag: flags of TCP
 * @tcp_win: TCP windows size
 * @tcp_xsum: Checksum
 * @tcp_ugr: Pointer to urgent data
 */
struct ip_tcp_hdr {
	u8		ip_hl_v;
	u8		ip_tos;
	u16		ip_len;
	u16		ip_id;
	u16		ip_off;
	u8		ip_ttl;
	u8		ip_p;
	u16		ip_sum;
	struct in_addr	ip_src;
	struct in_addr	ip_dst;
	u16		tcp_src;
	u16		tcp_dst;
	u32		tcp_seq;
	u32		tcp_ack;
	u8		tcp_hlen;
	u8		tcp_flags;
	u16		tcp_win;
	u16		tcp_xsum;
	u16		tcp_ugr;
} __packed;

#define IP_TCP_HDR_SIZE		(sizeof(struct ip_tcp_hdr))
#define TCP_HDR_SIZE		(IP_TCP_HDR_SIZE  - IP_HDR_SIZE)

#define TCP_DATA	0x00	/* Data Packet - internal use only	*/
#define TCP_FIN		0x01	/* Finish flag				*/
#define TCP_SYN		0x02	/* Synch (start) flag			*/
#define TCP_RST		0x04	/* reset flag				*/
#define TCP_PUSH	0x08	/* Push - Notify app			*/
#define TCP_ACK		0x10	/* Acknowledgment of data received	*/
#define TCP_URG		0x20	/* Urgent				*/
#define TCP_ECE		0x40	/* Congestion control			*/
#define TCP_CWR		0x80	/* Congestion Control			*/

/*
 * TCP header options, Seq, MSS, and SACK
 */

#define TCP_SACK 32			/* Number of packets analyzed   */
					/* on leading edge of stream    */

#define TCP_O_END	0x00		/* End of option list		*/
#define TCP_1_NOP	0x01		/* Single padding NOP		*/
#define TCP_O_NOP	0x01010101	/* NOPs pad to 32 bit boundary	*/
#define TCP_O_MSS	0x02		/* MSS Size option		*/
#define TCP_O_SCL	0x03		/* Window Scale option		*/
#define TCP_P_SACK	0x04		/* SACK permitted		*/
#define TCP_V_SACK	0x05		/* SACK values			*/
#define TCP_O_TS	0x08		/* Timestamp option		*/
#define TCP_OPT_LEN_2	0x02
#define TCP_OPT_LEN_3	0x03
#define TCP_OPT_LEN_4	0x04
#define TCP_OPT_LEN_6	0x06
#define TCP_OPT_LEN_8	0x08
#define TCP_OPT_LEN_A	0x0a		/* Timestamp Length		*/
#define TCP_MSS		1460		/* Max segment size		*/
#define TCP_SCALE	0x01		/* Scale			*/

/**
 * struct tcp_mss - TCP option structure for MSS (Max segment size)
 * @kind: Field ID
 * @len: Field length
 * @mss: Segment size value
 */
struct tcp_mss {
	u8	kind;
	u8	len;
	u16	mss;
} __packed;

/**
 * struct tcp_scale - TCP option structure for Windows scale
 * @kind: Field ID
 * @len: Field length
 * @scale: windows shift value used for networks with many hops.
 *         Typically 4 or more hops
 */
struct tcp_scale {
	u8	kind;
	u8	len;
	u8	scale;
} __packed;

/**
 * struct tcp_sack_p - TCP option structure for SACK permitted
 * @kind: Field ID
 * @len: Field length
 */
struct tcp_sack_p {
	u8	kind;
	u8	len;
} __packed;

/**
 * struct sack_edges - structure for SACK edges
 * @l: Left edge of stream
 * @r: right edge of stream
 */
struct sack_edges {
	u32	l;
	u32	r;
} __packed;

#define TCP_SACK_SIZE (sizeof(struct sack_edges))

/*
 * A TCP stream has holes when packets are missing or disordered.
 * A hill is the inverse of a hole, and is data received.
 * TCP received hills (a sequence of data), and inferrs Holes
 * from the "hills" or packets received.
 */

#define TCP_SACK_HILLS	4

/**
 * struct tcp_sack_v - TCP option structure for SACK
 * @kind: Field ID
 * @len: Field length
 * @hill: L & R window edges
 */
struct tcp_sack_v {
	u8	kind;
	u8	len;
	struct	sack_edges hill[TCP_SACK_HILLS];
} __packed;

/**
 * struct tcp_t_opt - TCP option structure for time stamps
 * @kind: Field ID
 * @len: Field length
 * @t_snd: Sender timestamp
 * @t_rcv: Receiver timestamp
 */
struct tcp_t_opt {
	u8	kind;
	u8	len;
	u32	t_snd;
	u32	t_rcv;
} __packed;

#define TCP_TSOPT_SIZE (sizeof(struct tcp_t_opt))

/*
 * ip tcp  structure with options
 */

/**
 * struct ip_tcp_hdr_o - IP + TCP header + TCP options
 * @hdr: IP + TCP header
 * @mss: TCP MSS Option
 * @scale: TCP Windows Scale Option
 * @sack_p: TCP Sack-Permitted Option
 * @t_opt: TCP Timestamp Option
 * @end: end of options
 */
struct ip_tcp_hdr_o {
	struct	ip_tcp_hdr hdr;
	struct	tcp_mss	   mss;
	struct	tcp_scale  scale;
	struct	tcp_sack_p sack_p;
	struct	tcp_t_opt  t_opt;
	u8	end;
} __packed;

#define IP_TCP_O_SIZE (sizeof(struct ip_tcp_hdr_o))

/**
 * struct ip_tcp_hdr_s - IP + TCP header + TCP options
 * @hdr: IP + TCP header
 * @t_opt: TCP Timestamp Option
 * @sack_v: TCP SACK Option
 * @end: end of options
 */
struct ip_tcp_hdr_s {
	struct	ip_tcp_hdr	hdr;
	struct	tcp_t_opt	t_opt;
	struct	tcp_sack_v	sack_v;
	u8	end;
} __packed;

#define IP_TCP_SACK_SIZE (sizeof(struct ip_tcp_hdr_s))

/*
 * TCP pseudo header definitions
 */
#define PSEUDO_PAD_SIZE	8

/**
 * struct pseudo_hdr - Pseudo Header
 * @padding: pseudo hdr size = ip_tcp hdr size
 * @p_src: Source IP address
 * @p_dst: Destination IP address
 * @rsvd: reserved
 * @p: protocol
 * @len: length of header
 */
struct pseudo_hdr {
	u8 padding[PSEUDO_PAD_SIZE];
	struct in_addr p_src;
	struct in_addr p_dst;
	u8      rsvd;
	u8      p;
	u16     len;
} __packed;

#define PSEUDO_HDR_SIZE	((sizeof(struct pseudo_hdr)) - PSEUDO_PAD_SIZE)

/**
 * union tcp_build_pkt - union for building TCP/IP packet.
 * @ph: pseudo header
 * @ip: IP and TCP header plus TCP options
 * @sack: IP and TCP header plus SACK options
 * @raw: buffer
 *
 * Build Pseudo header in packed buffer
 * first, calculate TCP checksum, then build IP header in packed buffer.
 *
 */
union tcp_build_pkt {
	struct pseudo_hdr ph;
	struct ip_tcp_hdr_o ip;
	struct ip_tcp_hdr_s sack;
	uchar  raw[1600];
} __packed;

/**
 * enum tcp_state - TCP State machine states for connection
 * @TCP_CLOSED: Need to send SYN to connect
 * @TCP_SYN_SENT: Trying to connect, waiting for SYN ACK
 * @TCP_SYN_RECEIVED: Initial SYN received, waiting for ACK
 * @TCP_ESTABLISHED: both server & client have a connection
 * @TCP_CLOSE_WAIT: Rec FIN, passed to app for FIN, ACK rsp
 * @TCP_CLOSING: Rec FIN, sent FIN, ACK waiting for ACK
 * @TCP_FIN_WAIT_1: Sent FIN waiting for response
 * @TCP_FIN_WAIT_2: Rec ACK from FIN sent, waiting for FIN
 * @TCP_LAST_ACK: Waiting for ACK of the connection termination
 */
enum tcp_state {
	TCP_CLOSED,
	TCP_SYN_SENT,
	TCP_SYN_RECEIVED,
	TCP_ESTABLISHED,
	TCP_CLOSE_WAIT,
	TCP_CLOSING,
	TCP_FIN_WAIT_1,
	TCP_FIN_WAIT_2,
	TCP_LAST_ACK,
};

/**
 * enum tcp_status - TCP stream status for connection
 * @TCP_ERR_OK: no rx/tx errors
 * @TCP_ERR_TOUT: rx/tx timeout happened
 * @TCP_ERR_RST: connection was reset
 * @TCP_ERR_IO: input/output error
 */
enum tcp_status {
	TCP_ERR_OK = 0,
	TCP_ERR_TOUT,
	TCP_ERR_RST,
	TCP_ERR_IO
};

/**
 * struct tcp_stream - TCP data stream structure
 * @rhost:		Remote host, network byte order
 * @rport:		Remote port, host byte order
 * @lport:		Local port, host byte order
 *
 * @priv:		User private data (not used by tcp module)
 *
 * @max_retry_count:	Maximum retransmit attempts (default 3)
 * @initial_timeout:	Timeout from initial TX to reTX (default 2 sec)
 * @rx_inactiv_timeout:	Maximum time from last rx till connection drop
 *			  (default 30 sec)
 *
 * @on_closed:		User callback, called just before destroying TCP stream
 * @on_established:	User callback, called when TCP stream enters
 *			  TCP_ESTABLISHED state
 * @on_rcv_nxt_update:	User callback, called when all data in the segment
 *			  [0..rx_bytes - 1] was received
 * @on_snd_una_update:	User callback, called when all data in the segment
 *			  [0..tx_bytes - 1] were transferred and acknowledged
 * @rx:			User callback, called on receive of segment
 *			  [rx_offs..rx_offs+len-1]. If NULL -- all incoming data
 *			  will be ignored. User SHOULD store the segment and
 *			  return the number of accepted bytes or negative value
 *			  on error.
 *			  WARNING: Previous segmengs may not be received yet
 * @tx:			User callback, called on transmit/retransmit of segment
 *			  [tx_offs..tx_offs+maxlen-1]. If NULL -- no data will
 *			  be transmitted. User SHOULD fill provided buffer and
 *			  return the number of bytes in the buffer or negative
 *			  value on error.
 *			  WARNING: do not use tcp_stream_close() from this
 *			    callback (it will break stream). Better use
 *			    on_snd_una_update() callback for such purposes.
 *
 * @time_last_rx:	Arrival time of last valid incoming package (ticks)
 * @time_start:		Timeout start time (ticks)
 * @time_delta:		Timeout duration (ticks)
 * @time_handler	Timeout handler for a stream
 *
 * @state:		TCP connection state
 * @status:		TCP stream status (OK or ERR)
 * @rx_packets:		total number of received packets
 * @tx_packets:		total number of transmitted packets
 *
 * @fin_rx:		Non-zero if TCP_FIN was received
 * @fin_rx_seq:		TCP sequence of rx FIN bit
 * @fin_tx:		Non-zero if TCP_FIN was sent (or planned to send)
 * @fin_tx_seq:		TCP sequence of tx FIN bit
 *
 * @iss:		Initial send sequence number
 * @snd_una:		Send unacknowledged
 * @snd_nxt:		Send next
 * @snd_wnd:		Send window (in bytes)
 * @snd_wl1:		Segment sequence number used for last window update
 * @snd_wl2:		Segment acknowledgment number used for last window update
 *
 * @irs:		Initial receive sequence number
 * @rcv_nxt:		Receive next
 * @rcv_wnd:		Receive window (in bytes)
 *
 * @loc_timestamp:	Local timestamp
 * @rmt_timestamp:	Remote timestamp
 *
 * @rmt_win_scale:	Remote window scale factor
 *
 * @lost:		Used for SACK
 *
 * @retry_cnt:		Number of retry attempts remaining. Only SYN, FIN
 *			  or DATA segments are tried to retransmit.
 * @retry_timeout:	Current retry timeout (ms)
 * @retry_action:	TCP flags used for sending
 * @retry_seq_num:	TCP sequence for retransmit
 * retry_tx_len:	Number of data to transmit
 * @retry_tx_offs:	Position in the TX stream
 */
struct tcp_stream {
	struct in_addr	rhost;
	u16		rport;
	u16		lport;

	void		*priv;

	int		max_retry_count;
	int		initial_timeout;
	int		rx_inactiv_timeout;

	void		(*on_closed)(struct tcp_stream *tcp);
	void		(*on_established)(struct tcp_stream *tcp);
	void		(*on_rcv_nxt_update)(struct tcp_stream *tcp, u32 rx_bytes);
	void		(*on_snd_una_update)(struct tcp_stream *tcp, u32 tx_bytes);
	int		(*rx)(struct tcp_stream *tcp, u32 rx_offs, void *buf, int len);
	int		(*tx)(struct tcp_stream *tcp, u32 tx_offs, void *buf, int maxlen);

	ulong		time_last_rx;
	ulong		time_start;
	ulong		time_delta;
	void		(*time_handler)(struct tcp_stream *tcp);

	enum tcp_state	state;
	enum tcp_status	status;
	u32		rx_packets;
	u32		tx_packets;

	int		fin_rx;
	u32		fin_rx_seq;

	int		fin_tx;
	u32		fin_tx_seq;

	u32		iss;
	u32		snd_una;
	u32		snd_nxt;
	u32		snd_wnd;
	u32		snd_wl1;
	u32		snd_wl2;

	u32		irs;
	u32		rcv_nxt;
	u32		rcv_wnd;

	/* TCP option timestamp */
	u32		loc_timestamp;
	u32		rmt_timestamp;

	/* TCP window scale */
	u8		rmt_win_scale;

	/* TCP sliding window control used to request re-TX */
	struct tcp_sack_v lost;

	/* used for data retransmission */
	int		retry_cnt;
	int		retry_timeout;
	u8		retry_action;
	u32		retry_seq_num;
	u32		retry_tx_len;
	u32		retry_tx_offs;
};

void tcp_init(void);

/*
 * This function sets user callback called on TCP stream creation.
 * Callback should:
 *  + Check TCP stream endpoint and make connection verdict
 *    - return non-zero value to accept connection
 *    - return zero to drop connection
 *  + Setup TCP stream callbacks like: on_closed(), on_established(),
 *    n_rcv_nxt_update(), on_snd_una_update(), rx() and tx().
 *  + Setup other stream related data
 *
 * WARNING: User MUST setup TCP stream on_create handler. Without it
 *          no connection (including outgoung) will be created.
 */
void tcp_stream_set_on_create_handler(int (*on_create)(struct tcp_stream *));

/*
 * tcp_stream_get -- Get or create TCP stream
 * @is_new:	if non-zero and no stream found, then create a new one
 * @rhost:	Remote host, network byte order
 * @rport:	Remote port, host byte order
 * @lport:	Local port, host byte order
 *
 * Returns: TCP stream structure or NULL (if not found/created)
 */
struct tcp_stream *tcp_stream_get(int is_new, struct in_addr rhost,
				  u16 rport, u16 lport);

/*
 * tcp_stream_connect -- Create new TCP stream for remote connection.
 * @rhost:	Remote host, network byte order
 * @rport:	Remote port, host byte order
 *
 * Returns: TCP new stream structure or NULL (if not created).
 *          Random local port will be used.
 */
struct tcp_stream *tcp_stream_connect(struct in_addr rhost, u16 rport);

/*
 * tcp_stream_put -- Return stream to a TCP subsystem. Subsystem will
 *                   check stream and destroy it (if stream was already
 *                   closed). Otherwize no stream change will happen.
 * @tcp:	TCP stream to put
 */
void tcp_stream_put(struct tcp_stream *tcp);

/*
 * tcp_stream_restart_rx_timer -- Restart RX inactivity timer. Usually there
 *                                is no needs to call this function. Timer
 *                                will be restarted on receiving of any valid
 *                                tcp packet belonging to a stream.
 *
 *                                This function may be used to prevent connection
 *                                break in the following case:
 *                                  - u-boot is busy with very long data processing
 *                                  - remote side waits for u-boot reply
 *
 * @tcp:	TCP stream to put
 */
void tcp_stream_restart_rx_timer(struct tcp_stream *tcp);

enum tcp_state  tcp_stream_get_state(struct tcp_stream *tcp);
enum tcp_status tcp_stream_get_status(struct tcp_stream *tcp);

/*
 * tcp_stream_rx_offs(),
 * tcp_stream_tx_offs()  -- Returns offset of first unacknowledged byte
 *                          in receive/transmit stream correspondingly.
 *                          The result is NOT affected by sin/fin flags.
 * @tcp:	TCP stream
 */
u32 tcp_stream_rx_offs(struct tcp_stream *tcp);
u32 tcp_stream_tx_offs(struct tcp_stream *tcp);

/* reset tcp stream */
void tcp_stream_reset(struct tcp_stream *tcp);
/* force TCP stream closing, do NOT use from tcp->tx callback */
void tcp_stream_close(struct tcp_stream *tcp);

void tcp_streams_poll(void);

int tcp_set_tcp_header(struct tcp_stream *tcp, uchar *pkt, int payload_len,
		       u8 action, u32 tcp_seq_num, u32 tcp_ack_num);

void rxhand_tcp_f(union tcp_build_pkt *b, unsigned int len);

u16 tcp_set_pseudo_header(uchar *pkt, struct in_addr src, struct in_addr dest,
			  int tcp_len, int pkt_len);
