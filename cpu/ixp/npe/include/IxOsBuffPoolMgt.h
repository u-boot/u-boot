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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
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

