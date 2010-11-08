/**
 * @file IxNpeDl.c
 *
 * @author Intel Corporation
 * @date 08 January 2002
 *
 * @brief This file contains the implementation of the public API for the
 *        IXP425 NPE Downloader component
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
 * Put the system defined include files required
 */

/*
 * Put the user defined include files required
 */
#include "IxNpeDl.h"
#include "IxNpeDlImageMgr_p.h"
#include "IxNpeDlNpeMgr_p.h"
#include "IxNpeDlMacros_p.h"
#include "IxFeatureCtrl.h"
#include "IxOsal.h"
/*
 * #defines used in this file
 */
 #define IMAGEID_MAJOR_NUMBER_DEFAULT 0
 #define IMAGEID_MINOR_NUMBER_DEFAULT 0

/*
 * Typedefs whose scope is limited to this file.
 */
typedef struct
{
    BOOL validImage;
    IxNpeDlImageId imageId;
} IxNpeDlNpeState;

/* module statistics counters */
typedef struct
{
    UINT32 attemptedDownloads;
    UINT32 successfulDownloads;
    UINT32 criticalFailDownloads;
} IxNpeDlStats;

/*
 * Variable declarations global to this file only.  Externs are followed
 * by static variables.
 */
static IxNpeDlNpeState ixNpeDlNpeState[IX_NPEDL_NPEID_MAX] =
{
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}},
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}},
    {FALSE, {IX_NPEDL_NPEID_MAX, 0, 0, 0}}
};

static IxNpeDlStats ixNpeDlStats;

/*
 * Software guard to prevent NPE from being started multiple times.
 */
static BOOL ixNpeDlNpeStarted[IX_NPEDL_NPEID_MAX] ={FALSE, FALSE, FALSE} ;


/*
 * static function prototypes.
 */
PRIVATE IX_STATUS
ixNpeDlNpeInitAndStartInternal (UINT32 *imageLibrary, UINT32 imageId);

/*
 * Function definition: ixNpeDlImageDownload
 */
PUBLIC IX_STATUS
ixNpeDlImageDownload (IxNpeDlImageId *imageIdPtr,
                      BOOL verify)
{
    UINT32        imageSize;
    UINT32       *imageCodePtr  = NULL;
    IX_STATUS     status;
    IxNpeDlNpeId  npeId           = imageIdPtr->npeId;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlImageDownload\n");

    ixNpeDlStats.attemptedDownloads++;

    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlImageDownload - invalid parameter\n");
    }
    else
    {
        /* Ensure initialisation has been completed */
        ixNpeDlNpeMgrInit();

	/* If not IXP42X A0 stepping, proceed to check for existence of npe's */
	if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	     (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	    || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
	{
            if (npeId == IX_NPEDL_NPEID_NPEA)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE A component you specified does"
                                            " not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of if(npeId) */
            else if (npeId == IX_NPEDL_NPEID_NPEB)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE B component you specified"
                                            " does not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of elseif(npeId) */
            else if (npeId == IX_NPEDL_NPEID_NPEC)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE C component you specified"
                                            " does not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of elseif(npeId) */
        } /* end of if(IX_FEATURE_CTRL_SILICON_TYPE_B0) */ /*End of Silicon Type Check*/

        /* stop and reset the NPE */
        if (IX_SUCCESS != ixNpeDlNpeStopAndReset (npeId))
        {
            IX_NPEDL_ERROR_REPORT ("Failed to stop and reset NPE\n");
            return IX_FAIL;
        }

        /* Locate image */
        status = ixNpeDlImageMgrImageLocate (imageIdPtr, &imageCodePtr,
                                             &imageSize);
        if (IX_SUCCESS == status)
        {
            /*
             * If download was successful, store image Id in list of
             * currently loaded images. If a critical error occured
             * during download, record that the NPE has an invalid image
             */
            status = ixNpeDlNpeMgrImageLoad (npeId, imageCodePtr,
                                             verify);
            if (IX_SUCCESS == status)
            {
                ixNpeDlNpeState[npeId].imageId = *imageIdPtr;
                ixNpeDlNpeState[npeId].validImage = TRUE;
                ixNpeDlStats.successfulDownloads++;

                status =  ixNpeDlNpeExecutionStart (npeId);
            }
            else if ((status == IX_NPEDL_CRITICAL_NPE_ERR) ||
                     (status == IX_NPEDL_CRITICAL_MICROCODE_ERR))
            {
                ixNpeDlNpeState[npeId].imageId = *imageIdPtr;
                ixNpeDlNpeState[npeId].validImage = FALSE;
                ixNpeDlStats.criticalFailDownloads++;
            }
        } /* end of if(IX_SUCCESS) */ /* condition: image located successfully in microcode image */
    } /* end of if-else(npeId) */ /* condition: parameter checks ok */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlImageDownload : status = %d\n", status);
    return status;
}

/*
 * Function definition: ixNpeDlAvailableImagesCountGet
 */
PUBLIC IX_STATUS
ixNpeDlAvailableImagesCountGet (UINT32 *numImagesPtr)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlAvailableImagesCountGet\n");

    /* Check input parameters */
    if (numImagesPtr == NULL)
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlAvailableImagesCountGet - "
                               "invalid parameter\n");
    }
    else
    {
        /*
         * Use ImageMgr module to get no. of images listed in Image Library Header.
         * If NULL is passed as imageListPtr parameter to following function,
         * it will only fill number of images into numImagesPtr
         */
        status = ixNpeDlImageMgrImageListExtract (NULL, numImagesPtr);
    } /* end of if-else(numImagesPtr) */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlAvailableImagesCountGet : "
                     "status = %d\n", status);
    return status;
}

/*
 * Function definition: ixNpeDlAvailableImagesListGet
 */
PUBLIC IX_STATUS
ixNpeDlAvailableImagesListGet (IxNpeDlImageId *imageIdListPtr,
                               UINT32 *listSizePtr)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlAvailableImagesListGet\n");

    /* Check input parameters */
    if ((imageIdListPtr == NULL) || (listSizePtr == NULL))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlAvailableImagesListGet - "
                               "invalid parameter\n");
    }
    else
    {
        /* Call ImageMgr to get list of images listed in Image Library Header */
        status = ixNpeDlImageMgrImageListExtract (imageIdListPtr,
                                                  listSizePtr);
    } /* end of if-else(imageIdListPtr) */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlAvailableImagesListGet : status = %d\n",
                     status);
    return status;
}

/*
 * Function definition: ixNpeDlLoadedImageGet
 */
PUBLIC IX_STATUS
ixNpeDlLoadedImageGet (IxNpeDlNpeId npeId,
                       IxNpeDlImageId *imageIdPtr)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlLoadedImageGet\n");

    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0) || (imageIdPtr == NULL))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedImageGet - invalid parameter\n");
    }
    else
    {

         /* If not IXP42X A0 stepping, proceed to check for existence of npe's */
         if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	      (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	     || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
        {
            if (npeId == IX_NPEDL_NPEID_NPEA &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE A component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */

            if (npeId == IX_NPEDL_NPEID_NPEB &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE B component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */

            if (npeId == IX_NPEDL_NPEID_NPEC &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE C component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */
        } /* end of if not IXP42x-A0 silicon */

        if (ixNpeDlNpeState[npeId].validImage)
        {
            /* use npeId to get imageId from list of currently loaded
               images */
            *imageIdPtr = ixNpeDlNpeState[npeId].imageId;
        }
        else
        {
            status = IX_FAIL;
        } /* end of if-else(ixNpeDlNpeState) */
    } /* end of if-else(npeId) */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlLoadedImageGet : status = %d\n",
                     status);
    return status;
}

/*
 * Function definition: ixNpeDlLatestImageGet
 */
PUBLIC IX_STATUS
ixNpeDlLatestImageGet (
    IxNpeDlNpeId npeId,
    IxNpeDlFunctionalityId functionalityId,
    IxNpeDlImageId *imageIdPtr)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlLatestImageGet\n");

    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) ||
        (npeId < 0) ||
        (imageIdPtr == NULL))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlLatestImageGet - "
                               "invalid parameter\n");
    } /* end of if(npeId) */
    else
    {

	/* If not IXP42X A0 stepping, proceed to check for existence of npe's */
	if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	     (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	    || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
        {
            if (npeId == IX_NPEDL_NPEID_NPEA &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE A component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */

            if (npeId == IX_NPEDL_NPEID_NPEB &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE B component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */

            if (npeId == IX_NPEDL_NPEID_NPEC &&
                (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC) ==
                 IX_FEATURE_CTRL_COMPONENT_DISABLED))
            {
                IX_NPEDL_WARNING_REPORT("Warning: the NPE C component you specified does"
                                        " not exist\n");
                return IX_SUCCESS;
            } /* end of if(npeId) */
        } /* end of if not IXP42x-A0 silicon */

        imageIdPtr->npeId = npeId;
        imageIdPtr->functionalityId = functionalityId;
        imageIdPtr->major = IMAGEID_MAJOR_NUMBER_DEFAULT;
        imageIdPtr->minor = IMAGEID_MINOR_NUMBER_DEFAULT;
        /* Call ImageMgr to get list of images listed in Image Library Header */
        status = ixNpeDlImageMgrLatestImageExtract(imageIdPtr);
    } /* end of if-else(npeId) */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlLatestImageGet : status = %d\n",
                     status);

    return status;
}

/*
 * Function definition: ixNpeDlNpeStopAndReset
 */
PUBLIC IX_STATUS
ixNpeDlNpeStopAndReset (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlNpeStopAndReset\n");

    /* Ensure initialisation has been completed */
    ixNpeDlNpeMgrInit();

    /* If not IXP42X A0 stepping, proceed to check for existence of npe's */
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	 (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	|| (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
        /*
         * Check whether NPE is present
         */
        if (IX_NPEDL_NPEID_NPEA == npeId)
        {
            /* Check whether NPE A is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE A does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeStopAndReset - Warning:NPEA does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of if(IX_NPEDL_NPEID_NPEA) */
        else if (IX_NPEDL_NPEID_NPEB == npeId)
        {
            /* Check whether NPE B is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE B does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeStopAndReset - Warning:NPEB does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEB) */
        else if (IX_NPEDL_NPEID_NPEC == npeId)
        {
            /* Check whether NPE C is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE C does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeStopAndReset - Warning:NPEC does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEC) */
        else
        {
            /* Invalid NPE ID */
            IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeStopAndReset - invalid Npe ID\n");
            status = IX_NPEDL_PARAM_ERR;
        } /* end of if-else(IX_NPEDL_NPEID_NPEC) */
    } /* end of if not IXP42x-A0 Silicon */

    if (status == IX_SUCCESS)
    {
        /* call NpeMgr function to stop the NPE */
        status = ixNpeDlNpeMgrNpeStop (npeId);
        if (status == IX_SUCCESS)
        {
            /* call NpeMgr function to reset the NPE */
            status = ixNpeDlNpeMgrNpeReset (npeId);
        }
    } /* end of if(status) */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlNpeStopAndReset : status = %d\n", status);

    if (IX_SUCCESS == status)
    {
        /* Indicate NPE has been stopped */
        ixNpeDlNpeStarted[npeId] = FALSE ;
    }

    return status;
}

/*
 * Function definition: ixNpeDlNpeExecutionStart
 */
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStart (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlNpeExecutionStart\n");

    /* If not IXP42X A0 stepping, proceed to check for existence of npe's */
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	 (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	|| (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
        /*
         * Check whether NPE is present
         */
        if (IX_NPEDL_NPEID_NPEA == npeId)
        {
            /* Check whether NPE A is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE A does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStart - Warning:NPEA does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of if(IX_NPEDL_NPEID_NPEA) */
        else if (IX_NPEDL_NPEID_NPEB == npeId)
        {
            /* Check whether NPE B is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE B does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStart - Warning:NPEB does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEB) */
        else if (IX_NPEDL_NPEID_NPEC ==  npeId)
        {
            /* Check whether NPE C is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE C does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStart - Warning:NPEC does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEC) */
        else
        {
            /* Invalid NPE ID */
            IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeExecutionStart - invalid Npe ID\n");
            return IX_NPEDL_PARAM_ERR;
        } /* end of if-else(IX_NPEDL_NPEID_NPEC) */
    } /* end of if not IXP42x-A0 Silicon */

    if (TRUE == ixNpeDlNpeStarted[npeId])
    {
        /* NPE has been started. */
        return IX_SUCCESS ;
    }

    /* Ensure initialisation has been completed */
    ixNpeDlNpeMgrInit();

    /* call NpeMgr function to start the NPE */
    status = ixNpeDlNpeMgrNpeStart (npeId);

    if (IX_SUCCESS == status)
    {
        /* Indicate NPE has started */
        ixNpeDlNpeStarted[npeId] = TRUE ;
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlNpeExecutionStart : status = %d\n",
                     status);

    return status;
}

/*
 * Function definition: ixNpeDlNpeExecutionStop
 */
PUBLIC IX_STATUS
ixNpeDlNpeExecutionStop (IxNpeDlNpeId npeId)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlNpeExecutionStop\n");

    /* Ensure initialisation has been completed */
    ixNpeDlNpeMgrInit();

    /* If not IXP42X A0 stepping, proceed to check for existence of npe's */
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	 (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	|| (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
        /*
         * Check whether NPE is present
         */
        if (IX_NPEDL_NPEID_NPEA == npeId)
        {
            /* Check whether NPE A is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE A does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStop - Warning:NPEA does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of if(IX_NPEDL_NPEID_NPEA) */
        else if (IX_NPEDL_NPEID_NPEB == npeId)
        {
            /* Check whether NPE B is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE B does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStop - Warning:NPEB does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEB) */
        else if (IX_NPEDL_NPEID_NPEC == npeId)
        {
            /* Check whether NPE C is present */
            if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                IX_FEATURE_CTRL_COMPONENT_DISABLED)
            {
                /* NPE C does not present */
                IX_NPEDL_WARNING_REPORT ("ixNpeDlNpeExecutionStop - Warning:NPEC does not present.\n");
                return IX_SUCCESS;
            }
        } /* end of elseif(IX_NPEDL_NPEID_NPEC) */
        else
        {
            /* Invalid NPE ID */
            IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeExecutionStop - invalid Npe ID\n");
            status = IX_NPEDL_PARAM_ERR;
        } /* end of if-else(IX_NPEDL_NPEID_NPEC) */
    } /* end of if not IXP42X-AO Silicon */

    if (status == IX_SUCCESS)
    {
        /* call NpeMgr function to stop the NPE */
        status = ixNpeDlNpeMgrNpeStop (npeId);
    }

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlNpeExecutionStop : status = %d\n",
                     status);

    if (IX_SUCCESS == status)
    {
        /* Indicate NPE has been stopped */
        ixNpeDlNpeStarted[npeId] = FALSE ;
    }

    return status;
}

/*
 * Function definition: ixNpeDlUnload
 */
PUBLIC IX_STATUS
ixNpeDlUnload (void)
{
    IX_STATUS status;

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlUnload\n");

    status = ixNpeDlNpeMgrUninit();

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlUnload : status = %d\n",
                     status);
    return status;
}

/*
 * Function definition: ixNpeDlStatsShow
 */
PUBLIC void
ixNpeDlStatsShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\nixNpeDlStatsShow:\n"
               "\tDownloads Attempted by user: %u\n"
               "\tSuccessful Downloads: %u\n"
               "\tFailed Downloads (due to Critical Error): %u\n\n",
               ixNpeDlStats.attemptedDownloads,
               ixNpeDlStats.successfulDownloads,
               ixNpeDlStats.criticalFailDownloads,
               0,0,0);

    ixNpeDlImageMgrStatsShow ();
    ixNpeDlNpeMgrStatsShow ();
}

/*
 * Function definition: ixNpeDlStatsReset
 */
PUBLIC void
ixNpeDlStatsReset (void)
{
    ixNpeDlStats.attemptedDownloads = 0;
    ixNpeDlStats.successfulDownloads = 0;
    ixNpeDlStats.criticalFailDownloads = 0;

    ixNpeDlImageMgrStatsReset ();
    ixNpeDlNpeMgrStatsReset ();
}

/*
 * Function definition: ixNpeDlNpeInitAndStartInternal
 */
PRIVATE IX_STATUS
ixNpeDlNpeInitAndStartInternal (UINT32 *imageLibrary,
                                UINT32 imageId)
{
    UINT32        imageSize;
    UINT32       *imageCodePtr  = NULL;
    IX_STATUS     status;
    IxNpeDlNpeId  npeId = IX_NPEDL_NPEID_FROM_IMAGEID_GET(imageId);
    IxFeatureCtrlDeviceId deviceId = IX_NPEDL_DEVICEID_FROM_IMAGEID_GET(imageId);

    IX_NPEDL_TRACE0 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Entering ixNpeDlNpeInitAndStartInternal\n");

    ixNpeDlStats.attemptedDownloads++;

    /* Check input parameter device correctness */
    if ((deviceId >= IX_FEATURE_CTRL_DEVICE_TYPE_MAX) ||
        (deviceId < IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeInitAndStartInternal - "
                               "invalid parameter\n");
    } /* End valid device id checking */

    /* Check input parameters */
    else if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeInitAndStartInternal - "
                               "invalid parameter\n");
    }

    else
    {
        /* Ensure initialisation has been completed */
        ixNpeDlNpeMgrInit();

        /* Checking if image being loaded is meant for device that is running.
         * Image is forward compatible. i.e Image built for IXP42X should run
         * on IXP46X but not vice versa.*/
        if (deviceId > (ixFeatureCtrlDeviceRead() & IX_FEATURE_CTRL_DEVICE_TYPE_MASK))
        {
            IX_NPEDL_ERROR_REPORT ("ixNpeDlNpeInitAndStartInternal - "
                                   "Device type mismatch. NPE Image not "
                                   "meant for device in use \n");
            return IX_NPEDL_DEVICE_ERR;
        }/* if statement - matching image device and current device */

	/* If not IXP42X A0 stepping, proceed to check for existence of npe's */
	if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	     (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	    || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
        {
            if (npeId == IX_NPEDL_NPEID_NPEA)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA) ==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE A component you specified does"
                                            " not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of if(npeId) */
            else if (npeId == IX_NPEDL_NPEID_NPEB)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEB)==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE B component you specified"
                                            " does not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of elseif(npeId) */
            else if (npeId == IX_NPEDL_NPEID_NPEC)
            {
                if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEC)==
                    IX_FEATURE_CTRL_COMPONENT_DISABLED)
                {
                    IX_NPEDL_WARNING_REPORT("Warning: the NPE C component you specified"
                                            " does not exist\n");
                    return IX_SUCCESS;
                }
            } /* end of elseif(npeId) */
        } /* end of if not IXP42X-A0 Silicon */

        /* stop and reset the NPE */
        status = ixNpeDlNpeStopAndReset (npeId);
        if (IX_SUCCESS != status)
        {
            IX_NPEDL_ERROR_REPORT ("Failed to stop and reset NPE\n");
            return status;
        }

        /* Locate image */
        status = ixNpeDlImageMgrImageFind (imageLibrary, imageId,
                                           &imageCodePtr, &imageSize);
        if (IX_SUCCESS == status)
        {
            /*
             * If download was successful, store image Id in list of
             * currently loaded images. If a critical error occured
             * during download, record that the NPE has an invalid image
             */
            status = ixNpeDlNpeMgrImageLoad (npeId, imageCodePtr, TRUE);
            if (IX_SUCCESS == status)
            {
                ixNpeDlNpeState[npeId].validImage = TRUE;
                ixNpeDlStats.successfulDownloads++;

                status = ixNpeDlNpeExecutionStart (npeId);
            }
            else if ((status == IX_NPEDL_CRITICAL_NPE_ERR) ||
                     (status == IX_NPEDL_CRITICAL_MICROCODE_ERR))
            {
                ixNpeDlNpeState[npeId].validImage = FALSE;
                ixNpeDlStats.criticalFailDownloads++;
            }

            /* NOTE - The following section of code is here to support
             * a deprecated function ixNpeDlLoadedImageGet().  When that
             * function is removed from the API, this code should be revised.
             */
            ixNpeDlNpeState[npeId].imageId.npeId = npeId;
            ixNpeDlNpeState[npeId].imageId.functionalityId =
                IX_NPEDL_FUNCTIONID_FROM_IMAGEID_GET(imageId);
            ixNpeDlNpeState[npeId].imageId.major =
                IX_NPEDL_MAJOR_FROM_IMAGEID_GET(imageId);
            ixNpeDlNpeState[npeId].imageId.minor =
                IX_NPEDL_MINOR_FROM_IMAGEID_GET(imageId);
        } /* end of if(IX_SUCCESS) */ /* condition: image located successfully in microcode image */
    } /* end of if-else(npeId-deviceId) */ /* condition: parameter checks ok */

    IX_NPEDL_TRACE1 (IX_NPEDL_FN_ENTRY_EXIT,
                     "Exiting ixNpeDlNpeInitAndStartInternal : "
                     "status = %d\n", status);
    return status;
}

/*
 * Function definition: ixNpeDlCustomImageNpeInitAndStart
 */
PUBLIC IX_STATUS
ixNpeDlCustomImageNpeInitAndStart (UINT32 *imageLibrary,
                                   UINT32 imageId)
{
    IX_STATUS status;

    if (imageLibrary == NULL)
    {
        status = IX_NPEDL_PARAM_ERR;
        IX_NPEDL_ERROR_REPORT ("ixNpeDlCustomImageNpeInitAndStart "
                               "- invalid parameter\n");
    }
    else
    {
        status = ixNpeDlNpeInitAndStartInternal (imageLibrary, imageId);
    } /* end of if-else(imageLibrary) */

    return status;
}

/*
 * Function definition: ixNpeDlNpeInitAndStart
 */
PUBLIC IX_STATUS
ixNpeDlNpeInitAndStart (UINT32 imageId)
{
    return ixNpeDlNpeInitAndStartInternal (NULL, imageId);
}

/*
 * Function definition: ixNpeDlLoadedImageFunctionalityGet
 */
PUBLIC IX_STATUS
ixNpeDlLoadedImageFunctionalityGet (IxNpeDlNpeId npeId,
                                    UINT8 *functionalityId)
{
    /* Check input parameters */
    if ((npeId >= IX_NPEDL_NPEID_MAX) || (npeId < 0))
    {
        IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedImageFunctionalityGet "
                               "- invalid parameter\n");
        return IX_NPEDL_PARAM_ERR;
    }
    if (functionalityId == NULL)
    {
        IX_NPEDL_ERROR_REPORT ("ixNpeDlLoadedImageFunctionalityGet "
                               "- invalid parameter\n");
        return IX_NPEDL_PARAM_ERR;
    }

    if (ixNpeDlNpeState[npeId].validImage)
    {
        *functionalityId = ixNpeDlNpeState[npeId].imageId.functionalityId;
        return IX_SUCCESS;
    }
    else
    {
        return IX_FAIL;
    }
}
