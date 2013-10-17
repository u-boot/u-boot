/**
 * @file IxOsBuffPoolMgt.h (Replaced by OSAL)
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool implementation API
 *
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 * This module contains the implementation of the OS Services buffer pool
 * management service.  This module provides routines for creating pools
 * of buffers for exchange of network data, getting and returning buffers
 * from and to the pool, and some other utility functions. 
 * <P>
 * Currently, the pool has 2 underlying implementations - one for the vxWorks
 * OS, and another which attempts to be OS-agnostic so that it can be used on
 * other OS's such as Linux.  The API is largely the same for all OS's,
 * but there are some differences to be aware of.  These are documented
 * in the API descriptions below.
 * <P>
 * The most significant difference is this: when this module is used with
 * the WindRiver VxWorks OS, it will create a pool of vxWorks "MBufs".
 * These can be used directly with the vxWorks "netBufLib" OS Library.
 * For other OS's, it will create a pool of generic buffers.  These may need
 * to be converted into other buffer types (sk_buff's in Linux, for example)
 * before being used with any built-in OS routines available for
 * manipulating network data buffers.
 *
 * @sa IxOsBuffMgt.h
 */

#ifndef IXOSBUFFPOOLMGT_H
#define IXOSBUFFPOOLMGT_H

#include "IxOsalBackward.h"

#endif  /* IXOSBUFFPOOLMGT_H */

