/**
 * @file IxFeatureCtrl.c
 *
 * @author Intel Corporation
 * @date 29-Jan-2003
 *
 * @brief Feature Control Public API Implementation
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

#include "IxOsal.h"
#include "IxVersionId.h"
#include "IxFeatureCtrl.h"

/* Macro to read from the Feature Control Register */
#define IX_FEATURE_CTRL_READ(result) \
do { \
ixFeatureCtrlExpMap(); \
(result) = IX_OSAL_READ_LONG(ixFeatureCtrlRegister); \
} while (0)

/* Macro to write to the Feature Control Register */
#define IX_FEATURE_CTRL_WRITE(value) \
do { \
ixFeatureCtrlExpMap(); \
IX_OSAL_WRITE_LONG(ixFeatureCtrlRegister, (value)); \
} while (0)

/*
 * This is the offset of the feature register relative to the base of the
 * Expansion Bus Controller MMR.
 */
#define IX_FEATURE_CTRL_REG_OFFSET (0x00000028)


/* Boolean to mark the fact that the EXP_CONFIG address space was mapped */
PRIVATE BOOL ixFeatureCtrlExpCfgRegionMapped = false;

/* Pointer holding the virtual address of the Feature Control Register */
PRIVATE VUINT32 *ixFeatureCtrlRegister = NULL;

/* Place holder to store the software configuration */
PRIVATE BOOL swConfiguration[IX_FEATURECTRL_SWCONFIG_MAX];

/* Flag to control swConfiguration[] is initialized once */
PRIVATE BOOL swConfigurationFlag = false ;

/* Array containing component mask values */
#ifdef __ixp42X
UINT32 componentMask[IX_FEATURECTRL_MAX_COMPONENTS] = {
    (0x1<<IX_FEATURECTRL_RCOMP),
    (0x1<<IX_FEATURECTRL_USB),
    (0x1<<IX_FEATURECTRL_HASH),
    (0x1<<IX_FEATURECTRL_AES),
    (0x1<<IX_FEATURECTRL_DES),
    (0x1<<IX_FEATURECTRL_HDLC),
    (0x1<<IX_FEATURECTRL_AAL),
    (0x1<<IX_FEATURECTRL_HSS),
    (0x1<<IX_FEATURECTRL_UTOPIA),
    (0x1<<IX_FEATURECTRL_ETH0),
    (0x1<<IX_FEATURECTRL_ETH1),
    (0x1<<IX_FEATURECTRL_NPEA),
    (0x1<<IX_FEATURECTRL_NPEB),
    (0x1<<IX_FEATURECTRL_NPEC),
    (0x1<<IX_FEATURECTRL_PCI),
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE,
    (0x3<<IX_FEATURECTRL_UTOPIA_PHY_LIMIT),
    (0x1<<IX_FEATURECTRL_UTOPIA_PHY_LIMIT_BIT2),
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE,
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE,
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE,
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE,
    IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE
};
#elif defined (__ixp46X)
UINT32 componentMask[IX_FEATURECTRL_MAX_COMPONENTS] = {
    (0x1<<IX_FEATURECTRL_RCOMP),
    (0x1<<IX_FEATURECTRL_USB),
    (0x1<<IX_FEATURECTRL_HASH),
    (0x1<<IX_FEATURECTRL_AES),
    (0x1<<IX_FEATURECTRL_DES),
    (0x1<<IX_FEATURECTRL_HDLC),
    IX_FEATURECTRL_COMPONENT_ALWAYS_AVAILABLE,  /* AAL component is always on */
    (0x1<<IX_FEATURECTRL_HSS),
    (0x1<<IX_FEATURECTRL_UTOPIA),
    (0x1<<IX_FEATURECTRL_ETH0),
    (0x1<<IX_FEATURECTRL_ETH1),
    (0x1<<IX_FEATURECTRL_NPEA),
    (0x1<<IX_FEATURECTRL_NPEB),
    (0x1<<IX_FEATURECTRL_NPEC),
    (0x1<<IX_FEATURECTRL_PCI),
    (0x1<<IX_FEATURECTRL_ECC_TIMESYNC),
    (0x3<<IX_FEATURECTRL_UTOPIA_PHY_LIMIT),
    (0x1<<IX_FEATURECTRL_UTOPIA_PHY_LIMIT_BIT2), /* NOT TO BE USED */
    (0x1<<IX_FEATURECTRL_USB_HOST_CONTROLLER),
    (0x1<<IX_FEATURECTRL_NPEA_ETH),
    (0x1<<IX_FEATURECTRL_NPEB_ETH),
    (0x1<<IX_FEATURECTRL_RSA),
    (0x3<<IX_FEATURECTRL_XSCALE_MAX_FREQ),
    (0x1<<IX_FEATURECTRL_XSCALE_MAX_FREQ_BIT2)
};
#endif /* __ixp42X */

/**
 * Forward declaration
 */
PRIVATE
void ixFeatureCtrlExpMap(void);

PRIVATE 
void ixFeatureCtrlSwConfigurationInit(void);

/**
 * Function to map EXP_CONFIG space
 */
PRIVATE
void ixFeatureCtrlExpMap(void)
{
    UINT32 expCfgBaseAddress = 0;

    /* If the EXP Configuration space has already been mapped then
     * return */
    if (ixFeatureCtrlExpCfgRegionMapped == true)
    {
	return;
    }

    /* Map (get virtual address) for the EXP_CONFIG space */
    expCfgBaseAddress = (UINT32)
	(IX_OSAL_MEM_MAP(IX_OSAL_IXP400_EXP_BUS_REGS_PHYS_BASE,
			   IX_OSAL_IXP400_EXP_REG_MAP_SIZE));

    /* Assert that the mapping operation succeeded */
    IX_OSAL_ASSERT(expCfgBaseAddress);

    /* Set the address of the Feature register */
    ixFeatureCtrlRegister =
	(VUINT32 *) (expCfgBaseAddress + IX_FEATURE_CTRL_REG_OFFSET);

    /* Mark the fact that the EXP_CONFIG space has already been mapped */
    ixFeatureCtrlExpCfgRegionMapped = true;
}

/**
 * Function definition: ixFeatureCtrlSwConfigurationInit
 * This function will only initialize software configuration once.
 */
PRIVATE void ixFeatureCtrlSwConfigurationInit(void)
{
  UINT32 i;
  if (false == swConfigurationFlag)
  {
    for (i=0; i<IX_FEATURECTRL_SWCONFIG_MAX ; i++)
    {
        /* By default, all software configuration are enabled */
        swConfiguration[i]= true ;
    }
    /*Make sure this function only initializes swConfiguration[] once*/
    swConfigurationFlag = true ;
  }  
}

/**
 * Function definition: ixFeatureCtrlRead
 */
IxFeatureCtrlReg 
ixFeatureCtrlRead (void)
{
    IxFeatureCtrlReg result;

#if CPU!=SIMSPARCSOLARIS
    /* Read the feature control register */
    IX_FEATURE_CTRL_READ(result);
    return result;
#else
    /* Return an invalid value for VxWorks simulation */
    result = 0xFFFFFFFF;
    return result;
#endif
}

/**
 * Function definition: ixFeatureCtrlWrite
 */
void
ixFeatureCtrlWrite (IxFeatureCtrlReg expUnitReg)
{
#if CPU!=SIMSPARCSOLARIS
    /* Write value to feature control register */
    IX_FEATURE_CTRL_WRITE(expUnitReg);
#endif
}


/**
 * Function definition: ixFeatureCtrlHwCapabilityRead
 */
IxFeatureCtrlReg
ixFeatureCtrlHwCapabilityRead (void)
{ 
  IxFeatureCtrlReg currentReg, hwCapability;
  
  /* Capture a copy of feature control register */
  currentReg = ixFeatureCtrlRead(); 

  /* Try to enable all hardware components. 
   * Only software disable hardware can be enabled again */
  ixFeatureCtrlWrite(0);
  
  /* Read feature control register to know the hardware capability. */ 
  hwCapability = ixFeatureCtrlRead();
     
  /* Restore initial feature control value */
  ixFeatureCtrlWrite(currentReg);

  /* return Hardware Capability */
  return hwCapability;  
}


/**
 * Function definition: ixFeatureCtrlComponentCheck
 */
IX_STATUS 
ixFeatureCtrlComponentCheck (IxFeatureCtrlComponentType componentType)
{
  IxFeatureCtrlReg expUnitReg; 
  UINT32 mask = 0;

  /* Lookup mask of component */
  mask=componentMask[componentType];

  /* Check if mask is available or not */
  if(IX_FEATURECTRL_COMPONENT_NOT_AVAILABLE == mask)
  {
      return IX_FEATURE_CTRL_COMPONENT_DISABLED;
  }

  if(IX_FEATURECTRL_COMPONENT_ALWAYS_AVAILABLE == mask)
  {
      return IX_FEATURE_CTRL_COMPONENT_ENABLED;
  }

  /* Read feature control register to know current hardware capability. */ 
  expUnitReg = ixFeatureCtrlRead();

  /* For example: To check for Hashing Coprocessor (bit-2) 
   *                   expUniteg    = 0x0010
   *                  ~expUnitReg   = 0x1101 
   *                  componentType = 0x0100
   *    ~expUnitReg & componentType = 0x0100 (Not zero)                      
   */
 
  /* 
   * Inverse the bit value because available component is 0 in value 
   */
  expUnitReg = ~expUnitReg ;

  if (expUnitReg & mask)
  {
     return (IX_FEATURE_CTRL_COMPONENT_ENABLED);
  }   
  else
  {  
     return (IX_FEATURE_CTRL_COMPONENT_DISABLED);
  } 
}


/**
 * Function definition: ixFeatureCtrlProductIdRead
 */
IxFeatureCtrlProductId
ixFeatureCtrlProductIdRead ()
{
#if CPU!=SIMSPARCSOLARIS
  IxFeatureCtrlProductId  pdId = 0 ;
   
  /* Use ARM instruction to move register0 from coprocessor to ARM register */ 
    
#ifndef __wince
    __asm__("mrc p15, 0, %0, cr0, cr0, 0;" : "=r"(pdId) :);
#else
      
#ifndef IN_KERNEL
        BOOL  mode;
#endif
    extern  IxFeatureCtrlProductId AsmixFeatureCtrlProductIdRead();
    
#ifndef IN_KERNEL
    mode = SetKMode(true);
#endif
    pdId = AsmixFeatureCtrlProductIdRead();
#ifndef IN_KERNEL
    SetKMode(mode);
#endif

#endif
  return (pdId);
#else
  /* Return an invalid value for VxWorks simulation */
  return 0xffffffff;
#endif
}

/**
 * Function definition: ixFeatureCtrlDeviceRead
 */
IxFeatureCtrlDeviceId
ixFeatureCtrlDeviceRead ()
{
  return ((ixFeatureCtrlProductIdRead() >> IX_FEATURE_CTRL_DEVICE_TYPE_OFFSET) 
             & IX_FEATURE_CTRL_DEVICE_TYPE_MASK);
} /* End function ixFeatureCtrlDeviceRead */


/**
 * Function definition: ixFeatureCtrlSwConfigurationCheck
 */
IX_STATUS
ixFeatureCtrlSwConfigurationCheck (IxFeatureCtrlSwConfig swConfigType)
{
  if (swConfigType >= IX_FEATURECTRL_SWCONFIG_MAX)  
  {
     ixOsalLog(IX_OSAL_LOG_LVL_WARNING, 
               IX_OSAL_LOG_DEV_STDOUT,
               "FeatureCtrl: Invalid software configuraiton input.\n",
               0, 0, 0, 0, 0, 0);  

     return IX_FEATURE_CTRL_SWCONFIG_DISABLED;
  }

  /* The function will only initialize once. */
  ixFeatureCtrlSwConfigurationInit();
  
  /* Check and return software configuration */
  return  ((swConfiguration[(UINT32)swConfigType] == true) ? IX_FEATURE_CTRL_SWCONFIG_ENABLED: IX_FEATURE_CTRL_SWCONFIG_DISABLED);
}

/**
 * Function definition: ixFeatureCtrlSwConfigurationWrite
 */
void
ixFeatureCtrlSwConfigurationWrite (IxFeatureCtrlSwConfig swConfigType, BOOL enabled)
{
  if (swConfigType >= IX_FEATURECTRL_SWCONFIG_MAX)  
  {
     ixOsalLog(IX_OSAL_LOG_LVL_WARNING, 
               IX_OSAL_LOG_DEV_STDOUT,
               "FeatureCtrl: Invalid software configuraiton input.\n",
               0, 0, 0, 0, 0, 0);  

     return;
  }

  /* The function will only initialize once. */
  ixFeatureCtrlSwConfigurationInit();
  
  /* Write software configuration */
  swConfiguration[(UINT32)swConfigType]=enabled ;
}

/**
 * Function definition: ixFeatureCtrlIxp400SwVersionShow
 */
void
ixFeatureCtrlIxp400SwVersionShow (void)
{
    printf ("\nIXP400 Software Release %s %s\n\n", IX_VERSION_ID, IX_VERSION_INTERNAL_ID);

}

/**
 * Function definition: ixFeatureCtrlSoftwareBuildGet
 */
IxFeatureCtrlBuildDevice
ixFeatureCtrlSoftwareBuildGet (void)
{
    #ifdef __ixp42X
    return IX_FEATURE_CTRL_SW_BUILD_IXP42X;
    #else
    return IX_FEATURE_CTRL_SW_BUILD_IXP46X;
    #endif
}
