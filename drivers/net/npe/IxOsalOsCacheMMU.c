/**
 * @file IxOsalOsCacheMMU.c (linux)
 *
 * @brief Cache MemAlloc and MemFree.
 *
 *
 * @par
 * IXP400 SW Release version 1.5
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

#include "IxOsal.h"

#include <malloc.h>

/*
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
void *
ixOsalCacheDmaMalloc (UINT32 n)
{
	return malloc(n);
}

/*
 *
 */
void
ixOsalCacheDmaFree (void *ptr)
{
	free(ptr);
}
