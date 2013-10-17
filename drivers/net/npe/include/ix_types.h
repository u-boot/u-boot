/**
 * ============================================================================
 * = COPYRIGHT
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
 * = PRODUCT
 *      Intel(r) IXP425 Software Release
 *
 * = FILENAME
 *      ix_types.h
 *
 * = DESCRIPTION
 *      This file will define generic types that will guarantee the protability
 *      between different architectures and compilers. It should be used the entire
 *      IXA SDK Framework API.
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = CHANGE HISTORY
 *      4/22/2002 4:44:17 PM - creation time 
 * ============================================================================
 */

#if !defined(__IX_TYPES_H__)
#define __IX_TYPES_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */


/**
 * Define generic integral data types that will guarantee the size.
 */

/**
 * TYPENAME: ix_int8
 * 
 * DESCRIPTION: This type defines an 8 bit signed integer value.
 *
 */
typedef signed char ix_int8;


/**
 * TYPENAME: ix_uint8
 * 
 * DESCRIPTION: This type defines an 8 bit unsigned integer value.
 *
 */
typedef unsigned char ix_uint8;


/**
 * TYPENAME: ix_int16
 * 
 * DESCRIPTION: This type defines an 16 bit signed integer value.
 *
 */
typedef signed short int ix_int16;


/**
 * TYPENAME: ix_uint16
 * 
 * DESCRIPTION: This type defines an 16 bit unsigned integer value.
 *
 */
typedef unsigned short int ix_uint16;


/**
 * TYPENAME: ix_int32
 * 
 * DESCRIPTION: This type defines an 32 bit signed integer value.
 *
 */
typedef signed int ix_int32;


/**
 * TYPENAME: ix_uint32
 * 
 * DESCRIPTION: This type defines an 32 bit unsigned integer value.
 *
 */
#ifndef __wince
typedef unsigned int ix_uint32;
#else
typedef unsigned long ix_uint32;
#endif

/**
 * TYPENAME: ix_int64
 * 
 * DESCRIPTION: This type defines an 64 bit signed integer value.
 *
 */
#ifndef __wince
__extension__ typedef signed long long int ix_int64;
#endif

/**
 * TYPENAME: ix_uint64
 * 
 * DESCRIPTION: This type defines an 64 bit unsigned integer value.
 *
 */
#ifndef __wince
__extension__ typedef unsigned long long int ix_uint64;
#endif


/**
 * TYPENAME: ix_bit_mask8
 * 
 * DESCRIPTION: This is a generic type for a 8 bit mask. 
 */
typedef ix_uint8 ix_bit_mask8;


/**
 * TYPENAME: ix_bit_mask16
 * 
 * DESCRIPTION: This is a generic type for a 16 bit mask. 
 */
typedef ix_uint16 ix_bit_mask16;


/**
 * TYPENAME: ix_bit_mask32
 * 
 * DESCRIPTION: This is a generic type for a 32 bit mask. 
 */
typedef ix_uint32 ix_bit_mask32;


/**
 * TYPENAME: ix_bit_mask64
 * 
 * DESCRIPTION: This is a generic type for a 64 bit mask. 
 */
#ifndef __wince
typedef ix_uint64 ix_bit_mask64;
#endif


/**
 * TYPENAME: ix_handle
 * 
 * DESCRIPTION: This type defines a generic handle.
 *
 */
typedef ix_uint32 ix_handle;



/**
 * DESCRIPTION: This symbol defines a NULL handle
 *
 */
#define IX_NULL_HANDLE   ((ix_handle)0) 


#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_TYPES_H__) */
