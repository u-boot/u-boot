/**
 * @file IxEthAccMii.c
 *
 * @author Intel Corporation
 * @date 
 *
 * @brief  MII control functions
 *
 * Design Notes:
 *
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
#include "IxEthAcc.h"
#include "IxEthAcc_p.h"
#include "IxEthAccMac_p.h"
#include "IxEthAccMii_p.h"


PRIVATE UINT32 miiBaseAddressVirt;
PRIVATE IxOsalMutex miiAccessLock;

PUBLIC UINT32 ixEthAccMiiRetryCount    = IX_ETH_ACC_MII_TIMEOUT_10TH_SECS;
PUBLIC UINT32 ixEthAccMiiAccessTimeout = IX_ETH_ACC_MII_10TH_SEC_IN_MILLIS;

/* -----------------------------------
 * private function prototypes
 */
PRIVATE void
ixEthAccMdioCmdWrite(UINT32 mdioCommand);

PRIVATE void
ixEthAccMdioCmdRead(UINT32 *data);

PRIVATE void
ixEthAccMdioStatusRead(UINT32 *data);


PRIVATE void
ixEthAccMdioCmdWrite(UINT32 mdioCommand)
{
    REG_WRITE(miiBaseAddressVirt,
	      IX_ETH_ACC_MAC_MDIO_CMD_1,
	      mdioCommand & 0xff);

    REG_WRITE(miiBaseAddressVirt,
	      IX_ETH_ACC_MAC_MDIO_CMD_2,
	      (mdioCommand >> 8) & 0xff);

    REG_WRITE(miiBaseAddressVirt,
	      IX_ETH_ACC_MAC_MDIO_CMD_3,
	      (mdioCommand >> 16) & 0xff);

    REG_WRITE(miiBaseAddressVirt,
	      IX_ETH_ACC_MAC_MDIO_CMD_4,
	      (mdioCommand >> 24) & 0xff);
}

PRIVATE void
ixEthAccMdioCmdRead(UINT32 *data)
{
    UINT32 regval;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_CMD_1,
	     regval);

    *data = regval & 0xff;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_CMD_2,
	     regval);

    *data |= (regval & 0xff) << 8;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_CMD_3,
	     regval);

    *data |= (regval & 0xff) << 16;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_CMD_4,
	     regval);

    *data |= (regval & 0xff) << 24;

}

PRIVATE void
ixEthAccMdioStatusRead(UINT32 *data)
{
    UINT32 regval;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_STS_1,
	     regval);

    *data = regval & 0xff;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_STS_2,
	     regval);

    *data |= (regval & 0xff) << 8;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_STS_3,
	     regval);

    *data |= (regval & 0xff) << 16;

    REG_READ(miiBaseAddressVirt,
	     IX_ETH_ACC_MAC_MDIO_STS_4,
	     regval);
    
    *data |= (regval & 0xff) << 24;
    
}


/********************************************************************
 * ixEthAccMiiInit
 */
IxEthAccStatus
ixEthAccMiiInit()
{
    if(ixOsalMutexInit(&miiAccessLock)!= IX_SUCCESS)
    {
	return IX_ETH_ACC_FAIL;
    }

    miiBaseAddressVirt = (UINT32) IX_OSAL_MEM_MAP(IX_ETH_ACC_MAC_0_BASE, IX_OSAL_IXP400_ETHA_MAP_SIZE);
    
    if (miiBaseAddressVirt == 0)
    {
      ixOsalLog(IX_OSAL_LOG_LVL_FATAL, 
		IX_OSAL_LOG_DEV_STDOUT, 
		"EthAcc: Could not map MII I/O mapped memory\n", 
		0, 0, 0, 0, 0, 0);
      
      return IX_ETH_ACC_FAIL;
    }
    
    return IX_ETH_ACC_SUCCESS;
}

void
ixEthAccMiiUnload(void)
{
    IX_OSAL_MEM_UNMAP(miiBaseAddressVirt);
  
    miiBaseAddressVirt = 0;
}

PUBLIC IxEthAccStatus
ixEthAccMiiAccessTimeoutSet(UINT32 timeout, UINT32 retryCount)
{
    if (retryCount < 1) return IX_ETH_ACC_FAIL;

    ixEthAccMiiRetryCount    = retryCount;
    ixEthAccMiiAccessTimeout = timeout;

    return IX_ETH_ACC_SUCCESS;
}

/*********************************************************************
 * ixEthAccMiiReadRtn - read a 16 bit value from a PHY
 */
IxEthAccStatus      
ixEthAccMiiReadRtn (UINT8 phyAddr, 
		    UINT8 phyReg, 
		    UINT16 *value)
{
    UINT32 mdioCommand;
    UINT32 regval;
    UINT32 miiTimeout;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if ((phyAddr >= IXP425_ETH_ACC_MII_MAX_ADDR) 
	|| (phyReg >= IXP425_ETH_ACC_MII_MAX_REG))
    {
	return (IX_ETH_ACC_FAIL);
    }

    if (value == NULL)
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&miiAccessLock, IX_OSAL_WAIT_FOREVER);
    mdioCommand = phyReg <<  IX_ETH_ACC_MII_REG_SHL 
	| phyAddr << IX_ETH_ACC_MII_ADDR_SHL;
    mdioCommand |= IX_ETH_ACC_MII_GO;

    ixEthAccMdioCmdWrite(mdioCommand);
    
    miiTimeout = ixEthAccMiiRetryCount;

    while(miiTimeout)
    {
	
	ixEthAccMdioCmdRead(&regval);
     
	if((regval & IX_ETH_ACC_MII_GO) == 0x0)
	{	    
	    break;
	}
	/* Sleep for a while */
	ixOsalSleep(ixEthAccMiiAccessTimeout);
	miiTimeout--;
    }
    

    
    if(miiTimeout == 0)
    {	
	ixOsalMutexUnlock(&miiAccessLock);
	*value = 0xffff;
	return IX_ETH_ACC_FAIL;
    }
    
    
    ixEthAccMdioStatusRead(&regval);
    if(regval & IX_ETH_ACC_MII_READ_FAIL)
    {
	ixOsalMutexUnlock(&miiAccessLock);
	*value = 0xffff;
	return IX_ETH_ACC_FAIL;
    }

    *value = regval & 0xffff;
    ixOsalMutexUnlock(&miiAccessLock);
    return IX_ETH_ACC_SUCCESS;
    
}


/*********************************************************************
 * ixEthAccMiiWriteRtn - write a 16 bit value to a PHY
 */
IxEthAccStatus
ixEthAccMiiWriteRtn (UINT8 phyAddr, 
		     UINT8 phyReg, 
		     UINT16 value)
{
    UINT32 mdioCommand;
    UINT32 regval;
    UINT16 readVal;
    UINT32 miiTimeout;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    if ((phyAddr >= IXP425_ETH_ACC_MII_MAX_ADDR) 
	|| (phyReg >= IXP425_ETH_ACC_MII_MAX_REG))
    {
	return (IX_ETH_ACC_FAIL);
    }
   
    /* ensure that a PHY is present at this address */
    if(ixEthAccMiiReadRtn(phyAddr,
                          IX_ETH_ACC_MII_CTRL_REG,
                          &readVal) != IX_ETH_ACC_SUCCESS)
    {
        return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&miiAccessLock, IX_OSAL_WAIT_FOREVER);
    mdioCommand = phyReg << IX_ETH_ACC_MII_REG_SHL
	| phyAddr << IX_ETH_ACC_MII_ADDR_SHL ;
    mdioCommand |= IX_ETH_ACC_MII_GO | IX_ETH_ACC_MII_WRITE | value;

    ixEthAccMdioCmdWrite(mdioCommand);
    
    miiTimeout = ixEthAccMiiRetryCount;

    while(miiTimeout)
    {
	
	ixEthAccMdioCmdRead(&regval);

	/*The "GO" bit is reset to 0 when the write completes*/
	if((regval & IX_ETH_ACC_MII_GO) == 0x0)
	{		    
	    break;
	}
	/* Sleep for a while */
	ixOsalSleep(ixEthAccMiiAccessTimeout);
        miiTimeout--;
    }
    
    ixOsalMutexUnlock(&miiAccessLock);
    if(miiTimeout == 0)
    {
	return IX_ETH_ACC_FAIL;
    }
    return IX_ETH_ACC_SUCCESS;
}


/*****************************************************************
 *
 *  Phy query functions
 *
 */
IxEthAccStatus
ixEthAccMiiStatsShow (UINT32 phyAddr)
{
    UINT16 regval;
    printf("Regisers on PHY at address 0x%x\n", phyAddr);
    ixEthAccMiiReadRtn(phyAddr, IX_ETH_ACC_MII_CTRL_REG, &regval);
    printf("    Control Register                  :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_STAT_REG, &regval);
    printf("    Status Register                   :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_PHY_ID1_REG, &regval);
    printf("    PHY ID1 Register                  :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_PHY_ID2_REG, &regval);
    printf("    PHY ID2 Register                  :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_ADS_REG, &regval);
    printf("    Auto Neg ADS Register             :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_PRTN_REG, &regval);
    printf("    Auto Neg Partner Ability Register :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_EXP_REG, &regval);
    printf("    Auto Neg Expansion Register       :      0x%4.4x\n", regval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_ACC_MII_AN_NEXT_REG, &regval);
    printf("    Auto Neg Next Register            :      0x%4.4x\n", regval);

    return IX_ETH_ACC_SUCCESS;
}


/*****************************************************************
 *
 *  Interface query functions
 *
 */
IxEthAccStatus
ixEthAccMdioShow (void)
{
    UINT32 regval;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }

    ixOsalMutexLock(&miiAccessLock, IX_OSAL_WAIT_FOREVER);
    ixEthAccMdioCmdRead(&regval);
    ixOsalMutexUnlock(&miiAccessLock);
    
    printf("MDIO command register\n");
    printf("    Go bit      : 0x%x\n", (regval & BIT(31)) >> 31);
    printf("    MDIO Write  : 0x%x\n", (regval & BIT(26)) >> 26);
    printf("    PHY address : 0x%x\n", (regval >> 21) & 0x1f);
    printf("    Reg address : 0x%x\n", (regval >> 16) & 0x1f);
	
    ixOsalMutexLock(&miiAccessLock, IX_OSAL_WAIT_FOREVER);
    ixEthAccMdioStatusRead(&regval);
    ixOsalMutexUnlock(&miiAccessLock);

    printf("MDIO status register\n");
    printf("    Read OK     : 0x%x\n", (regval & BIT(31)) >> 31);
    printf("    Read Data   : 0x%x\n", (regval >> 16) & 0xff);

    return IX_ETH_ACC_SUCCESS;   
}

