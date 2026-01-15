// SPDX-License-Identifier: GPL-2.0+
/*
 * NFS support driver - based on etherboot and U-BOOT's tftp.c
 *
 * Masami Komiya <mkomiya@sonare.it> 2004
 *
 */

/* NOTE: the NFS code is heavily inspired by the NetBSD netboot code (read:
 * large portions are copied verbatim) as distributed in OSKit 0.97.  A few
 * changes were necessary to adapt the code to Etherboot and to fix several
 * inconsistencies.  Also the RPC message preparation is done "by hand" to
 * avoid adding netsprintf() which I find hard to understand and use.
 */

/* NOTE 2: Etherboot does not care about things beyond the kernel image, so
 * it loads the kernel image off the boot server (ARP_SERVER) and does not
 * access the client root disk (root-path in dhcpd.conf), which would use
 * ARP_ROOTSERVER.  The root disk is something the operating system we are
 * about to load needs to use.	This is different from the OSKit 0.97 logic.
 */

/* NOTE 3: Symlink handling introduced by Anselm M Hoffmeister, 2003-July-14
 * If a symlink is encountered, it is followed as far as possible (recursion
 * possible, maximum 16 steps). There is no clearing of ".."'s inside the
 * path, so please DON'T DO THAT. thx.
 */

/* NOTE 4: NFSv3 support added by Guillaume GARDET, 2016-June-20.
 * NFSv2 is still used by default. But if server does not support NFSv2, then
 * NFSv3 is used, if available on NFS server.
 */

/* NOTE 5: NFSv1 support added by Christian Gmeiner, Thomas Rienoessl,
 * September 27, 2018. As of now, NFSv3 is the default choice. If the server
 * does not support NFSv3, we fall back to versions 2 or 1.
 */

#ifdef CONFIG_SYS_DIRECT_FLASH_NFS
#include <flash.h>
#endif
#include <image.h>
#include <log.h>
#include <net.h>
#include <mapmem.h>
#include "nfs.h"
#include "nfs-common.h"

static int fs_mounted;
static char dirfh[NFS3_FHSIZE]; /* NFSv2 / NFSv3 file handle of directory */
static unsigned int dirfh3_length; /* (variable) length of dirfh when NFSv3 */
static char filefh[NFS3_FHSIZE]; /* NFSv2 / NFSv3 file handle */
static unsigned int filefh3_length;	/* (variable) length of filefh when NFSv3 */

enum net_loop_state nfs_download_state;
char *nfs_filename;
char *nfs_path;
char nfs_path_buff[2048];
struct in_addr nfs_server_ip;
int nfs_server_mount_port;
int nfs_server_port;
int nfs_our_port;
int nfs_state;
int nfs_timeout_count;
unsigned long rpc_id;
int nfs_offset = -1;
int nfs_len;

const ulong nfs_timeout = CONFIG_NFS_TIMEOUT;

enum nfs_version choosen_nfs_version = NFS_V3;

static inline int store_block(uchar *src, unsigned int offset, unsigned int len)
{
	ulong newsize = offset + len;
#ifdef CONFIG_SYS_DIRECT_FLASH_NFS
	int i, rc = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		/* start address in flash? */
		if (image_load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { /* Flash is destination for this packet */
		rc = flash_write((uchar *)src, (ulong)image_load_addr + offset,
				 len);
		if (rc) {
			flash_perror(rc);
			return -1;
		}
	} else
#endif /* CONFIG_SYS_DIRECT_FLASH_NFS */
	{
		void *ptr = map_sysmem(image_load_addr + offset, len);

		memcpy(ptr, src, len);
		unmap_sysmem(ptr);
	}

	if (net_boot_file_size < (offset + len))
		net_boot_file_size = newsize;
	return 0;
}

char *nfs_basename(char *path)
{
	char *fname;

	fname = path + strlen(path) - 1;
	while (fname >= path) {
		if (*fname == '/') {
			fname++;
			break;
		}
		fname--;
	}
	return fname;
}

char *nfs_dirname(char *path)
{
	char *fname;

	fname = nfs_basename(path);
	--fname;
	*fname = '\0';
	return path;
}

/**************************************************************************
 * RPC_ADD_CREDENTIALS - Add RPC authentication/verifier entries
 **************************************************************************
 */
static uint32_t *rpc_add_credentials(uint32_t *p)
{
	/* Here's the executive summary on authentication requirements of the
	 * various NFS server implementations:	Linux accepts both AUTH_NONE
	 * and AUTH_UNIX authentication (also accepts an empty hostname field
	 * in the AUTH_UNIX scheme).  *BSD refuses AUTH_NONE, but accepts
	 * AUTH_UNIX (also accepts an empty hostname field in the AUTH_UNIX
	 * scheme).  To be safe, use AUTH_UNIX and pass the hostname if we have
	 * it (if the BOOTP/DHCP reply didn't give one, just use an empty
	 * hostname).
	 */

	/* Provide an AUTH_UNIX credential.  */
	*p++ = htonl(1);		/* AUTH_UNIX */
	*p++ = htonl(20);		/* auth length */
	*p++ = 0;			/* stamp */
	*p++ = 0;			/* hostname string */
	*p++ = 0;			/* uid */
	*p++ = 0;			/* gid */
	*p++ = 0;			/* auxiliary gid list */

	/* Provide an AUTH_NONE verifier.  */
	*p++ = 0;			/* AUTH_NONE */
	*p++ = 0;			/* auth length */

	return p;
}

void rpc_req_common(int rpc_prog, int rpc_proc, uint32_t *data, int datalen,
		    uchar *txbuff, int *pktlen, int *sport)
{
	struct rpc_t rpc_pkt;
	unsigned long id;
	u32 *p;

	id = ++rpc_id;
	rpc_pkt.u.call.id = htonl(id);
	rpc_pkt.u.call.type = htonl(MSG_CALL);
	rpc_pkt.u.call.rpcvers = htonl(2);	/* use RPC version 2 */
	rpc_pkt.u.call.prog = htonl(rpc_prog);
	switch (rpc_prog) {
	case PROG_NFS:
		switch (choosen_nfs_version) {
		case NFS_V1:
		case NFS_V2:
			rpc_pkt.u.call.vers = htonl(2);
			break;

		case NFS_V3:
			rpc_pkt.u.call.vers = htonl(3);
			break;

		case NFS_UNKOWN:
			/* nothing to do */
			break;
		}
		break;
	case PROG_MOUNT:
		switch (choosen_nfs_version) {
		case NFS_V1:
			rpc_pkt.u.call.vers = htonl(1);
			break;

		case NFS_V2:
			rpc_pkt.u.call.vers = htonl(2);
			break;

		case NFS_V3:
			rpc_pkt.u.call.vers = htonl(3);
			break;

		case NFS_UNKOWN:
			/* nothing to do */
			break;
		}
		break;
	case PROG_PORTMAP:
	default:
		rpc_pkt.u.call.vers = htonl(2);	/* portmapper is version 2 */
	}
	rpc_pkt.u.call.proc = htonl(rpc_proc);
	p = rpc_pkt.u.call.data;

	if (datalen)
		memcpy(p, data, datalen * sizeof(uint32_t));

	*pktlen = (char *)p + datalen * sizeof(uint32_t) - (char *)&rpc_pkt;

	memcpy(txbuff, &rpc_pkt.u.data[0], *pktlen);

	if (rpc_prog == PROG_PORTMAP)
		*sport = SUNRPC_PORT;
	else if (rpc_prog == PROG_MOUNT)
		*sport = nfs_server_mount_port;
	else
		*sport = nfs_server_port;
}

/**************************************************************************
 * RPC_LOOKUP - Lookup RPC Port numbers
 **************************************************************************
 */
static void rpc_lookup_req(int prog, int ver)
{
	u32 data[16];

	data[0] = 0; data[1] = 0;	/* auth credential */
	data[2] = 0; data[3] = 0;	/* auth verifier */
	data[4] = htonl(prog);
	data[5] = htonl(ver);
	data[6] = htonl(17);	/* IP_UDP */
	data[7] = 0;
	rpc_req(PROG_PORTMAP, PORTMAP_GETPORT, data, 8);
}

/**************************************************************************
 * NFS_MOUNT - Mount an NFS Filesystem
 **************************************************************************
 */
static void nfs_mount_req(char *path)
{
	u32 data[1024];
	u32 *p;
	int len;
	int pathlen;

	pathlen = strlen(path);

	p = &data[0];
	p = rpc_add_credentials(p);

	*p++ = htonl(pathlen);
	if (pathlen & 3)
		*(p + pathlen / 4) = 0;
	memcpy(p, path, pathlen);
	p += (pathlen + 3) / 4;

	len = (uint32_t *)p - (uint32_t *)&data[0];

	rpc_req(PROG_MOUNT, MOUNT_ADDENTRY, data, len);
}

/**************************************************************************
 * NFS_UMOUNTALL - Unmount all our NFS Filesystems on the Server
 **************************************************************************
 */
static void nfs_umountall_req(void)
{
	u32 data[1024];
	u32 *p;
	int len;

	if ((nfs_server_mount_port == -1) || !fs_mounted) {
		/* Nothing mounted, nothing to umount */
		net_set_state(NETLOOP_FAIL);
		return;
	}

	p = &data[0];
	p = rpc_add_credentials(p);

	len = (uint32_t *)p - (uint32_t *)&data[0];

	rpc_req(PROG_MOUNT, MOUNT_UMOUNTALL, data, len);
}

/***************************************************************************
 * NFS_READLINK (AH 2003-07-14)
 * This procedure is called when read of the first block fails -
 * this probably happens when it's a directory or a symlink
 * In case of successful readlink(), the dirname is manipulated,
 * so that inside the nfs() function a recursion can be done.
 ***************************************************************************
 */
static void nfs_readlink_req(void)
{
	u32 data[1024];
	u32 *p;
	int len;

	p = &data[0];
	p = rpc_add_credentials(p);

	if (choosen_nfs_version != NFS_V3) {
		memcpy(p, filefh, NFS_FHSIZE);
		p += (NFS_FHSIZE / 4);
	} else { /* NFS_V3 */
		*p++ = htonl(filefh3_length);
		memcpy(p, filefh, filefh3_length);
		p += (filefh3_length / 4);
	}

	len = (uint32_t *)p - (uint32_t *)&data[0];

	rpc_req(PROG_NFS, NFS_READLINK, data, len);
}

/**************************************************************************
 * NFS_LOOKUP - Lookup Pathname
 **************************************************************************
 */
static void nfs_lookup_req(char *fname)
{
	u32 data[1024];
	u32 *p;
	int len;
	int fnamelen;

	fnamelen = strlen(fname);

	p = &data[0];
	p = rpc_add_credentials(p);

	if (choosen_nfs_version != NFS_V3) {
		memcpy(p, dirfh, NFS_FHSIZE);
		p += (NFS_FHSIZE / 4);
		*p++ = htonl(fnamelen);
		if (fnamelen & 3)
			*(p + fnamelen / 4) = 0;
		memcpy(p, fname, fnamelen);
		p += (fnamelen + 3) / 4;

		len = (uint32_t *)p - (uint32_t *)&data[0];

		rpc_req(PROG_NFS, NFS_LOOKUP, data, len);
	} else {  /* NFS_V3 */
		*p++ = htonl(dirfh3_length);	/* Dir handle length */
		memcpy(p, dirfh, dirfh3_length);
		p += (dirfh3_length / 4);
		*p++ = htonl(fnamelen);
		if (fnamelen & 3)
			*(p + fnamelen / 4) = 0;
		memcpy(p, fname, fnamelen);
		p += (fnamelen + 3) / 4;

		len = (uint32_t *)p - (uint32_t *)&data[0];

		rpc_req(PROG_NFS, NFS3PROC_LOOKUP, data, len);
	}
}

/**************************************************************************
 * NFS_READ - Read File on NFS Server
 **************************************************************************
 */
static void nfs_read_req(int offset, int readlen)
{
	u32 data[1024];
	u32 *p;
	int len;

	p = &data[0];
	p = rpc_add_credentials(p);

	if (choosen_nfs_version != NFS_V3) {
		memcpy(p, filefh, NFS_FHSIZE);
		p += (NFS_FHSIZE / 4);
		*p++ = htonl(offset);
		*p++ = htonl(readlen);
		*p++ = 0;
	} else { /* NFS_V3 */
		*p++ = htonl(filefh3_length);
		memcpy(p, filefh, filefh3_length);
		p += (filefh3_length / 4);
		*p++ = htonl(0); /* offset is 64-bit long, so fill with 0 */
		*p++ = htonl(offset);
		*p++ = htonl(readlen);
		*p++ = 0;
	}

	len = (uint32_t *)p - (uint32_t *)&data[0];

	rpc_req(PROG_NFS, NFS_READ, data, len);
}

/**************************************************************************
 * RPC request dispatcher
 **************************************************************************
 */
void nfs_send(void)
{
	switch (nfs_state) {
	case STATE_PRCLOOKUP_PROG_MOUNT_REQ:
		if (choosen_nfs_version != NFS_V3)
			rpc_lookup_req(PROG_MOUNT, 1);
		else  /* NFS_V3 */
			rpc_lookup_req(PROG_MOUNT, 3);
		break;
	case STATE_PRCLOOKUP_PROG_NFS_REQ:
		if (choosen_nfs_version != NFS_V3)
			rpc_lookup_req(PROG_NFS, 2);
		else  /* NFS_V3 */
			rpc_lookup_req(PROG_NFS, 3);
		break;
	case STATE_MOUNT_REQ:
		nfs_mount_req(nfs_path);
		break;
	case STATE_UMOUNT_REQ:
		nfs_umountall_req();
		break;
	case STATE_LOOKUP_REQ:
		nfs_lookup_req(nfs_filename);
		break;
	case STATE_READ_REQ:
		nfs_read_req(nfs_offset, nfs_len);
		break;
	case STATE_READLINK_REQ:
		nfs_readlink_req();
		break;
	}
}

/**************************************************************************
 * Handlers for the reply from server
 **************************************************************************
 */

static int rpc_handle_error(struct rpc_t *rpc_pkt)
{
	if (rpc_pkt->u.reply.rstatus  ||
	    rpc_pkt->u.reply.verifier ||
	    rpc_pkt->u.reply.astatus  ||
	    rpc_pkt->u.reply.data[0]) {
		switch (ntohl(rpc_pkt->u.reply.astatus)) {
		case NFS_RPC_SUCCESS: /* Not an error */
			break;
		case NFS_RPC_PROG_MISMATCH: {
			/* Remote can't support NFS version */
			const int min = ntohl(rpc_pkt->u.reply.data[0]);
			const int max = ntohl(rpc_pkt->u.reply.data[1]);

			if (max < NFS_V1 || max > NFS_V3 || min > NFS_V3) {
				puts("*** ERROR: NFS version not supported");
				debug(": Requested: V%d, accepted: min V%d - max V%d\n",
				      choosen_nfs_version,
				      ntohl(rpc_pkt->u.reply.data[0]),
				      ntohl(rpc_pkt->u.reply.data[1]));
				puts("\n");
				choosen_nfs_version = NFS_UNKOWN;
				break;
			}

			debug("*** Warning: NFS version not supported: Requested: V%d, accepted: min V%d - max V%d\n",
			      choosen_nfs_version,
			      ntohl(rpc_pkt->u.reply.data[0]),
			      ntohl(rpc_pkt->u.reply.data[1]));
			debug("Will retry with NFSv%d\n", min);
			choosen_nfs_version = min;
			return -NFS_RPC_PROG_MISMATCH;
		}
		case NFS_RPC_PROG_UNAVAIL:
		case NFS_RPC_PROC_UNAVAIL:
		case NFS_RPC_GARBAGE_ARGS:
		case NFS_RPC_SYSTEM_ERR:
		default: /* Unknown error on 'accept state' flag */
			debug("*** ERROR: accept state error (%d)\n",
			      ntohl(rpc_pkt->u.reply.astatus));
			break;
		}
		return -1;
	}

	return 0;
}

static int rpc_lookup_reply(int prog, uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;

	memcpy(&rpc_pkt.u.data[0], pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus)
		return -1;

	switch (prog) {
	case PROG_MOUNT:
		nfs_server_mount_port = ntohl(rpc_pkt.u.reply.data[0]);
		break;
	case PROG_NFS:
		nfs_server_port = ntohl(rpc_pkt.u.reply.data[0]);
		break;
	}

	return 0;
}

static int nfs_mount_reply(uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;
	int ret;

	memcpy(&rpc_pkt.u.data[0], pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	ret = rpc_handle_error(&rpc_pkt);
	if (ret)
		return ret;

	fs_mounted = 1;
	/*  NFSv2 and NFSv3 use same structure */
	if (choosen_nfs_version != NFS_V3) {
		memcpy(dirfh, rpc_pkt.u.reply.data + 1, NFS_FHSIZE);
	} else {
		dirfh3_length = ntohl(rpc_pkt.u.reply.data[1]);
		if (dirfh3_length > NFS3_FHSIZE)
			dirfh3_length  = NFS3_FHSIZE;
		memcpy(dirfh, rpc_pkt.u.reply.data + 2, dirfh3_length);
	}

	return 0;
}

static int nfs_umountall_reply(uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;

	memcpy(&rpc_pkt.u.data[0], pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus)
		return -1;

	fs_mounted = 0;
	memset(dirfh, 0, sizeof(dirfh));

	return 0;
}

static int nfs_lookup_reply(uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;
	int ret;

	memcpy(&rpc_pkt.u.data[0], pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	ret = rpc_handle_error(&rpc_pkt);
	if (ret)
		return ret;

	if (choosen_nfs_version != NFS_V3) {
		if (((uchar *)&rpc_pkt.u.reply.data[0] - (uchar *)&rpc_pkt + NFS_FHSIZE) > len)
			return -NFS_RPC_DROP;
		memcpy(filefh, rpc_pkt.u.reply.data + 1, NFS_FHSIZE);
	} else {  /* NFS_V3 */
		filefh3_length = ntohl(rpc_pkt.u.reply.data[1]);
		if (filefh3_length > NFS3_FHSIZE)
			filefh3_length  = NFS3_FHSIZE;
		memcpy(filefh, rpc_pkt.u.reply.data + 2, filefh3_length);
	}

	return 0;
}

static int nfs3_get_attributes_offset(uint32_t *data)
{
	if (data[1]) {
		/* 'attributes_follow' flag is TRUE,
		 * so we have attributes on 21 dwords
		 */
		/* Skip unused values :
		 *	type;	32 bits value,
		 *	mode;	32 bits value,
		 *	nlink;	32 bits value,
		 *	uid;	32 bits value,
		 *	gid;	32 bits value,
		 *	size;	64 bits value,
		 *	used;	64 bits value,
		 *	rdev;	64 bits value,
		 *	fsid;	64 bits value,
		 *	fileid;	64 bits value,
		 *	atime;	64 bits value,
		 *	mtime;	64 bits value,
		 *	ctime;	64 bits value,
		 */
		return 22;
	}

	/* 'attributes_follow' flag is FALSE,
	 * so we don't have any attributes
	 */
	return 1;
}

static int nfs_readlink_reply(uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;
	int rlen;
	int nfsv3_data_offset = 0;

	memcpy((unsigned char *)&rpc_pkt, pkt, len);

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus  ||
	    rpc_pkt.u.reply.data[0])
		return -1;

	if (choosen_nfs_version == NFS_V3) {
		nfsv3_data_offset =
			nfs3_get_attributes_offset(rpc_pkt.u.reply.data);
	}

	/* new path length */
	rlen = ntohl(rpc_pkt.u.reply.data[1 + nfsv3_data_offset]);

	if (((uchar *)&rpc_pkt.u.reply.data[0] - (uchar *)&rpc_pkt + rlen) > len)
		return -NFS_RPC_DROP;

	if (*((char *)&rpc_pkt.u.reply.data[2 + nfsv3_data_offset]) != '/') {
		int pathlen;

		strcat(nfs_path, "/");
		pathlen = strlen(nfs_path);
		memcpy(nfs_path + pathlen,
		       (uchar *)&rpc_pkt.u.reply.data[2 + nfsv3_data_offset],
		       rlen);
		nfs_path[pathlen + rlen] = 0;
	} else {
		memcpy(nfs_path,
		       (uchar *)&rpc_pkt.u.reply.data[2 + nfsv3_data_offset],
		       rlen);
		nfs_path[rlen] = 0;
	}
	return 0;
}

static int nfs_read_reply(uchar *pkt, unsigned int len)
{
	struct rpc_t rpc_pkt;
	int rlen;
	uchar *data_ptr;

	memcpy(&rpc_pkt.u.data[0], pkt, sizeof(rpc_pkt.u.reply));

	if (ntohl(rpc_pkt.u.reply.id) > rpc_id)
		return -NFS_RPC_ERR;
	else if (ntohl(rpc_pkt.u.reply.id) < rpc_id)
		return -NFS_RPC_DROP;

	if (rpc_pkt.u.reply.rstatus  ||
	    rpc_pkt.u.reply.verifier ||
	    rpc_pkt.u.reply.astatus  ||
	    rpc_pkt.u.reply.data[0]) {
		if (rpc_pkt.u.reply.rstatus)
			return -9999;
		if (rpc_pkt.u.reply.astatus)
			return -9999;
		return -ntohl(rpc_pkt.u.reply.data[0]);
	}

	if (nfs_offset != 0 && !((nfs_offset) %
			(NFS_READ_SIZE / 2 * 10 * HASHES_PER_LINE)))
		puts("\n\t ");
	if (!(nfs_offset % ((NFS_READ_SIZE / 2) * 10)))
		putc('#');

	if (choosen_nfs_version != NFS_V3) {
		rlen = ntohl(rpc_pkt.u.reply.data[18]);
		data_ptr = (uchar *)&rpc_pkt.u.reply.data[19];
	} else {  /* NFS_V3 */
		int nfsv3_data_offset =
			nfs3_get_attributes_offset(rpc_pkt.u.reply.data);

		/* count value */
		rlen = ntohl(rpc_pkt.u.reply.data[1 + nfsv3_data_offset]);
		/* Skip unused values :
		 *	EOF:		32 bits value,
		 *	data_size:	32 bits value,
		 */
		data_ptr = (uchar *)
			&rpc_pkt.u.reply.data[4 + nfsv3_data_offset];
	}

	if (((uchar *)&rpc_pkt.u.reply.data[0] - (uchar *)&rpc_pkt + rlen) > len)
		return -9999;

	if (store_block(data_ptr, nfs_offset, rlen))
		return -9999;

	return rlen;
}

void nfs_pkt_recv(uchar *pkt, unsigned int len)
{
	int rlen;
	int reply;

	switch (nfs_state) {
	case STATE_PRCLOOKUP_PROG_MOUNT_REQ:
		if (rpc_lookup_reply(PROG_MOUNT, pkt, len) == -NFS_RPC_DROP)
			break;
		nfs_state = STATE_PRCLOOKUP_PROG_NFS_REQ;
		nfs_send();
		break;

	case STATE_PRCLOOKUP_PROG_NFS_REQ:
		if (rpc_lookup_reply(PROG_NFS, pkt, len) == -NFS_RPC_DROP)
			break;
		nfs_state = STATE_MOUNT_REQ;
		nfs_send();
		break;

	case STATE_MOUNT_REQ:
		reply = nfs_mount_reply(pkt, len);
		if (reply == -NFS_RPC_DROP) {
			break;
		} else if (reply == -NFS_RPC_ERR) {
			puts("*** ERROR: Cannot mount\n");
			/* just to be sure... */
			nfs_state = STATE_UMOUNT_REQ;
			nfs_send();
		} else if (reply == -NFS_RPC_PROG_MISMATCH &&
			   choosen_nfs_version != NFS_UNKOWN) {
			nfs_state = STATE_MOUNT_REQ;
			nfs_send();
		} else {
			nfs_state = STATE_LOOKUP_REQ;
			nfs_send();
		}
		break;

	case STATE_UMOUNT_REQ:
		reply = nfs_umountall_reply(pkt, len);
		if (reply == -NFS_RPC_DROP) {
			break;
		} else if (reply == -NFS_RPC_ERR) {
			debug("*** ERROR: Cannot umount\n");
			net_set_state(NETLOOP_FAIL);
		} else {
			puts("\ndone\n");
			net_set_state(nfs_download_state);
		}
		break;

	case STATE_LOOKUP_REQ:
		reply = nfs_lookup_reply(pkt, len);
		if (reply == -NFS_RPC_DROP) {
			break;
		} else if (reply == -NFS_RPC_ERR) {
			puts("*** ERROR: File lookup fail\n");
			nfs_state = STATE_UMOUNT_REQ;
			nfs_send();
		} else if (reply == -NFS_RPC_PROG_MISMATCH &&
			   choosen_nfs_version != NFS_UNKOWN) {
			/* umount */
			nfs_state = STATE_UMOUNT_REQ;
			nfs_send();
			/* And retry with another supported version */
			nfs_state = STATE_PRCLOOKUP_PROG_MOUNT_REQ;
			nfs_send();
		} else {
			nfs_state = STATE_READ_REQ;
			nfs_offset = 0;
			nfs_len = NFS_READ_SIZE;
			nfs_send();
		}
		break;

	case STATE_READLINK_REQ:
		reply = nfs_readlink_reply(pkt, len);
		if (reply == -NFS_RPC_DROP) {
			break;
		} else if (reply == -NFS_RPC_ERR) {
			puts("*** ERROR: Symlink fail\n");
			nfs_state = STATE_UMOUNT_REQ;
			nfs_send();
		} else {
			debug("Symlink --> %s\n", nfs_path);
			nfs_filename = nfs_basename(nfs_path);
			nfs_path     = nfs_dirname(nfs_path);

			nfs_state = STATE_MOUNT_REQ;
			nfs_send();
		}
		break;

	case STATE_READ_REQ:
		rlen = nfs_read_reply(pkt, len);
		if (rlen == -NFS_RPC_DROP)
			break;
		nfs_refresh_timeout();
		if (rlen > 0) {
			nfs_offset += rlen;
			nfs_send();
		} else if ((rlen == -NFSERR_ISDIR) || (rlen == -NFSERR_INVAL)) {
			/* symbolic link */
			nfs_state = STATE_READLINK_REQ;
			nfs_send();
		} else {
			if (!rlen)
				nfs_download_state = NETLOOP_SUCCESS;
			if (rlen < 0)
				debug("NFS READ error (%d)\n", rlen);
			nfs_state = STATE_UMOUNT_REQ;
			nfs_send();
		}
		break;
	}
}

