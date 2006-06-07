/**
 * @file IxNpeDlImageMgr.c
 *
 * @author Intel Corporation
 * @date 09 January 2002
 *
 * @brief This file contains the implementation of the private API for the 
 *        IXP425 NPE Downloader ImageMgr module
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
*/


/*
 * Put the system defined include files required.
 */
#include "IxOsal.h"

/*
 * Put the user defined include files required.
 */
#include "IxNpeDlImageMgr_p.h"
#include "IxNpeDlMacros_p.h"

/*
 * define the flag which toggles the firmare inclusion
 */
#define IX_NPE_MICROCODE_FIRMWARE_INCLUDED 1
#include "IxNpeMicrocode.h"

/*
 * Indicates the start of an NPE Image, in new NPE Image Library format.
 * 2 consecutive occurances indicates the end of the NPE Image Library
 */
#define NPE_IMAGE_MARKER 0xfeedf00d

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * TO BE DEPRECATED IN A FUTURE RELEASE
 */
typedef struct
{
    UINT32 size;
    UINT32 offset;
    UINT32 id;
} IxNpeDlImageMgrImageEntry;

/*
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * TO BE DEPRECATED IN A FUTURE RELEASE
 */
typedef union
{
    IxNpeDlImageMgrImageEntry image;
    UINT32 eohMarker;
} IxNpeDlImageMgrHeaderEntry;

/*
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * TO BE DEPRECATED IN A FUTURE RELEASE
 */
typedef struct
{
    UINT32 signature;
    /* 1st entry in the header (there may be more than one) */
    IxNpeDlImageMgrHeaderEntry entry[1];
} IxNpeDlImageMgrImageLibraryHeader;


/*
 * NPE Image Header definition, used in new NPE Image Library format
 */
typedef struct
{
    UINT32 marker;
    UINT32 id;
    UINT32 size;
} IxNpeDlImageMgrImageHeader;

/* module statistics counters */
typedef struct
{
    UINT32 invalidSignature;
    UINT32 imageIdListOverflow;
    UINT32 imageIdNotFound;
} IxNpeDlImageMgrStats;


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static IxNpeDlImageMgrStats ixNpeDlImageMgrStats;

/* default image */
#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE
static UINT32 *IxNpeMicroCodeImageLibrary = NULL;  /* Gets set to proper value at runtime */
#else
static UINT32 *IxNpeMicroCodeImageLibrary = (UINT32 *)IxNpeMicrocode_array;
#endif


/*
 * static function prototypes.
 */
PRIVATE BOOL
ixNpeDlImageMgrSignatureCheck (UINT32 *microCodeImageLibrary);

PRIVATE void  
ixNpeDlImageMgrImageIdFormat (UINT32 rawImageId, IxNpeDlImageId *imageId);

PRIVATE BOOL
ixNpeDlImageMgrImageIdCompare (IxNpeDlImageId *imageIdA, 
				 IxNpeDlImageId *imageIdB);
				 
PRIVATE BOOL
ixNpeDlImageMgrNpeFunctionIdCompare (IxNpeDlImageId *imageIdA,
    				       IxNpeDlImageId *imageIdB);

PRIVATE IX_STATUS
ixNpeDlImageMgrImageFind_legacy (UINT32 *imageLibrary,
                                 UINT32 imageId,
                                 UINT32 **imagePtr,
                                 UINT32 *imageSize);

/*
 * Function definition: ixNpeDlImageMgrMicrocodeImageLibraryOverride
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
IX_STATUS
ixNpeDlImageMgrMicrocodeImageLibraryOverride (
    UINT32 *clientImageLibrary)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlImageMgrMicrocodeImageLibraryOverride\n");

    if (ixNpeDlImageMgrSignatureCheck (clientImageLibrary))
    {
	IxNpeMicroCodeImageLibrary = clientImageLibrary;
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrMicrocodeImageLibraryOverride: "
			       "Client-supplied image has invalid signature\n");
	status = IX_FAIL;
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlImageMgrMicrocodeImageLibraryOverride: status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrImageListExtract
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
IX_STATUS
ixNpeDlImageMgrImageListExtract (
    IxNpeDlImageId *imageListPtr,
    UINT32 *numImages)
{
    UINT32 rawImageId;
    IxNpeDlImageId formattedImageId;
    IX_STATUS status = IX_SUCCESS;
    UINT32 imageCount = 0;
    IxNpeDlImageMgrImageLibraryHeader *header;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Entering ixNpeDlImageMgrImageListExtract\n");

    header = (IxNpeDlImageMgrImageLibraryHeader *) IxNpeMicroCodeImageLibrary;

    if (ixNpeDlImageMgrSignatureCheck (IxNpeMicroCodeImageLibrary))
    {
	/* for each image entry in the image header ... */
	while (header->entry[imageCount].eohMarker !=
	       IX_NPEDL_IMAGEMGR_END_OF_HEADER)
	{
	    /*
	     * if the image list container from calling function has capacity,
	     * add the image id to the list 
	     */
	    if ((imageListPtr != NULL) && (imageCount < *numImages))
	    {
		rawImageId = header->entry[imageCount].image.id;
	        ixNpeDlImageMgrImageIdFormat (rawImageId, &formattedImageId);
		imageListPtr[imageCount] = formattedImageId;
	    }
	    /* imageCount reflects no. of image entries in image library header */
	    imageCount++;  
	}
	
	/*
	 * if image list container from calling function was too small to
	 * contain all image ids in the header, set return status to FAIL
	 */
	if ((imageListPtr != NULL) && (imageCount > *numImages))
	{
	    status = IX_FAIL;
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageListExtract: "
				   "number of Ids found exceeds list capacity\n");
	    ixNpeDlImageMgrStats.imageIdListOverflow++;
	}
	/* return number of image ids found in image library header */
	*numImages = imageCount;  
    }
    else
    {
	status = IX_FAIL;
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageListExtract: "
			       "invalid signature in image\n");
    }
    
    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT, 
		     "Exiting ixNpeDlImageMgrImageListExtract: status = %d\n",
		     status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrImageLocate
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
IX_STATUS
ixNpeDlImageMgrImageLocate (
    IxNpeDlImageId *imageId,
    UINT32 **imagePtr,
    UINT32 *imageSize)
{
    UINT32 imageOffset;
    UINT32 rawImageId;
    IxNpeDlImageId formattedImageId;
    /* used to index image entries in image library header */
    UINT32 imageCount = 0;   
    IX_STATUS status = IX_FAIL;
    IxNpeDlImageMgrImageLibraryHeader *header;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlImageMgrImageLocate\n");

    header = (IxNpeDlImageMgrImageLibraryHeader *) IxNpeMicroCodeImageLibrary;

    if (ixNpeDlImageMgrSignatureCheck (IxNpeMicroCodeImageLibrary))
    {
	/* for each image entry in the image library header ... */
	while (header->entry[imageCount].eohMarker !=
	       IX_NPEDL_IMAGEMGR_END_OF_HEADER)
	{
	    rawImageId = header->entry[imageCount].image.id;
	    ixNpeDlImageMgrImageIdFormat (rawImageId, &formattedImageId);
	    /* if a match for imageId is found in the image library header... */
	    if (ixNpeDlImageMgrImageIdCompare (imageId, &formattedImageId))
	    {
		/*
		 * get pointer to the image in the image library using offset from
		 * 1st word in image library
		 */
		imageOffset = header->entry[imageCount].image.offset;
		*imagePtr = &IxNpeMicroCodeImageLibrary[imageOffset];
		/* get the image size */
		*imageSize = header->entry[imageCount].image.size;
		status = IX_SUCCESS;
		break;
	    }
	    imageCount++;
	}
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageLocate: "
				   "imageId not found in image library header\n");
	    ixNpeDlImageMgrStats.imageIdNotFound++;
	}
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageLocate: "
			       "invalid signature in image library\n");
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlImageMgrImageLocate: status = %d\n", status);
    return status;
}

/*
 * Function definition: ixNpeDlImageMgrLatestImageExtract
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
IX_STATUS
ixNpeDlImageMgrLatestImageExtract (IxNpeDlImageId *imageId)
{
    UINT32 imageCount = 0; 
    UINT32 rawImageId;
    IxNpeDlImageId formattedImageId;
    IX_STATUS status = IX_FAIL;
    IxNpeDlImageMgrImageLibraryHeader *header;
    
    
    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlImageMgrLatestImageExtract\n");
		     
    header = (IxNpeDlImageMgrImageLibraryHeader *) IxNpeMicroCodeImageLibrary;
    
    if (ixNpeDlImageMgrSignatureCheck (IxNpeMicroCodeImageLibrary))
    {
	/* for each image entry in the image library header ... */
	while (header->entry[imageCount].eohMarker !=
	       IX_NPEDL_IMAGEMGR_END_OF_HEADER)
	{
	    rawImageId = header->entry[imageCount].image.id;
	    ixNpeDlImageMgrImageIdFormat (rawImageId, &formattedImageId);
	    /* 
	     * if a match for the npe Id and functionality Id of the imageId is
	     *  found in the image library header... 
	     */
            if(ixNpeDlImageMgrNpeFunctionIdCompare(imageId, &formattedImageId))
            {
                if(imageId->major <= formattedImageId.major)
                {
                    if(imageId->minor < formattedImageId.minor)
                    {
                        imageId->minor = formattedImageId.minor;
                    }
                    imageId->major = formattedImageId.major;
                }
                status = IX_SUCCESS;
            }
            imageCount++;
	}
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrLatestImageExtract: "
				   "imageId not found in image library header\n");
	    ixNpeDlImageMgrStats.imageIdNotFound++;
	}
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrLatestImageGet: "
			       "invalid signature in image library\n");
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlImageMgrLatestImageGet: status = %d\n", status);
    return status;
}

/*
 * Function definition: ixNpeDlImageMgrSignatureCheck
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
PRIVATE BOOL
ixNpeDlImageMgrSignatureCheck (UINT32 *microCodeImageLibrary)
{
    IxNpeDlImageMgrImageLibraryHeader *header =
	(IxNpeDlImageMgrImageLibraryHeader *) microCodeImageLibrary;
    BOOL result = TRUE;

    if (header->signature != IX_NPEDL_IMAGEMGR_SIGNATURE)
    {
	result = FALSE;
	ixNpeDlImageMgrStats.invalidSignature++;
    }

    return result;
}


/*
 * Function definition: ixNpeDlImageMgrImageIdFormat
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
PRIVATE void
ixNpeDlImageMgrImageIdFormat (
    UINT32 rawImageId, 
    IxNpeDlImageId *imageId)
{  
    imageId->npeId = (rawImageId >>
				IX_NPEDL_IMAGEID_NPEID_OFFSET) &
	IX_NPEDL_NPEIMAGE_FIELD_MASK;
    imageId->functionalityId = (rawImageId >> 
				  IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET) &
	IX_NPEDL_NPEIMAGE_FIELD_MASK;
    imageId->major = (rawImageId >>
				IX_NPEDL_IMAGEID_MAJOR_OFFSET) &
	IX_NPEDL_NPEIMAGE_FIELD_MASK;
    imageId->minor = (rawImageId >>
				IX_NPEDL_IMAGEID_MINOR_OFFSET) &
	IX_NPEDL_NPEIMAGE_FIELD_MASK;

}


/*
 * Function definition: ixNpeDlImageMgrImageIdCompare
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
PRIVATE BOOL
ixNpeDlImageMgrImageIdCompare (
    IxNpeDlImageId *imageIdA,
    IxNpeDlImageId *imageIdB)
{
    if ((imageIdA->npeId   == imageIdB->npeId)   &&
	(imageIdA->functionalityId == imageIdB->functionalityId) &&
	(imageIdA->major   == imageIdB->major)   &&
	(imageIdA->minor   == imageIdB->minor))
    {
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}

/*
 * Function definition: ixNpeDlImageMgrNpeFunctionIdCompare
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
PRIVATE BOOL
ixNpeDlImageMgrNpeFunctionIdCompare (
    IxNpeDlImageId *imageIdA,
    IxNpeDlImageId *imageIdB)
{
    if ((imageIdA->npeId   == imageIdB->npeId)   &&
	(imageIdA->functionalityId == imageIdB->functionalityId))
    {
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}


/*
 * Function definition: ixNpeDlImageMgrStatsShow
 */
void
ixNpeDlImageMgrStatsShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\nixNpeDlImageMgrStatsShow:\n"
               "\tInvalid Image Signatures: %u\n"
               "\tImage Id List capacity too small: %u\n"
               "\tImage Id not found: %u\n\n",
               ixNpeDlImageMgrStats.invalidSignature,
               ixNpeDlImageMgrStats.imageIdListOverflow,
               ixNpeDlImageMgrStats.imageIdNotFound,
               0,0,0);
}


/*
 * Function definition: ixNpeDlImageMgrStatsReset
 */
void
ixNpeDlImageMgrStatsReset (void)
{
    ixNpeDlImageMgrStats.invalidSignature = 0;
    ixNpeDlImageMgrStats.imageIdListOverflow = 0;
    ixNpeDlImageMgrStats.imageIdNotFound = 0;
}


/*
 * Function definition: ixNpeDlImageMgrImageFind_legacy
 *
 * FOR BACKWARD-COMPATIBILITY WITH OLD NPE IMAGE LIBRARY FORMAT
 * AND/OR LEGACY API FUNCTIONS. TO BE DEPRECATED IN A FUTURE RELEASE
 */
PRIVATE IX_STATUS
ixNpeDlImageMgrImageFind_legacy (
    UINT32 *imageLibrary,
    UINT32 imageId,
    UINT32 **imagePtr,
    UINT32 *imageSize)
{
    UINT32 imageOffset;
    /* used to index image entries in image library header */
    UINT32 imageCount = 0;   
    IX_STATUS status = IX_FAIL;
    IxNpeDlImageMgrImageLibraryHeader *header;
    BOOL imageFound = FALSE;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Entering ixNpeDlImageMgrImageFind\n");


    /* If user didn't specify a library to use, use the default
     * one from IxNpeMicrocode.h
     */
    if (imageLibrary == NULL)
    {
	imageLibrary = IxNpeMicroCodeImageLibrary;
    }
    
    if (ixNpeDlImageMgrSignatureCheck (imageLibrary))
    {
	header = (IxNpeDlImageMgrImageLibraryHeader *) imageLibrary;
    
	/* for each image entry in the image library header ... */
	while ((header->entry[imageCount].eohMarker !=
               IX_NPEDL_IMAGEMGR_END_OF_HEADER) && !(imageFound))
	{
	    /* if a match for imageId is found in the image library header... */
	    if (imageId == header->entry[imageCount].image.id)
	    {
		/*
		 * get pointer to the image in the image library using offset from
		 * 1st word in image library
		 */
		imageOffset = header->entry[imageCount].image.offset;
		*imagePtr = &imageLibrary[imageOffset];
		/* get the image size */
		*imageSize = header->entry[imageCount].image.size;
		status = IX_SUCCESS;
		imageFound = TRUE;
	    }
	    imageCount++;
	}
	if (status != IX_SUCCESS)
	{
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
				   "imageId not found in image library header\n");
	    ixNpeDlImageMgrStats.imageIdNotFound++;
	}
    }
    else
    {
	IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
			       "invalid signature in image library\n");
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
		     "Exiting ixNpeDlImageMgrImageFind: status = %d\n", status);
    return status;
}


/*
 * Function definition: ixNpeDlImageMgrImageFind
 */
IX_STATUS
ixNpeDlImageMgrImageFind (
    UINT32 *imageLibrary,
    UINT32 imageId,
    UINT32 **imagePtr,
    UINT32 *imageSize)
{
    IxNpeDlImageMgrImageHeader *image;
    UINT32 offset = 0;

    /* If user didn't specify a library to use, use the default
     * one from IxNpeMicrocode.h
     */
    if (imageLibrary == NULL)
    {
#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE
	if (ixNpeMicrocode_binaryArray == NULL)
        {
	    printk (KERN_ERR "ixp400.o:  ERROR, no Microcode found in memory\n");
	    return IX_FAIL;
	}
	else
	{
	    imageLibrary = ixNpeMicrocode_binaryArray;
	}
#else
	imageLibrary = IxNpeMicroCodeImageLibrary;
#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */
    }

    /* For backward's compatibility with previous image format */
    if (ixNpeDlImageMgrSignatureCheck(imageLibrary))
    {
        return ixNpeDlImageMgrImageFind_legacy(imageLibrary,
                                               imageId,
                                               imagePtr,
                                               imageSize);
    }

    while (*(imageLibrary+offset) == NPE_IMAGE_MARKER)
    {
        image = (IxNpeDlImageMgrImageHeader *)(imageLibrary+offset);
        offset += sizeof(IxNpeDlImageMgrImageHeader)/sizeof(UINT32);
        
        if (image->id == imageId)
        {
            *imagePtr = imageLibrary + offset;
            *imageSize = image->size;
            return IX_SUCCESS;
        }
        /* 2 consecutive NPE_IMAGE_MARKER's indicates end of library */
        else if (image->id == NPE_IMAGE_MARKER)
        {
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
				   "imageId not found in image library header\n");
	    ixNpeDlImageMgrStats.imageIdNotFound++;
            /* reached end of library, image not found */
            return IX_FAIL;
        }
        offset += image->size;
    }

    /* If we get here, our image library may be corrupted */
    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
                           "image library format may be invalid or corrupted\n");
    return IX_FAIL;
}

