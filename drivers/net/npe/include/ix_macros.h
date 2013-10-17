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
 *      ix_macros.h
 *
 * = DESCRIPTION
 *      This file will define the basic preprocessor macros that are going to be used
 *      the IXA SDK Framework API.
 *
 * = AUTHOR
 *      Intel Corporation
 *
 * = CHANGE HISTORY
 *      4/22/2002 4:41:05 PM - creation time 
 * ============================================================================
 */

#if !defined(__IX_MACROS_H__)
#define __IX_MACROS_H__


#if defined(__cplusplus)
extern "C"
{
#endif /* end defined(__cplusplus) */


/**
 * MACRO NAME: IX_BIT_FIELD_MASK16
 *
 * DESCRIPTION: Builds the mask required to extract the bit field from a 16 bit unsigned integer value.
 *
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns a 16 bit mask that will extract the bit field from a 16 bit unsigned integer value.
 */
#define IX_BIT_FIELD_MASK16( \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                            ) \
                            ((ix_bit_mask16)((((ix_uint16)1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - \
                            (ix_uint16)1) << arg_FieldLSBBit))



/**
 * MACRO NAME: IX_GET_BIT_FIELD16
 *
 * DESCRIPTION: Extracts a bit field from 16 bit unsigned integer. The returned value is normalized in
 *          in the sense that will be right aligned.
 *
 * @Param:  - IN arg_PackedData16 a 16 bit unsigned integer that contains the bit field of interest.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns the value of the bit field. The value can be from 0 to (1 << (arg_FieldMSBBit + 1 -
 *          arg_FieldLSBBit)) - 1.
 */
#define IX_GET_BIT_FIELD16( \
                            arg_PackedData16, \
                            arg_FieldLSBBit, \
                            arg_FieldMSBBit \
                          ) \
                          (((ix_uint16)(arg_PackedData16) & IX_BIT_FIELD_MASK16(arg_FieldLSBBit, arg_FieldMSBBit)) >> \
                             arg_FieldLSBBit)


/**
 * MACRO NAME: IX_MAKE_BIT_FIELD16
 *
 * DESCRIPTION: This macro will create a temporary 16 bit value with the bit field
 *          desired set to the desired value.
 *
 * @Param:  - IN arg_BitFieldValue is the new value of the bit field. The value can be from 0 to 
 *          (1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - 1.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 * 
 * @Return: Returns a temporary ix_uint16 value that has the bit field set to the appropriate value.
 */
#define IX_MAKE_BIT_FIELD16( \
                             arg_BitFieldValue, \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                           ) \
                           (((ix_uint16)(arg_BitFieldValue) << arg_FieldLSBBit) & \
                           IX_BIT_FIELD_MASK16(arg_FieldLSBBit, arg_FieldMSBBit))
                                 
/**
 * MACRO NAME: IX_SET_BIT_FIELD16
 *
 * DESCRIPTION: Sets a new value for a bit field from a 16 bit unsigned integer.
 *
 * @Param:  - IN arg_PackedData16 a 16 bit unsigned integer that contains the bit field of interest.
 * @Param:  - IN arg_BitFieldValue is the new vale of the bit field. The value can be from 0 to 
 *          (1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - 1.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns the updated value of arg_PackedData16.
 */
#define IX_SET_BIT_FIELD16( \
                            arg_PackedData16, \
                            arg_BitFieldValue, \
                            arg_FieldLSBBit, \
                            arg_FieldMSBBit \
                          ) \
                          (arg_PackedData16 = (((ix_uint16)(arg_PackedData16) & \
                          ~(IX_BIT_FIELD_MASK16(arg_FieldLSBBit, arg_FieldMSBBit))) | \
                          IX_MAKE_BIT_FIELD16(arg_BitFieldValue, arg_FieldLSBBit, arg_FieldMSBBit))) 


/**
 * MACRO NAME: IX_BIT_FIELD_MASK32
 *
 * DESCRIPTION: Builds the mask required to extract the bit field from a 32 bit unsigned integer value.
 *
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns a 32 bit mask that will extract the bit field from a 32 bit unsigned integer value.
 */
#define IX_BIT_FIELD_MASK32( \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                           ) \
                           ((ix_bit_mask32)((((ix_uint32)1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - \
                           (ix_uint32)1) << arg_FieldLSBBit))



/**
 * MACRO NAME: IX_GET_BIT_FIELD32
 *
 * DESCRIPTION: Extracts a bit field from 32 bit unsigned integer. The returned value is normalized in
 *          in the sense that will be right aligned.
 *
 * @Param:  - IN arg_PackedData32 a 32 bit unsigned integer that contains the bit field of interest.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns the value of the bit field. The value can be from 0 to (1 << (arg_FieldMSBBit + 1 -
 *          arg_FieldLSBBit)) - 1.
 */
#define IX_GET_BIT_FIELD32( \
                            arg_PackedData32, \
                            arg_FieldLSBBit, \
                            arg_FieldMSBBit \
                          ) \
                          (((ix_uint32)(arg_PackedData32) & IX_BIT_FIELD_MASK32(arg_FieldLSBBit, arg_FieldMSBBit)) >> \
                             arg_FieldLSBBit)




/**
 * MACRO NAME: IX_MAKE_BIT_FIELD32
 *
 * DESCRIPTION: This macro will create a temporary 32 bit value with the bit field
 *          desired set to the desired value.
 *
 * @Param:  - IN arg_BitFieldValue is the new value of the bit field. The value can be from 0 to 
 *          (1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - 1.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 * 
 * @Return: Returns a temporary ix_uint32 value that has the bit field set to the appropriate value.
 */
#define IX_MAKE_BIT_FIELD32( \
                             arg_BitFieldValue, \
                             arg_FieldLSBBit, \
                             arg_FieldMSBBit \
                           ) \
                           (((ix_uint32)(arg_BitFieldValue) << arg_FieldLSBBit) & \
                           IX_BIT_FIELD_MASK32(arg_FieldLSBBit, arg_FieldMSBBit))
                            
                                
/**
 * MACRO NAME: IX_SET_BIT_FIELD32
 *
 * DESCRIPTION: Sets a new value for a bit field from a 32 bit unsigned integer.
 *
 * @Param:  - IN arg_PackedData32 a 32 bit unsigned integer that contains the bit field of interest.
 * @Param:  - IN arg_BitFieldValue is the new value of the bit field. The value can be from 0 to 
 *          (1 << (arg_FieldMSBBit + 1 - arg_FieldLSBBit)) - 1.
 * @Param:  - IN arg_FieldLSBBit an unsigned integer value representing the position of the least significant
 *          bit of the bit field.
 * @Param:  - IN arg_FieldMSBBit an unsigned integer value representing the position of the most significant
 *          bit of the bit field.
 *
 * @Return: Returns the updated value of arg_PackedData32.
 */
#define IX_SET_BIT_FIELD32( \
                            arg_PackedData32, \
                            arg_BitFieldValue, \
                            arg_FieldLSBBit, \
                            arg_FieldMSBBit \
                          ) \
                          (arg_PackedData32 = (((ix_uint32)(arg_PackedData32) & \
                          ~(IX_BIT_FIELD_MASK32(arg_FieldLSBBit, arg_FieldMSBBit))) | \
                          IX_MAKE_BIT_FIELD32(arg_BitFieldValue, arg_FieldLSBBit, arg_FieldMSBBit))) 



#if defined(__cplusplus)
}
#endif /* end defined(__cplusplus) */

#endif /* end !defined(__IX_MACROS_H__) */
