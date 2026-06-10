// SPDX-License-Identifier: GPL-2.0
/*
 * Regression test for IP fragment reassembly.
 *
 * The test drives the real RX path via net_process_received_packet(). Final IP
 * fragment (MF=0) is duplicated, crafted payload triggers redelivery of the datagram,
 * which fails the test for the unfixed code.
 */

#include <net.h>
#include <string.h>
#include <test/ut.h>
#include <dm/test.h>

#define FRAG_LEN (8)
#define PAYLOAD_OFFSET (ETHER_HDR_SIZE + IP_HDR_SIZE)
#define FRAME_LEN (PAYLOAD_OFFSET + FRAG_LEN)

static int udp_rx_count;

static void defrag_udp_handler(uchar *pkt, unsigned int dport,
			       struct in_addr sip, unsigned int sport,
			       unsigned int len)
{
	udp_rx_count++;
}

static int build_frag(uchar *buf, u16 off_flags, const u16 *payload)
{
	struct ethernet_hdr *et = (struct ethernet_hdr *)buf;
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)(buf + ETHER_HDR_SIZE);

	memset(buf, 0, FRAME_LEN);
	et->et_protlen = htons(PROT_IP);

	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_HDR_SIZE + FRAG_LEN);
	ip->ip_id = htons(0x4321);
	ip->ip_off = htons(off_flags);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	/* Broadcast destination is accepted regardless of net_ip. */
	ip->ip_dst.s_addr = 0xffffffff;
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);

	memcpy(buf + PAYLOAD_OFFSET, payload, FRAG_LEN);

	return FRAME_LEN;
}

static int dm_test_net_ip_defrag_dup_last(struct unit_test_state *uts)
{
	rxhand_f *saved_handler = net_get_udp_handler();
	uchar frame[FRAME_LEN];
	/* UDP header, carried by first fragment. */
	u16 udp_hdr[4] = { htons(5000), htons(5001),
			   htons(UDP_HDR_SIZE + FRAG_LEN), 0 };
	/*
	 * Second fragment's payload doubles as a fake hole
	 * {last_byte >= FRAG_LEN, next_hole = 0, prev_hole = 0}, so that the
	 * buggy code re-reading it on a duplicate re-delivers the datagram.
	 */
	u16 frag_b[4] = { 2 * FRAG_LEN, 0, 0, 0 };

	udp_rx_count = 0;
	net_set_udp_handler(defrag_udp_handler);

	/* UDP header, offset 0, MF=1; then data, offset 1, MF=0 */
	net_process_received_packet(frame, build_frag(frame, IP_FLAGS_MFRAG, udp_hdr));
	net_process_received_packet(frame, build_frag(frame, 1, frag_b));
	ut_asserteq(1, udp_rx_count);

	/* Duplicate the final fragment: UDP datagram must not be delivered again. */
	net_process_received_packet(frame, build_frag(frame, 1, frag_b));
	ut_asserteq(1, udp_rx_count);

	net_set_udp_handler(saved_handler);

	return 0;
}

DM_TEST(dm_test_net_ip_defrag_dup_last, 0);
