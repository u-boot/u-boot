/**
 * @file IxNpeDlImageMgr_p.h
 *
 * @author Intel Corporation
 * @date 14 December 2001

 * @brief This file contains the private API for the ImageMgr module
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
*/

/**
 * @defgroup IxNpeDlImageMgr_p IxNpeDlImageMgr_p
 *
 * @brief The private API for the IxNpeDl ImageMgr module
 * 
 * @{
 */

#ifndef IXNPEDLIMAGEMGR_P_H
#define IXNPEDLIMAGEMGR_P_H


/*
 * Put the user defined include files required.
 */
#include "IxNpeDl.h"
#include "IxOsalTypes.h"


/*
 * #defines and macros
 */

/**
 * @def IX_NPEDL_IMAGEMGR_SIGNATURE
 *
 * @brief Signature found as 1st word in a microcode image library
 */
#define IX_NPEDL_IMAGEMGR_SIGNATURE      0xDEADBEEF

/**
 * @def IX_NPEDL_IMAGEMGR_END_OF_HEADER
 *
 * @brief Marks end of header in a microcode image library
 */
#define IX_NPEDL_IMAGEMGR_END_OF_HEADER  0xFFFFFFFF

/**
 * @def IX_NPEDL_IMAGEID_NPEID_OFFSET
 *
 * @brief Offset from LSB of NPE ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_NPEID_OFFSET      24

/**
 * @def IX_NPEDL_IMAGEID_DEVICEID_OFFSET
 *
 * @brief Offset from LSB of Device ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_DEVICEID_OFFSET   28

/**
 * @def IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET
 *
 * @brief Offset from LSB of Functionality ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET 16

/**
 * @def IX_NPEDL_IMAGEID_MAJOR_OFFSET
 *
 * @brief Offset from LSB of Major revision field in Image ID
 */
#define IX_NPEDL_IMAGEID_MAJOR_OFFSET      8

/**
 * @def IX_NPEDL_IMAGEID_MINOR_OFFSET
 *
 * @brief Offset from LSB of Minor revision field in Image ID
 */
#define IX_NPEDL_IMAGEID_MINOR_OFFSET      0


/**
 * @def IX_NPEDL_NPEID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract NPE ID field from Image ID
 */
#define IX_NPEDL_NPEID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_NPEID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_NPEID_MASK)

/**
 * @def IX_NPEDL_DEVICEID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract NPE ID field from Image ID
 */
#define IX_NPEDL_DEVICEID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_DEVICEID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_DEVICEID_MASK)

/**
 * @def IX_NPEDL_FUNCTIONID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Functionality ID field from Image ID
 */
#define IX_NPEDL_FUNCTIONID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)

/**
 * @def IX_NPEDL_MAJOR_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Major revision field from Image ID
 */
#define IX_NPEDL_MAJOR_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_MAJOR_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)

/**
 * @def IX_NPEDL_MINOR_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Minor revision field from Image ID
 */
#define IX_NPEDL_MINOR_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_MINOR_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)


/*
 * Prototypes for interface functions
 */

/**
 * @fn IX_STATUS ixNpeDlImageMgrMicrocodeImageLibraryOverride (UINT32 *clientImageLibrary)
 * 
 * @brief This instructs NPE Downloader to use client-supplied microcode image library.
 *
 * This function sets NPE Downloader to use a client-supplied microcode image library
 * instead of the standard image library which is included by the NPE Downloader.
 * 
 * @note THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.
 *       It will be removed in a future release.
 *       See API header file IxNpeDl.h for more information.             
 *
 * @pre
 *    - <i>clientImageLibrary</i> should point to a microcode image library valid for use
 *      by the NPE Downloader component.
 *
 * @post
 *    - the client-supplied image uibrary will be used for all subsequent operations
 *      performed by the NPE Downloader
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL if the client-supplied image library did not contain a valid signature
 */ 
IX_STATUS
ixNpeDlImageMgrMicrocodeImageLibraryOverride (UINT32 *clientImageLibrary);


/**
 * @fn IX_STATUS ixNpeDlImageMgrImageListExtract (IxNpeDlImageId *imageListPtr,
                                                    UINT32 *numImages)
 * 
 * @brief Extracts a list of images available in the NPE microcode image library.
 *
 * @param IxNpeDlImageId* [out] imageListPtr - pointer to array to contain
 *                                                 a list of images. If NULL,
 *                                                 only the number of images 
 *                                                 is returned (in
 *                                                 <i>numImages</i>)
 * @param UINT32* [inout] numImages - As input, it points to a variable
 *                                      containing the number of images which
 *                                      can be stored in the
 *                                      <i>imageListPtr</i> array. Its value
 *                                      is ignored as input if
 *                                      <i>imageListPtr</i> is NULL. As an
 *                                      output, it will contain number of
 *                                      images in the image library.
 * 
 * This function reads the header of the microcode image library and extracts a list of the
 * images available in the image library.  It can also be used to find the number of
 * images in the image library.
 * 
 * @note THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.
 *       It will be removed in a future release.
 *       See API header file IxNpeDl.h for more information.             
 *
 * @pre
 *    - if <i>imageListPtr</i> != NULL, <i>numImages</i> should reflect the
 *      number of image Id elements the <i>imageListPtr</i> can contain.
 *
 * @post
 *    - <i>numImages</i> will reflect the number of image Id's found in the
 *      microcode image library.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrImageListExtract (IxNpeDlImageId *imageListPtr,
				   UINT32 *numImages);


/**
 * @fn IX_STATUS ixNpeDlImageMgrImageLocate (IxNpeDlImageId *imageId,
                                               UINT32 **imagePtr,
                                               UINT32 *imageSize)
 * 
 * @brief Finds a image block in the NPE microcode image library. 
 *
 * @param IxNpeDlImageId* [in] imageId - the id of the image to locate
 * @param UINT32** [out] imagePtr        - pointer to the image in memory
 * @param UINT32* [out] imageSize        - size (in 32-bit words) of image
 * 
 * This function examines the header of the microcode image library for the location
 * and size of the specified image.
 * 
 * @note THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.
 *       It will be removed in a future release.
 *       See API header file IxNpeDl.h for more information.             
 *
 * @pre
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrImageLocate (IxNpeDlImageId *imageId,
			      UINT32 **imagePtr,
			      UINT32 *imageSize);

/**
 * @fn IX_STATUS ixNpeDlImageMgrLatestImageExtract (IxNpeDlImageId *imageId)
 * 
 * @brief Finds the most recent version of an image in the NPE image library. 
 *
 * @param IxNpeDlImageId* [inout] imageId - the id of the image 
 * 
 * This function determines the most recent version of a specified image by its 
 * higest major release and minor revision numbers
 * 
 * @note THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.
 *       It will be removed in a future release.
 *       See API header file IxNpeDl.h for more information.             
 *
 * @pre
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrLatestImageExtract (IxNpeDlImageId *imageId);

/**
 * @fn void ixNpeDlImageMgrStatsShow (void)
 *
 * @brief This function will display the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsShow (void);


/**
 * @fn void ixNpeDlImageMgrStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsReset (void);


/**
 * @fn IX_STATUS ixNpeDlImageMgrImageGet (UINT32 *imageLibrary,
                                          UINT32 imageId,
                                          UINT32 **imagePtr,
                                          UINT32 *imageSize)
 * 
 * @brief Finds a image block in the NPE microcode image library. 
 *
 * @param UINT32*  [in]  imageLibrary - the image library to use
 * @param UINT32   [in]  imageId      - the id of the image to locate
 * @param UINT32** [out] imagePtr     - pointer to the image in memory
 * @param UINT32*  [out] imageSize    - size (in 32-bit words) of image
 * 
 * This function examines the header of the specified microcode image library
 * for the location and size of the specified image.  It returns a pointer to
 * the image in the <i>imagePtr</i> parameter.
 * If no image library is specified (imageLibrary == NULL), then the default
 * built-in image library will be used.
 * 
 * @pre
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrImageFind (UINT32 *imageLibrary,
                          UINT32 imageId,
			  UINT32 **imagePtr,
			  UINT32 *imageSize);


#endif /* IXNPEDLIMAGEMGR_P_H */

/**
 * @} defgroup IxNpeDlImageMgr_p
 */
