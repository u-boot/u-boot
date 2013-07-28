/**
 * @file IxOsalOsBufferMgt.h
 *
 * @brief vxworks-specific buffer management module definitions.
 *
 * Design Notes:
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
 */


#ifndef IX_OSAL_OS_BUFFER_MGT_H
#define IX_OSAL_OS_BUFFER_MGT_H

/*
 * use the defaul bufferMgt provided by OSAL framework.
 */
#define IX_OSAL_USE_DEFAULT_BUFFER_MGT

#include "IxOsalBufferMgtDefault.h"

#if 0  /* FIXME */
/* Define os-specific buffer macros for subfields */
#define IX_OSAL_OSBUF_MDATA(osBufPtr) IX_OSAL_MBUF_MDATA(osBufPtr)
             ( ((M_BLK *) osBufPtr)->m_data )

#define IX_OSAL_OSBUF_MLEN(osBufPtr) \
             ( ((M_BLK *) osBufPtr)->m_len )

#define IX_OSAL_OSBUF_PKT_LEN(osBufPtr) \
             ( ((M_BLK *) osBufPtr)->m_pkthdr.len )

#define IX_OSAL_OS_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr) \
        { \
            IX_OSAL_MBUF_OSBUF_PTR( (IX_OSAL_MBUF *) ixpBufPtr) = (void *) osBufPtr; \
            IX_OSAL_MBUF_MDATA((IX_OSAL_MBUF *) ixpBufPtr) =  IX_OSAL_OSBUF_MDATA(osBufPtr); \
            IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_PKT_LEN(osBufPtr); \
            IX_OSAL_MBUF_MLEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_MLEN(osBufPtr); \
        }

#define IX_OSAL_OS_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr) \
        { \
            if (ixpBufPtr == NULL) \
            { /* Do nothing */ } \
            else \
            { \
                (M_BLK *) osBufPtr = (M_BLK *) IX_OSAL_MBUF_OSBUF_PTR((IX_OSAL_MBUF *) ixpBufPtr); \
                if (osBufPtr == NULL) \
                { /* Do nothing */ } \
                else \
                { \
                    IX_OSAL_OSBUF_MLEN(osBufPtr) =IX_OSAL_MBUF_MLEN((IX_OSAL_MBUF *) ixpBufPtr); \
                    IX_OSAL_OSBUF_PKT_LEN(osBufPtr) =IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr); \
                } \
            } \
        }

#endif /* FIXME */

#endif /* #define IX_OSAL_OS_BUFFER_MGT_H */
