/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 */

#ifndef __NFS_COMMON_H__
#define __NFS_COMMON_H__

#define SUNRPC_PORT     111

#define PROG_PORTMAP    100000
#define PROG_NFS        100003
#define PROG_MOUNT      100005

#define MSG_CALL        0
#define MSG_REPLY       1

#define HASHES_PER_LINE 65	/* Number of "loading" hashes per line	*/
#define NFS_RETRY_COUNT 30

#define NFS_RPC_ERR	1
#define NFS_RPC_DROP	124

#define NFSERR_PERM     1
#define NFSERR_NOENT    2
#define NFSERR_ACCES    13
#define NFSERR_ISDIR    21
#define NFSERR_INVAL    22

/* Values for Accept State flag on RPC answers (See: rfc1831) */
enum rpc_accept_stat {
	NFS_RPC_SUCCESS = 0,	/* RPC executed successfully */
	NFS_RPC_PROG_UNAVAIL = 1,	/* remote hasn't exported program */
	NFS_RPC_PROG_MISMATCH = 2,	/* remote can't support version # */
	NFS_RPC_PROC_UNAVAIL = 3,	/* program can't support procedure */
	NFS_RPC_GARBAGE_ARGS = 4,	/* procedure can't decode params */
	NFS_RPC_SYSTEM_ERR = 5	/* errors like memory allocation failure */
};

enum nfs_version {
	NFS_UNKOWN = 0,
	NFS_V1 = 1,
	NFS_V2 = 2,
	NFS_V3 = 3,
};

extern enum net_loop_state nfs_download_state;
extern char *nfs_filename;
extern char *nfs_path;
extern char nfs_path_buff[2048];
extern struct in_addr nfs_server_ip;
extern int nfs_server_mount_port;
extern int nfs_server_port;
extern int nfs_our_port;
extern int nfs_timeout_count;
extern unsigned long rpc_id;
extern int nfs_offset;
extern int nfs_len;

extern const ulong nfs_timeout;

extern int nfs_state;
#define STATE_PRCLOOKUP_PROG_MOUNT_REQ	1
#define STATE_PRCLOOKUP_PROG_NFS_REQ	2
#define STATE_MOUNT_REQ			3
#define STATE_UMOUNT_REQ		4
#define STATE_LOOKUP_REQ		5
#define STATE_READ_REQ			6
#define STATE_READLINK_REQ		7

/*
 * Block size used for NFS read accesses.  A RPC reply packet (including  all
 * headers) must fit within a single Ethernet frame to avoid fragmentation.
 * However, if CONFIG_IP_DEFRAG is set, a bigger value could be used.  In any
 * case, most NFS servers are optimized for a power of 2.
 */
#define NFS_READ_SIZE	1024	/* biggest power of two that fits Ether frame */
#define NFS_MAX_ATTRS	26

struct rpc_t {
	union {
		u8 data[NFS_READ_SIZE + (6 + NFS_MAX_ATTRS) *
			sizeof(uint32_t)];
		struct {
			u32 id;
			u32 type;
			u32 rpcvers;
			u32 prog;
			u32 vers;
			u32 proc;
			u32 data[1];
		} call;
		struct {
			u32 id;
			u32 type;
			u32 rstatus;
			u32 verifier;
			u32 v2;
			u32 astatus;
			u32 data[NFS_READ_SIZE / sizeof(u32) +
				NFS_MAX_ATTRS];
		} reply;
	} u;
};

char *nfs_basename(char *path);
char *nfs_dirname(char *path);
void rpc_req_common(int rpc_prog, int rpc_proc, uint32_t *data, int datalen,
		    uchar *txbuff, int *pktlen, int *sport);
void nfs_send(void);
void nfs_pkt_recv(uchar *pkt, unsigned int len);

extern enum nfs_version choosen_nfs_version;

/*
 * Implementation specific functions called from common code
 */
void rpc_req(int rpc_prog, int rpc_proc, uint32_t *data, int datalen);
void nfs_refresh_timeout(void);

/**********************************************************************/

#endif /* __NFS_COMMON_H__ */
