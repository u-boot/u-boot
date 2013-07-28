/**
 * @file IxNpeMhConfig_p.h
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the private API for the Configuration module.
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
 * @defgroup IxNpeMhConfig_p IxNpeMhConfig_p
 *
 * @brief The private API for the Configuration module.
 * 
 * @{
 */

#ifndef IXNPEMHCONFIG_P_H
#define IXNPEMHCONFIG_P_H

#include "IxOsal.h"

#include "IxNpeMh.h"
#include "IxNpeMhMacros_p.h"

/*
 * inline definition
 */
/* enable function inlining for performances */
#ifdef IXNPEMHSOLICITEDCBMGR_C
/* Non-inline functions will be defined in this translation unit. 
	 Reason is that in GNU Compiler, if the Optimization is turn off, all extern inline
	 functions will not be compiled.
*/
#		ifndef __wince
#			ifndef IXNPEMHCONFIG_INLINE
#				define IXNPEMHCONFIG_INLINE 
#			endif
#		else
#			ifndef IXNPEMHCONFIG_INLINE
#				define IXNPEMHCONFIG_INLINE IX_OSAL_INLINE_EXTERN
#			endif
#		endif /* __wince*/

#else

#		ifndef IXNPEMHCONFIG_INLINE
#			define IXNPEMHCONFIG_INLINE IX_OSAL_INLINE_EXTERN
#		endif /* IXNPEMHCONFIG_INLINE */
#endif /* IXNPEMHSOLICITEDCBMGR_C */
/*
 * Typedefs and #defines, etc.
 */
 
typedef void (*IxNpeMhConfigIsr) (int); /**< ISR function pointer */

/**
 * @struct IxNpeMhConfigNpeInfo
 *
 * @brief This structure is used to maintain the configuration information
 * associated with an NPE.
 */

typedef struct
{
    IxOsalMutex mutex;          /**< mutex */
    UINT32 interruptId;     /**< interrupt ID */
    UINT32 virtualRegisterBase; /**< register virtual base address */
    UINT32 statusRegister;      /**< status register virtual address */
    UINT32 controlRegister;     /**< control register virtual address */
    UINT32 inFifoRegister;      /**< inFIFO register virutal address */
    UINT32 outFifoRegister;     /**< outFIFO register virtual address */
    IxNpeMhConfigIsr isr;   /**< isr routine for handling interrupt */
    BOOL oldInterruptState; /**< old interrupt state (true => enabled) */
} IxNpeMhConfigNpeInfo;


/*
 * #defines for function return types, etc.
 */

/**< NPE register base address */
#define IX_NPEMH_NPE_BASE (IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE)

#define IX_NPEMH_NPEA_OFFSET (0x6000) /**< NPE-A register base offset */
#define IX_NPEMH_NPEB_OFFSET (0x7000) /**< NPE-B register base offset */
#define IX_NPEMH_NPEC_OFFSET (0x8000) /**< NPE-C register base offset */

#define IX_NPEMH_NPESTAT_OFFSET (0x002C) /**< NPE status register offset */
#define IX_NPEMH_NPECTL_OFFSET  (0x0030) /**< NPE control register offset */
#define IX_NPEMH_NPEFIFO_OFFSET (0x0038) /**< NPE FIFO register offset */

/** NPE-A register base address */
#define IX_NPEMH_NPEA_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEA_OFFSET)
/** NPE-B register base address */
#define IX_NPEMH_NPEB_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEB_OFFSET)
/** NPE-C register base address */
#define IX_NPEMH_NPEC_BASE (IX_NPEMH_NPE_BASE + IX_NPEMH_NPEC_OFFSET)

/* NPE-A configuration */

/** NPE-A interrupt */
#define IX_NPEMH_NPEA_INT  (IX_OSAL_IXP400_NPEA_IRQ_LVL)
/** NPE-A FIFO register */
#define IX_NPEMH_NPEA_FIFO (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-A control register */
#define IX_NPEMH_NPEA_CTL  (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-A status register */
#define IX_NPEMH_NPEA_STAT (IX_NPEMH_NPEA_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE-B configuration */

/** NPE-B interrupt */
#define IX_NPEMH_NPEB_INT  (IX_OSAL_IXP400_NPEB_IRQ_LVL)
/** NPE-B FIFO register */
#define IX_NPEMH_NPEB_FIFO (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-B control register */
#define IX_NPEMH_NPEB_CTL  (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-B status register */
#define IX_NPEMH_NPEB_STAT (IX_NPEMH_NPEB_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE-C configuration */

/** NPE-C interrupt */
#define IX_NPEMH_NPEC_INT  (IX_OSAL_IXP400_NPEC_IRQ_LVL)
/** NPE-C FIFO register */
#define IX_NPEMH_NPEC_FIFO (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPEFIFO_OFFSET)
/** NPE-C control register */
#define IX_NPEMH_NPEC_CTL  (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPECTL_OFFSET)
/** NPE-C status register */
#define IX_NPEMH_NPEC_STAT (IX_NPEMH_NPEC_BASE + IX_NPEMH_NPESTAT_OFFSET)

/* NPE control register bit definitions */
#define IX_NPEMH_NPE_CTL_OFE   (1 << 16) /**< OutFifoEnable */
#define IX_NPEMH_NPE_CTL_IFE   (1 << 17) /**< InFifoEnable */
#define IX_NPEMH_NPE_CTL_OFEWE (1 << 24) /**< OutFifoEnableWriteEnable */
#define IX_NPEMH_NPE_CTL_IFEWE (1 << 25) /**< InFifoEnableWriteEnable */

/* NPE status register bit definitions */
#define IX_NPEMH_NPE_STAT_OFNE  (1 << 16) /**< OutFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_IFNF  (1 << 17) /**< InFifoNotFull */
#define IX_NPEMH_NPE_STAT_OFNF  (1 << 18) /**< OutFifoNotFull */
#define IX_NPEMH_NPE_STAT_IFNE  (1 << 19) /**< InFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_MBINT (1 << 20) /**< Mailbox interrupt */
#define IX_NPEMH_NPE_STAT_IFINT (1 << 21) /**< InFifo interrupt */
#define IX_NPEMH_NPE_STAT_OFINT (1 << 22) /**< OutFifo interrupt */
#define IX_NPEMH_NPE_STAT_WFINT (1 << 23) /**< WatchFifo interrupt */


/**
 * Variable declarations.  Externs are followed by static variables.
 */
extern IxNpeMhConfigNpeInfo ixNpeMhConfigNpeInfo[IX_NPEMH_NUM_NPES];


/*
 * Prototypes for interface functions.
 */

/**
 * @fn void ixNpeMhConfigInitialize (
           IxNpeMhNpeInterrupts npeInterrupts)
 *
 * @brief This function initialises the Configuration module.
 *
 * @param IxNpeMhNpeInterrupts npeInterrupts (in) - whether or not to
 * service the NPE "outFIFO not empty" interrupts.
 *
 * @return No return value.
 */

void ixNpeMhConfigInitialize (
    IxNpeMhNpeInterrupts npeInterrupts);

/**
 * @fn void ixNpeMhConfigUninit (void)
 *
 * @brief This function uninitialises the Configuration module.
 *
 * @return No return value.
 */

void ixNpeMhConfigUninit (void);

/**
 * @fn void ixNpeMhConfigIsrRegister (
           IxNpeMhNpeId npeId,
           IxNpeMhConfigIsr isr)
 *
 * @brief This function registers an ISR to handle NPE "outFIFO not
 * empty" interrupts.
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be handled.
 * @param IxNpeMhConfigIsr isr (in) - the ISR function pointer that the
 * interrupt will trigger.
 *
 * @return No return value.
 */

void ixNpeMhConfigIsrRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhConfigIsr isr);

/**
 * @fn BOOL ixNpeMhConfigNpeInterruptEnable (
           IxNpeMhNpeId npeId)
 *
 * @brief This function enables a NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be enabled.
 *
 * @return Returns the previous state of the interrupt (true => enabled).
 */

BOOL ixNpeMhConfigNpeInterruptEnable (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigNpeInterruptDisable (
           IxNpeMhNpeId npeId)
 *
 * @brief This function disables a NPE's "outFIFO not empty" interrupt
 *
 * @param IxNpeMhNpeId npeId (in) - the ID of the NPE whose interrupt will
 * be disabled.
 *
 * @return Returns the previous state of the interrupt (true => enabled).
 */

BOOL ixNpeMhConfigNpeInterruptDisable (
    IxNpeMhNpeId npeId);

/**
 * @fn IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
           IxNpeMhMessage message)
 *
 * @brief This function gets the ID of a message.
 *
 * @param IxNpeMhMessage message (in) - the message to get the ID of.
 *
 * @return the ID of the message
 */

IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
    IxNpeMhMessage message);

/**
 * @fn BOOL ixNpeMhConfigNpeIdIsValid (
           IxNpeMhNpeId npeId)
 *
 * @brief This function checks to see if a NPE ID is valid.
 *
 * @param IxNpeMhNpeId npeId (in) - the NPE ID to validate.
 *
 * @return true if the NPE ID is valid, otherwise false.
 */

BOOL ixNpeMhConfigNpeIdIsValid (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigLockGet (
           IxNpeMhNpeId npeId)
 *
 * @brief This function gets a lock for exclusive NPE interaction, and
 * disables the NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which to get the
 * lock and disable its interrupt.
 *
 * @return No return value.
 */

void ixNpeMhConfigLockGet (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigLockRelease (
           IxNpeMhNpeId npeId)
 *
 * @brief This function releases a lock for exclusive NPE interaction, and
 * enables the NPE's "outFIFO not empty" interrupt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which to release
 * the lock and enable its interrupt.
 *
 * @return No return value.
 */

void ixNpeMhConfigLockRelease (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigInFifoIsEmpty (
           IxNpeMhNpeId npeId)
 *
 * @brief This inline function checks if a NPE's inFIFO is empty.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be checked.
 *
 * @return true if the inFIFO is empty, otherwise false.
 */

IXNPEMHCONFIG_INLINE BOOL ixNpeMhConfigInFifoIsEmpty (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigInFifoIsFull (
           IxNpeMhNpeId npeId)
 *
 * @brief This inline function checks if a NPE's inFIFO is full.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be checked.
 *
 * @return true if the inFIFO is full, otherwise false.
 */

IXNPEMHCONFIG_INLINE BOOL ixNpeMhConfigInFifoIsFull (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigOutFifoIsEmpty (
           IxNpeMhNpeId npeId)
 *
 * @brief This inline function checks if a NPE's outFIFO is empty.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be checked.
 *
 * @return true if the outFIFO is empty, otherwise false.
 */

IXNPEMHCONFIG_INLINE BOOL ixNpeMhConfigOutFifoIsEmpty (
    IxNpeMhNpeId npeId);

/**
 * @fn BOOL ixNpeMhConfigOutFifoIsFull (
           IxNpeMhNpeId npeId)
 *
 * @brief This inline function checks if a NPE's outFIFO is full.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be checked.
 *
 * @return true if the outFIFO is full, otherwise false.
 */

IXNPEMHCONFIG_INLINE BOOL ixNpeMhConfigOutFifoIsFull (
    IxNpeMhNpeId npeId);

/**
 * @fn IX_STATUS ixNpeMhConfigInFifoWrite (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage message)
 *
 * @brief This function writes a message to a NPE's inFIFO.  The caller
 * must first check that the NPE's inFifo is not full. After writing the first 
 * word of the message, this function will keep polling NPE's inFIFO is not
 * full to write the second word. If inFIFO is not available after maximum 
 * retries (IX_NPE_MH_MAX_NUM_OF_RETRIES), this function will return TIMEOUT 
 * status to indicate NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the inFIFO
 * will be written to.
 * @param IxNpeMhMessage message (in) - The message to write.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhConfigInFifoWrite (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message);

/**
 * @fn IX_STATUS ixNpeMhConfigOutFifoRead (
           IxNpeMhNpeId npeId,
           IxNpeMhMessage *message)
 *
 * @brief This function reads a message from a NPE's outFIFO.  The caller
 * must first check that the NPE's outFifo is not empty. After reading the first 
 * word of the message, this function will keep polling NPE's outFIFO is not
 * empty to read the second word. If outFIFO is empty after maximum 
 * retries (IX_NPE_MH_MAX_NUM_OF_RETRIES), this function will return TIMEOUT 
 * status to indicate NPE hang / halt.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE for which the outFIFO
 * will be read from.
 * @param IxNpeMhMessage message (out) - The message read.
 *
 * @return The function returns a status indicating success, failure or timeout.
 */

IX_STATUS ixNpeMhConfigOutFifoRead (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage *message);

/**
 * @fn void ixNpeMhConfigShow (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will display the current state of the Configuration
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to display state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhConfigShow (
    IxNpeMhNpeId npeId);

/**
 * @fn void ixNpeMhConfigShowReset (
           IxNpeMhNpeId npeId)
 *
 * @brief This function will reset the current state of the Configuration
 * module.
 *
 * @param IxNpeMhNpeId npeId (in) - The ID of the NPE to reset state
 * information for.
 *
 * @return No return value.
 */

void ixNpeMhConfigShowReset (
    IxNpeMhNpeId npeId);

/*
 * Inline functions
 */
 
/*
 * This inline function checks if a NPE's inFIFO is empty.
 */

IXNPEMHCONFIG_INLINE
BOOL ixNpeMhConfigInFifoIsEmpty (
    IxNpeMhNpeId npeId)
{
    UINT32 ifne;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the IFNE (InFifoNotEmpty) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ifne, IX_NPEMH_NPE_STAT_IFNE);

    /* if the IFNE status bit is unset then the inFIFO is empty */
    return (ifne == 0);
}


/*
 * This inline function checks if a NPE's inFIFO is full.
 */
IXNPEMHCONFIG_INLINE
BOOL ixNpeMhConfigInFifoIsFull (
    IxNpeMhNpeId npeId)
{
    UINT32 ifnf;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the IFNF (InFifoNotFull) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ifnf, IX_NPEMH_NPE_STAT_IFNF);

    /* if the IFNF status bit is unset then the inFIFO is full */
    return (ifnf == 0);
}


/*
 * This inline function checks if a NPE's outFIFO is empty.
 */
IXNPEMHCONFIG_INLINE
BOOL ixNpeMhConfigOutFifoIsEmpty (
    IxNpeMhNpeId npeId)
{
    UINT32 ofne;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the OFNE (OutFifoNotEmpty) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofne, IX_NPEMH_NPE_STAT_OFNE);

    /* if the OFNE status bit is unset then the outFIFO is empty */
    return (ofne == 0);
}

/*
 * This inline function checks if a NPE's outFIFO is full.
 */
IXNPEMHCONFIG_INLINE
BOOL ixNpeMhConfigOutFifoIsFull (
    IxNpeMhNpeId npeId)
{
    UINT32 ofnf;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    /* get the OFNF (OutFifoNotFull) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofnf, IX_NPEMH_NPE_STAT_OFNF);

    /* if the OFNF status bit is unset then the outFIFO is full */
    return (ofnf == 0);
}

#endif /* IXNPEMHCONFIG_P_H */

/**
 * @} defgroup IxNpeMhConfig_p
 */
