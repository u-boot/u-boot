/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Masami Komiya <mkomiya@sonare.it> 2004
 */

#ifndef __NFS_H__
#define __NFS_H__

#define PORTMAP_GETPORT 3

#define MOUNT_ADDENTRY  1
#define MOUNT_UMOUNTALL 4

#define NFS_LOOKUP      4
#define NFS_READLINK    5
#define NFS_READ        6

#define NFS3PROC_LOOKUP 3

#define NFS_FHSIZE      32
#define NFS3_FHSIZE     64

void nfs_start(void);	/* Begin NFS */

/**********************************************************************/

#endif /* __NFS_H__ */
