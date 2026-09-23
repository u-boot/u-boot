// SPDX-License-Identifier: GPL-2.0
/*
 * Regression tests for the NFS reply-length checks.
 */

#include <net.h>
#include <string.h>
#include <test/ut.h>
#include <dm/test.h>
#include "../../net/nfs-common.h"

static int dm_test_nfs_read_oob(struct unit_test_state *uts)
{
	int saved_state = nfs_state;
	unsigned long saved_id = rpc_id;
	int saved_offset = nfs_offset;
	enum nfs_version saved_version = choosen_nfs_version;
	u32 saved_size = net_boot_file_size;
	struct rpc_t reply;

	/* Pretend a READ request is outstanding (NFSv3). */
	choosen_nfs_version = NFS_V3;
	nfs_state = STATE_READ_REQ;
	nfs_offset = 0;
	rpc_id = 0x11223344;
	net_boot_file_size = 0;

	/* Accepted reply, matching xid, READ status OK, no attributes, then a
	 * count and data length with the top bit set.
	 */
	memset(&reply, 0, sizeof(reply));
	reply.u.reply.id = htonl((u32)rpc_id);
	reply.u.reply.data[0] = 0;			/* nfsstat3: OK */
	reply.u.reply.data[1] = 0;			/* attributes_follow: no */
	reply.u.reply.data[2] = htonl(0x80000000);	/* count */
	reply.u.reply.data[4] = htonl(0x80000000);	/* data length */

	nfs_pkt_recv((uchar *)&reply.u.reply, sizeof(reply.u.reply));

	/* Rejected: nothing stored. */
	ut_asserteq(0, net_boot_file_size);

	nfs_state = saved_state;
	rpc_id = saved_id;
	nfs_offset = saved_offset;
	choosen_nfs_version = saved_version;
	net_boot_file_size = saved_size;

	return 0;
}
DM_TEST(dm_test_nfs_read_oob, 0);

static int dm_test_nfs_readlink_oob(struct unit_test_state *uts)
{
	int saved_state = nfs_state;
	unsigned long saved_id = rpc_id;
	enum nfs_version saved_version = choosen_nfs_version;
	char *saved_path = nfs_path;
	struct rpc_t reply;

	/* Pretend a READLINK request is outstanding (NFSv3). */
	choosen_nfs_version = NFS_V3;
	nfs_state = STATE_READLINK_REQ;
	rpc_id = 0x11223344;
	nfs_path = nfs_path_buff;
	strcpy(nfs_path_buff, "dir");

	/* Accepted reply, matching xid, READLINK status OK, no attributes, a
	 * length of -1 that slips past the destination bound as pathlen - 1,
	 * then a relative (non-'/') target.
	 */
	memset(&reply, 0, sizeof(reply));
	reply.u.reply.id = htonl((u32)rpc_id);
	reply.u.reply.data[0] = 0;			/* nfsstat3: OK */
	reply.u.reply.data[1] = 0;			/* attributes_follow: no */
	reply.u.reply.data[2] = htonl(0xffffffff);	/* symlink length -1 */
	reply.u.reply.data[3] = htonl(0x61616161);	/* target, not '/' */

	nfs_pkt_recv((uchar *)&reply.u.reply, sizeof(reply.u.reply));

	/* Rejected: the path buffer is untouched. */
	ut_asserteq_str("dir", nfs_path_buff);

	nfs_state = saved_state;
	rpc_id = saved_id;
	choosen_nfs_version = saved_version;
	nfs_path = saved_path;

	return 0;
}
DM_TEST(dm_test_nfs_readlink_oob, 0);
