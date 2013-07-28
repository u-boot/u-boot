/**
 * @file IxNpeDl.h
 *
 * @date 14 December 2001

 * @brief This file contains the public API of the IXP400 NPE Downloader
 *        component.
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
 * @defgroup IxNpeDl IXP400 NPE-Downloader (IxNpeDl) API
 *
 * @brief The Public API for the IXP400 NPE Downloader
 *
 * @{
 */

#ifndef IXNPEDL_H
#define IXNPEDL_H

/*
 * Put the user defined include files required
 */
#include "IxOsalTypes.h"
#include "IxNpeMicrocode.h"

/*
 * #defines for function return types, etc.
 */

/**
 * @def IX_NPEDL_PARAM_ERR
 *
 * @brief NpeDl function return value for a parameter error
 */
#define IX_NPEDL_PARAM_ERR               2

/**
 * @def IX_NPEDL_RESOURCE_ERR
 *
 * @brief NpeDl function return value for a resource error
 */
#define IX_NPEDL_RESOURCE_ERR            3

/**
 * @def IX_NPEDL_CRITICAL_NPE_ERR
 *
 * @brief NpeDl function return value for a Critical NPE error occuring during
          download. Assume NPE is left in unstable condition if this value is
          returned or NPE is hang / halt.
 */
#define IX_NPEDL_CRITICAL_NPE_ERR        4

/**
 * @def IX_NPEDL_CRITICAL_MICROCODE_ERR
 *
 * @brief NpeDl function return value for a Critical Microcode error
 *        discovered during download. Assume NPE is left in unstable condition
 *        if this value is returned.
 */
#define IX_NPEDL_CRITICAL_MICROCODE_ERR  5

/**
 * @def IX_NPEDL_DEVICE_ERR
 *
 * @brief NpeDl function return value when image being downloaded
 *        is not meant for the device in use
 */
#define IX_NPEDL_DEVICE_ERR 6 

/**
 * @defgroup NPEImageID IXP400 NPE Image ID Definition
 *
 * @ingroup IxNpeDl
 *
 * @brief Definition of NPE Image ID to be passed to ixNpeDlNpeInitAndStart()
 *        as input of type UINT32 which has the following fields format:
 *
 *               Field [Bit Location] <BR>
 *               -------------------- <BR>
 *               Device ID [31 - 28] <BR>
 *               NPE ID [27 - 24] <BR>
 *               NPE Functionality ID [23 - 16] <BR>
 *               Major Release Number [15 -  8] <BR>
 *               Minor Release Number [7 - 0] <BR>
 *
 *
 * @{
 */

/**
 * @def IX_NPEDL_NPEIMAGE_FIELD_MASK
 *
 * @brief Mask for NPE Image ID's Field
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_NPEIMAGE_FIELD_MASK  0xff

/**
 * @def IX_NPEDL_NPEIMAGE_NPEID_MASK
 *
 * @brief Mask for NPE Image NPE ID's Field
 *
 */
#define IX_NPEDL_NPEIMAGE_NPEID_MASK  0xf

/**
 * @def IX_NPEDL_NPEIMAGE_DEVICEID_MASK
 *
 * @brief Mask for NPE Image Device ID's Field
 *
 */
#define IX_NPEDL_NPEIMAGE_DEVICEID_MASK  0xf

/**
 * @def IX_NPEDL_NPEIMAGE_BIT_LOC_NPEID
 *
 * @brief Location of NPE ID field in term of bit.
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_NPEIMAGE_BIT_LOC_NPEID  24

/**
 * @def IX_NPEDL_NPEIMAGE_BIT_LOC_FUNCTIONALITYID
 *
 * @brief Location of Functionality ID field in term of bit.
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_NPEIMAGE_BIT_LOC_FUNCTIONALITYID  16

/**
 * @def IX_NPEDL_NPEIMAGE_BIT_LOC_MAJOR
 *
 * @brief Location of Major Release Number field in term of bit.
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_NPEIMAGE_BIT_LOC_MAJOR  8

/**
 * @def IX_NPEDL_NPEIMAGE_BIT_LOC_MINOR
 *
 * @brief Location of Minor Release Number field in term of bit.
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_NPEIMAGE_BIT_LOC_MINOR  0

/**
 * @} addtogroup NPEImageID
 */

/**
 * @def ixNpeDlMicrocodeImageOverride(x)
 *
 * @brief  Map old terminology that uses term "image" to new term
 *        "image library"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define ixNpeDlMicrocodeImageOverride(x) ixNpeDlMicrocodeImageLibraryOverride(x)

/**
 * @def IxNpeDlVersionId
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IxNpeDlVersionId IxNpeDlImageId

/**
 * @def ixNpeDlVersionDownload
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define ixNpeDlVersionDownload(x,y) ixNpeDlImageDownload(x,y)

/**
 * @def ixNpeDlAvailableVersionsCountGet
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define ixNpeDlAvailableVersionsCountGet(x) ixNpeDlAvailableImagesCountGet(x)

/**
 * @def ixNpeDlAvailableVersionsListGet
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define ixNpeDlAvailableVersionsListGet(x,y) ixNpeDlAvailableImagesListGet(x,y)

 /**
 * @def ixNpeDlLoadedVersionGet
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define ixNpeDlLoadedVersionGet(x,y) ixNpeDlLoadedImageGet(x,y)

 /**
 * @def clientImage
 *
 * @brief  Map old terminology that uses term "image" to new term
 *        "image library"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define clientImage clientImageLibrary

 /**
 * @def versionIdPtr
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define versionIdPtr imageIdPtr

 /**
 * @def numVersionsPtr
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define numVersionsPtr numImagesPtr

/**
 * @def versionIdListPtr
 *
 * @brief  Map old terminology that uses term "version" to new term
 *        "image"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define versionIdListPtr imageIdListPtr

/**
 * @def IxNpeDlBuildId
 *
 * @brief  Map old terminology that uses term "buildId" to new term
 *        "functionalityId"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IxNpeDlBuildId IxNpeDlFunctionalityId

/**
 * @def buildId
 *
 * @brief  Map old terminology that uses term "buildId" to new term
 *        "functionalityId"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define buildId functionalityId

/**
 * @def IX_NPEDL_MicrocodeImage
 *
 * @brief  Map old terminology that uses term "image" to new term
 *        "image library"
 *
 * @warning <b>THIS #define HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
#define IX_NPEDL_MicrocodeImage IX_NPEDL_MicrocodeImageLibrary

/*
 * Typedefs
 */

/**
 * @typedef IxNpeDlFunctionalityId
 * @brief Used to make up Functionality ID field of Image Id
 *
 * @warning <b>THIS typedef HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
typedef UINT8 IxNpeDlFunctionalityId;

/**
 * @typedef IxNpeDlMajor
 * @brief Used to make up Major Release field of Image Id
 *
 * @warning <b>THIS typedef HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
typedef UINT8 IxNpeDlMajor;

/**
 * @typedef IxNpeDlMinor
 * @brief Used to make up Minor Revision field of Image Id
 *
 * @warning <b>THIS typedef HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
typedef UINT8 IxNpeDlMinor;

/*
 * Enums
 */

/**
 * @brief NpeId numbers to identify NPE A, B or C
 * @note In this context, for IXP425 Silicon (B0):<br>
 *      - NPE-A has HDLC, HSS, AAL and UTOPIA Coprocessors.<br>
 *      - NPE-B has Ethernet Coprocessor.<br>
 *      - NPE-C has Ethernet, AES, DES and HASH Coprocessors.<br>
 *      - IXP400 Product Line have different combinations of coprocessors.
 */
typedef enum
{
  IX_NPEDL_NPEID_NPEA = 0,    /**< Identifies NPE A */
  IX_NPEDL_NPEID_NPEB,        /**< Identifies NPE B */
  IX_NPEDL_NPEID_NPEC,        /**< Identifies NPE C */
  IX_NPEDL_NPEID_MAX          /**< Total Number of NPEs */
} IxNpeDlNpeId;

/*
 * Structs
 */

/**
 * @brief Image Id to identify each image contained in an image library
 *
 * @warning <b>THIS struct HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart for more information.
 */
typedef struct
{
    IxNpeDlNpeId   npeId;   /**< NPE ID */
    IxNpeDlFunctionalityId functionalityId; /**< Build ID indicates functionality of image */
    IxNpeDlMajor   major;   /**< Major Release Number */
    IxNpeDlMinor   minor;   /**< Minor Revision Number */
} IxNpeDlImageId;

/*
 * Prototypes for interface functions
 */

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeInitAndStart (UINT32 imageId)
 *
 * @brief Stop, reset, download microcode (firmware) and finally start NPE.
 *
 * @param imageId UINT32 [in] - Id of the microcode image to download.
 *
 * This function locates the image specified by the <i>imageId</i> parameter
 * from the default microcode image library which is included internally by
 * this component.
 * It then stops and resets the NPE, loads the firmware image onto the NPE,
 * and then restarts the NPE.
 *
 * @note A list of valid image IDs is included in this header file.
 *       See #defines with prefix IX_NPEDL_NPEIMAGE_...
 *
 * @note This function, along with @ref ixNpeDlCustomImageNpeInitAndStart
 *       and @ref ixNpeDlLoadedImageFunctionalityGet, supercedes the following
 *       functions which are deprecated and will be removed completely in a
 *       future release:
 *       - @ref ixNpeDlImageDownload
 *       - @ref ixNpeDlAvailableImagesCountGet
 *       - @ref ixNpeDlAvailableImagesListGet
 *       - @ref ixNpeDlLatestImageGet
 *       - @ref ixNpeDlLoadedImageGet
 *       - @ref ixNpeDlMicrocodeImageLibraryOverride
 *       - @ref ixNpeDlNpeExecutionStop
 *       - @ref ixNpeDlNpeStopAndReset
 *       - @ref ixNpeDlNpeExecutionStart
 *
 * @pre
 *         - The Client is responsible for ensuring mutual access to the NPE.
 * @post
 *         - The NPE Instruction Pipeline will be cleared if State Information
 *           has been downloaded.
 *         - If the download fails with a critical error, the NPE may
 *           be left in an ususable state.
 * @return
 *         - IX_SUCCESS if the download was successful;
 *         - IX_NPEDL_PARAM_ERR if a parameter error occured
 *         - IX_NPEDL_CRITICAL_NPE_ERR if a critical NPE error occured during
 *           download
 *         - IX_NPEDL_CRITICAL_MICROCODE_ERR if a critical microcode error
 *           occured during download
 *         - IX_NPEDL_DEVICE_ERR if the image being loaded is not meant for 
 *           the device currently running.
 *         - IX_FAIL if NPE is not available or image is failed to be located.
 *           A warning is issued if the NPE is not present.
 */
PUBLIC IX_STATUS
ixNpeDlNpeInitAndStart (UINT32 npeImageId);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlCustomImageNpeInitAndStart (UINT32 *imageLibrary,
                                                           UINT32 imageId)
 *
 * @brief Stop, reset, download microcode (firmware) and finally start NPE
 *
 * @param imageId UINT32 [in] - Id of the microcode image to download.
 *
 * This function locates the image specified by the <i>imageId</i> parameter
 * from the specified microcode image library which is pointed to by the
 * <i>imageLibrary</i> parameter.
 * It then stops and resets the NPE, loads the firmware image onto the NPE,
 * and then restarts the NPE.
 *
 * This is a facility for users who wish to use an image from an external
 * library of NPE firmware images.  To use a standard image from the
 * built-in library, see @ref ixNpeDlNpeInitAndStart instead.
 *
 * @note A list of valid image IDs is included in this header file.
 *       See #defines with prefix IX_NPEDL_NPEIMAGE_...
 *
 * @note This function, along with @ref ixNpeDlNpeInitAndStart
 *       and @ref ixNpeDlLoadedImageFunctionalityGet, supercedes the following
 *       functions which are deprecated and will be removed completely in a
 *       future release:
 *       - @ref ixNpeDlImageDownload
 *       - @ref ixNpeDlAvailableImagesCountGet
 *       - @ref ixNpeDlAvailableImagesListGet
 *       - @ref ixNpeDlLatestImageGet
 *       - @ref ixNpeDlLoadedImageGet
 *       - @ref ixNpeDlMicrocodeImageLibraryOverride
 *       - @ref ixNpeDlNpeExecutionStop
 *       - @ref ixNpeDlNpeStopAndReset
 *       - @ref ixNpeDlNpeExecutionStart
 *
 * @pre
 *         - The Client is responsible for ensuring mutual access to the NPE.
 *         - The image library supplied must be in the correct format for use
 *           by the NPE Downloader (IxNpeDl) component.  Details of the library
 *           format are contained in the Functional Specification document for
 *           IxNpeDl.
 * @post
 *         - The NPE Instruction Pipeline will be cleared if State Information
 *           has been downloaded.
 *         - If the download fails with a critical error, the NPE may
 *           be left in an ususable state.
 * @return
 *         - IX_SUCCESS if the download was successful;
 *         - IX_NPEDL_PARAM_ERR if a parameter error occured
 *         - IX_NPEDL_CRITICAL_NPE_ERR if a critical NPE error occured during
 *           download
 *         - IX_NPEDL_CRITICAL_MICROCODE_ERR if a critical microcode error
 *           occured during download
 *         - IX_NPEDL_DEVICE_ERR if the image being loaded is not meant for 
 *           the device currently running.
 *         - IX_FAIL if NPE is not available or image is failed to be located.
 *           A warning is issued if the NPE is not present.
 */
PUBLIC IX_STATUS
ixNpeDlCustomImageNpeInitAndStart (UINT32 *imageLibrary,
                    UINT32 npeImageId);


/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlLoadedImageFunctionalityGet (IxNpeDlNpeId npeId,
                                                        UINT8 *functionalityId)
 *
 * @brief Gets the functionality of the image last loaded on a particular NPE
 *
 * @param npeId  @ref IxNpeDlNpeId [in]      - Id of the target NPE.
 * @param functionalityId UINT8* [out] - the functionality ID of the image
 *                                       last loaded on the NPE.
 *
 * This function retrieves the functionality ID of the image most recently
 * downloaded successfully to the specified NPE.  If the NPE does not contain
 * a valid image, this function returns a FAIL status.
 *
 * @warning This function is not intended for general use, as a knowledge of
 * how to interpret the functionality ID is required.  As such, this function
 * should only be used by other Access Layer components of the IXP400 Software
 * Release.
 *
 * @pre
 *
 * @post
 *
 * @return
 *     -  IX_SUCCESS if the operation was successful
 *     -  IX_NPEDL_PARAM_ERR if a parameter error occured
 *     -  IX_FAIL if the NPE does not have a valid image loaded
 */
PUBLIC IX_STATUS
ixNpeDlLoadedImageFunctionalityGet (IxNpeDlNpeId npeId,
                                    UINT8 *functionalityId);


/**
 * @ingroup IxNpeDl
 *
 * @fn IX_STATUS ixNpeDlMicrocodeImageLibraryOverride (UINT32 *clientImageLibrary)
 *
 * @brief This instructs NPE Downloader to use client-supplied microcode image library.
 *
 * @param clientImageLibrary UINT32* [in]  - Pointer to the client-supplied
 *                                   NPE microcode image library
 *
 * This function sets NPE Downloader to use a client-supplied microcode image library
 * instead of the standard image library which is included by the NPE Downloader.
 * <b>This function is provided mainly for increased testability and should not
 * be used in normal circumstances.</b> When not used, NPE Downloader will use
 * a "built-in" image library, local to this component, which should always contain the
 * latest microcode for the NPEs.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *      - <i>clientImageLibrary</i> should point to a microcode image library valid for use
 *        by the NPE Downloader component.
 *
 * @post
 *      - the client-supplied image library will be used for all subsequent operations
 *        performed by the NPE Downloader
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL if the client-supplied image library did not contain a valid signature
 */
PUBLIC IX_STATUS
ixNpeDlMicrocodeImageLibraryOverride (UINT32 *clientImageLibrary);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlImageDownload (IxNpeDlImageId *imageIdPtr,
                                                BOOL verify)
 *
 * @brief Stop, reset, download microcode and finally start NPE.
 *
 * @param imageIdPtr @ref IxNpeDlImageId* [in] - Pointer to Id of the microcode
 *                                              image to download.
 * @param verify BOOL [in]     - ON/OFF option to verify the download. If ON
 *                               (verify == true), the Downloader will read back
 *                               each word written to the NPE registers to
 *                               ensure the download operation was successful.
 *
 * Using the <i>imageIdPtr</i>, this function locates a particular image of
 * microcode in the microcode image library in memory, and downloads the microcode
 * to a particular NPE.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *         - The Client is responsible for ensuring mutual access to the NPE.
 *       - The Client should use ixNpeDlLatestImageGet() to obtain the latest
 *         version of the image before attempting download.
 * @post
 *         - The NPE Instruction Pipeline will be cleared if State Information
 *           has been downloaded.
 *         - If the download fails with a critical error, the NPE may
 *           be left in an ususable state.
 * @return
 *         - IX_SUCCESS if the download was successful;
 *         - IX_NPEDL_PARAM_ERR if a parameter error occured
 *         - IX_NPEDL_CRITICAL_NPE_ERR if a critical NPE error occured during
 *           download
 *         - IX_PARAM_CRITICAL_MICROCODE_ERR if a critical microcode error
 *           occured during download
 *         - IX_FAIL if NPE is not available or image is failed to be located.
 *           A warning is issued if the NPE is not present.
 */
PUBLIC IX_STATUS
ixNpeDlImageDownload (IxNpeDlImageId *imageIdPtr,
            BOOL verify);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlAvailableImagesCountGet (UINT32 *numImagesPtr)
 *
 * @brief Get the number of Images available in a microcode image library
 *
 * @param numImagesPtr UINT32* [out] - A pointer to the number of images in
 *                                       the image library.
 *
 * Gets the number of images available in the microcode image library.
 * Then returns this in a variable pointed to by <i>numImagesPtr</i>.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *     - The Client should declare the variable to which numImagesPtr points
 *
 * @post
 *
 * @return
 *     - IX_SUCCESS if the operation was successful
 *     - IX_NPEDL_PARAM_ERR if a parameter error occured
 *     - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlAvailableImagesCountGet (UINT32 *numImagesPtr);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlAvailableImagesListGet (IxNpeDlImageId *imageIdListPtr,
                                                         UINT32 *listSizePtr)
 *
 * @brief Get a list of the images available in a microcode image library
 *
 * @param imageIdListPtr @ref IxNpeDlImageId* [out] - Array to contain list of
 *                                                   image Ids (memory
 *                                                   allocated by Client).
 * @param listSizePtr UINT32* [inout]  - As an input, this param should point
 *                                       to the max number of Ids the
 *                                       <i>imageIdListPtr</i> array can
 *                                       hold. NpeDl will replace the input
 *                                       value of this parameter with the
 *                                       number of image Ids actually filled
 *                                       into the array before returning.
 *
 * Finds list of images available in the microcode image library.
 * Fills these into the array pointed to by <i>imageIdListPtr</i>
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *     - The Client should declare the variable to which numImagesPtr points
 *     - The Client should create an array (<i>imageIdListPtr</i>) large
 *       enough to contain all the image Ids in the image library
 *
 * @post
 *
 * @return
 *     - IX_SUCCESS if the operation was successful
 *     - IX_NPEDL_PARAM_ERR if a parameter error occured
 *     - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlAvailableImagesListGet (IxNpeDlImageId *imageIdListPtr,
                 UINT32 *listSizePtr);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlLoadedImageGet (IxNpeDlNpeId npeId,
                                                IxNpeDlImageId *imageIdPtr)
 *
 * @brief Gets the Id of the image currently loaded on a particular NPE
 *
 * @param npeId @ref IxNpeDlNpeId [in]              - Id of the target NPE.
 * @param imageIdPtr @ref IxNpeDlImageId* [out]     - Pointer to the where the
 *                                               image id should be stored.
 *
 * If an image of microcode was previously downloaded successfully to the NPE
 * by NPE Downloader, this function returns in <i>imageIdPtr</i> the image
 * Id of that image loaded on the NPE.
 *
 * @pre
 *     - The Client has allocated memory to the <i>imageIdPtr</i> pointer.
 *
 * @post
 *
 * @return
 *     -  IX_SUCCESS if the operation was successful
 *     -  IX_NPEDL_PARAM_ERR if a parameter error occured
 *     -  IX_FAIL if the NPE doesn't currently have a image loaded
 */
PUBLIC IX_STATUS
ixNpeDlLoadedImageGet (IxNpeDlNpeId npeId,
             IxNpeDlImageId *imageIdPtr);

/**
 * @fn PUBLIC IX_STATUS ixNpeDlLatestImageGet (IxNpeDlNpeId npeId, IxNpeDlFunctionalityId
                                        functionalityId, IxNpeDlImageId *imageIdPtr)
 *
 * @brief This instructs NPE Downloader to get Id of the latest version for an
 * image that is specified by the client.
 *
 * @param npeId @ref IxNpeDlNpeId [in]                    - Id of the target NPE.
 * @param functionalityId @ref IxNpeDlFunctionalityId [in]    - functionality of the image
 * @param imageIdPtr @ref IxNpeDlImageId* [out]               - Pointer to the where the
 *                                                         image id should be stored.
 *
 * This function sets NPE Downloader to return the id of the latest version for
 * image.  The user will select the image by providing a particular NPE
 * (specifying <i>npeId</i>) with particular functionality (specifying
 * <i>FunctionalityId</i>). The most recent version available as determined by the
 * highest Major and Minor revision numbers is returned in <i>imageIdPtr</i>.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlLatestImageGet (IxNpeDlNpeId npeId,
             IxNpeDlFunctionalityId functionalityId,
                    IxNpeDlImageId *imageIdPtr);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId)
 *
 * @brief Stops and Resets an NPE
 *
 * @param npeId @ref IxNpeDlNpeId [in] - Id of the target NPE.
 *
 * This function performs a soft NPE reset by writing reset values to
 * particular NPE registers. Note that this does not reset NPE co-processors.
 * This implicitly stops NPE code execution before resetting the NPE.
 *
 * @note It is no longer necessary to call this function before downloading
 * a new image to the NPE.  It is left on the API only to allow greater control
 * of NPE execution if required.  Where appropriate, use @ref ixNpeDlNpeInitAndStart
 * or @ref ixNpeDlCustomImageNpeInitAndStart instead.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 *      - IX_NPEDL_CRITICAL_NPE_ERR failed to reset NPE due to timeout error. 
 *        Timeout error could happen if NPE hang
 */
PUBLIC IX_STATUS
ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId)
 *
 * @brief Starts code execution on a NPE
 *
 * @param npeId @ref IxNpeDlNpeId [in] - Id of the target NPE
 *
 * Starts execution of code on a particular NPE.  A client would typically use
 * this after a download to NPE is performed, to start/restart code execution
 * on the NPE.
 *
 * @note It is no longer necessary to call this function after downloading
 * a new image to the NPE.  It is left on the API only to allow greater control
 * of NPE execution if required.  Where appropriate, use @ref ixNpeDlNpeInitAndStart
 * or @ref ixNpeDlCustomImageNpeInitAndStart instead.
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *     - Note that this function does not set the NPE Next Program Counter
 *       (NextPC), so it should be set beforehand if required by downloading
 *       appropriate State Information (using ixNpeDlVersionDownload()).
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId)
 *
 * @brief Stops code execution on a NPE
 *
 * @param npeId @ref IxNpeDlNpeId [in] - Id of the target NPE
 *
 * Stops execution of code on a particular NPE.  This would typically be used
 * by a client before a download to NPE is performed, to stop code execution on
 * an NPE, unless ixNpeDlNpeStopAndReset() is used instead.  Unlike
 * ixNpeDlNpeStopAndReset(), this function only halts the NPE and leaves
 * all registers and settings intact. This is useful, for example, between
 * stages of a multi-stage download, to stop the NPE prior to downloading the
 * next image while leaving the current state of the NPE intact..
 *
 * @warning <b>THIS FUNCTION HAS BEEN DEPRECATED AND SHOULD NOT BE USED.</b>
 *       It will be removed in a future release.
 *       See @ref ixNpeDlNpeInitAndStart and @ref ixNpeDlCustomImageNpeInitAndStart.
 *
 * @pre
 *     - The Client is responsible for ensuring mutual access to the NPE.
 *
 * @post
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_NPEDL_PARAM_ERR if a parameter error occured
 *      - IX_FAIL otherwise
 */
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC IX_STATUS ixNpeDlUnload (void)
 *
 * @brief This function will uninitialise the IxNpeDl component.
 *
 * This function will uninitialise the IxNpeDl component.  It should only be
 * called once, and only if the IxNpeDl component has already been initialised by
 * calling any of the following functions:
 * - @ref ixNpeDlNpeInitAndStart
 * - @ref ixNpeDlCustomImageNpeInitAndStart
 * - @ref ixNpeDlImageDownload     (deprecated)
 * - @ref ixNpeDlNpeStopAndReset   (deprecated)
 * - @ref ixNpeDlNpeExecutionStop  (deprecated)
 * - @ref ixNpeDlNpeExecutionStart (deprecated)
 *
 * If possible, this function should be called before a soft reboot or unloading
 * a kernel module to perform any clean up operations required for IxNpeDl.
 *
 * The following actions will be performed by this function:
 * - Unmapping of any kernel memory mapped by IxNpeDl
 *
 * @return
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */

PUBLIC IX_STATUS 
ixNpeDlUnload (void);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC void ixNpeDlStatsShow (void)
 *
 * @brief This function will display run-time statistics from the IxNpeDl
 *        component
 *
 * @return none
 */
PUBLIC void
ixNpeDlStatsShow (void);

/**
 * @ingroup IxNpeDl
 *
 * @fn PUBLIC void ixNpeDlStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl component
 *
 * @return none
 */
PUBLIC void
ixNpeDlStatsReset (void);

#endif /* IXNPEDL_H */

/**
 * @} defgroup IxNpeDl
 */


