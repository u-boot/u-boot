/**
 * @file IxEthMii.c
 *
 * @author Intel Corporation
 * @date
 *
 * @brief  MII control functions
 *
 * Design Notes:
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

#include "IxOsal.h"

#include "IxEthAcc.h"
#include "IxEthMii_p.h"

#ifdef __wince
#include "IxOsPrintf.h"
#endif

/* Array to store the phy IDs of the discovered phys */
PRIVATE UINT32 ixEthMiiPhyId[IXP425_ETH_ACC_MII_MAX_ADDR];

/*********************************************************
 *
 * Scan for PHYs on the MII bus. This function returns
 * an array of booleans, one for each PHY address.
 * If a PHY is found at a particular address, the
 * corresponding entry in the array is set to TRUE.
 *
 */

PUBLIC IX_STATUS
ixEthMiiPhyScan(BOOL phyPresent[], UINT32 maxPhyCount)
{
    UINT32 i;
    UINT16 regval, regvalId1, regvalId2;

    /*Search for PHYs on the MII*/
    /*Search for existant phys on the MDIO bus*/

    if ((phyPresent == NULL) || 
	(maxPhyCount > IXP425_ETH_ACC_MII_MAX_ADDR))
    {
	return IX_FAIL;
    }

    /* fill the array */
    for(i=0;
        i<IXP425_ETH_ACC_MII_MAX_ADDR;
	i++)
    {
	phyPresent[i] = FALSE;
    }

    /* iterate through the PHY addresses */
    for(i=0;
	maxPhyCount > 0 && i<IXP425_ETH_ACC_MII_MAX_ADDR;
	i++)
    {
	ixEthMiiPhyId[i] = IX_ETH_MII_INVALID_PHY_ID;
	if(ixEthAccMiiReadRtn(i,
			      IX_ETH_MII_CTRL_REG,
			      &regval) == IX_ETH_ACC_SUCCESS)
	{
	    if((regval & 0xffff) != 0xffff)
	    {
		maxPhyCount--;
		/*Need to read the register twice here to flush PHY*/
		ixEthAccMiiReadRtn(i,  IX_ETH_MII_PHY_ID1_REG, &regvalId1);
		ixEthAccMiiReadRtn(i,  IX_ETH_MII_PHY_ID1_REG, &regvalId1);
		ixEthAccMiiReadRtn(i,  IX_ETH_MII_PHY_ID2_REG, &regvalId2);
		ixEthMiiPhyId[i] = (regvalId1 << IX_ETH_MII_REG_SHL) | regvalId2;
		if ((ixEthMiiPhyId[i] == IX_ETH_MII_KS8995_PHY_ID)
		    || (ixEthMiiPhyId[i] == IX_ETH_MII_LXT971_PHY_ID)
		    || (ixEthMiiPhyId[i] == IX_ETH_MII_LXT972_PHY_ID)
		    || (ixEthMiiPhyId[i] == IX_ETH_MII_LXT973_PHY_ID)
		    || (ixEthMiiPhyId[i] == IX_ETH_MII_LXT973A3_PHY_ID)
		    || (ixEthMiiPhyId[i] == IX_ETH_MII_LXT9785_PHY_ID)
		    )
		{
		    /* supported phy */
		    phyPresent[i] = TRUE;
		} /* end of if(ixEthMiiPhyId) */
		else
		{
		    if (ixEthMiiPhyId[i] != IX_ETH_MII_INVALID_PHY_ID)
		    {
			/* unsupported phy */
                        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                                   IX_OSAL_LOG_DEV_STDOUT,
				    "ixEthMiiPhyScan : unexpected Mii PHY ID %8.8x\n", 
				    ixEthMiiPhyId[i], 2, 3, 4, 5, 6);
			ixEthMiiPhyId[i] = IX_ETH_MII_UNKNOWN_PHY_ID;
			phyPresent[i] = TRUE;
		    }
		} 
	    }
	}
    }
    return IX_SUCCESS;
}

/************************************************************
 *
 * Configure the PHY at the specified address
 *
 */
PUBLIC IX_STATUS
ixEthMiiPhyConfig(UINT32 phyAddr,
		  BOOL speed100,
		  BOOL fullDuplex,
		  BOOL autonegotiate)
{
    UINT16 regval=0;

    /* parameter check */
    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
    /*
     * set the control register
     */
	if(autonegotiate)
	{
	    regval |= IX_ETH_MII_CR_AUTO_EN | IX_ETH_MII_CR_RESTART;
	}
	else
	{
	    if(speed100)
	    {
		regval |= IX_ETH_MII_CR_100;
	    }
	    if(fullDuplex)
	    {
		regval |= IX_ETH_MII_CR_FDX;
	    }
	} /* end of if-else() */
	if (ixEthAccMiiWriteRtn(phyAddr, 
				IX_ETH_MII_CTRL_REG, 
				regval) == IX_ETH_ACC_SUCCESS)
	{
	    return IX_SUCCESS;
	}
    } /* end of if(phyAddr) */
    return IX_FAIL;
}

/******************************************************************
 *
 *  Enable the PHY Loopback at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyLoopbackEnable (UINT32 phyAddr)
{
  UINT16 regval ;  

  if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) && 
      (IX_ETH_MII_INVALID_PHY_ID != ixEthMiiPhyId[phyAddr]))
  {
      /* read/write the control register */
      if(ixEthAccMiiReadRtn (phyAddr,
			     IX_ETH_MII_CTRL_REG, 
			     &regval) 
	 == IX_ETH_ACC_SUCCESS)
      {
	  if(ixEthAccMiiWriteRtn (phyAddr, 
				  IX_ETH_MII_CTRL_REG, 
				  regval | IX_ETH_MII_CR_LOOPBACK)
	     == IX_ETH_ACC_SUCCESS)
	  {
	      return IX_SUCCESS;
	  }
      }
  }
  return IX_FAIL;
}

/******************************************************************
 *
 *  Disable the PHY Loopback at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyLoopbackDisable (UINT32 phyAddr)
{
  UINT16 regval ;  

  if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) && 
      (IX_ETH_MII_INVALID_PHY_ID != ixEthMiiPhyId[phyAddr]))
  {
      /* read/write the control register */
      if(ixEthAccMiiReadRtn (phyAddr,
			     IX_ETH_MII_CTRL_REG, 
			     &regval) 
	 == IX_ETH_ACC_SUCCESS)
      {
	  if(ixEthAccMiiWriteRtn (phyAddr, 
				  IX_ETH_MII_CTRL_REG, 
				  regval & (~IX_ETH_MII_CR_LOOPBACK))
	     == IX_ETH_ACC_SUCCESS)
	  {
	      return IX_SUCCESS;
	  }
      }
  }
  return IX_FAIL;
}

/******************************************************************
 *
 *  Reset the PHY at the specified address
 */
PUBLIC IX_STATUS
ixEthMiiPhyReset(UINT32 phyAddr)
{
    UINT32 timeout;
    UINT16 regval;

    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
	if ((ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT971_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT972_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT973_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT973A3_PHY_ID)	||
		(ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT9785_PHY_ID)
	    )
	{
	    /* use the control register to reset the phy */
	    ixEthAccMiiWriteRtn(phyAddr, 
				IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_RESET);
	 
	    /* poll until the reset bit is cleared */
	    timeout = 0;
	    do
	    {
		ixOsalSleep (IX_ETH_MII_RESET_POLL_MS);

		/* read the control register and check for timeout */
		ixEthAccMiiReadRtn(phyAddr, 
				   IX_ETH_MII_CTRL_REG,
				   &regval);
		if ((regval & IX_ETH_MII_CR_RESET) == 0)
		{
		    /* timeout bit is self-cleared */
		    break;
		}
		timeout += IX_ETH_MII_RESET_POLL_MS;
	    }
	    while (timeout < IX_ETH_MII_RESET_DELAY_MS);

	    /* check for timeout */
	    if (timeout >= IX_ETH_MII_RESET_DELAY_MS)
	    {
		ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				    IX_ETH_MII_CR_NORM_EN);
		return IX_FAIL;
	    }

	    return IX_SUCCESS;
	} /* end of if(ixEthMiiPhyId) */
	else if (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_KS8995_PHY_ID)
	{
	    /* reset bit is reserved, just reset the control register */
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_NORM_EN);
	    return IX_SUCCESS;
	}
	else
	{
	    /* unknown PHY, set the control register reset bit,
	     * wait 2 s. and clear the control register.
	     */
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_RESET);
	    
	    ixOsalSleep (IX_ETH_MII_RESET_DELAY_MS);
	    
	    ixEthAccMiiWriteRtn(phyAddr, IX_ETH_MII_CTRL_REG,
				IX_ETH_MII_CR_NORM_EN);
	    return IX_SUCCESS;
	} /* end of if-else(ixEthMiiPhyId) */
    } /* end of if(phyAddr) */
    return IX_FAIL;
}

/*****************************************************************
 *
 *  Link state query functions
 */

PUBLIC IX_STATUS
ixEthMiiLinkStatus(UINT32 phyAddr,
           BOOL *linkUp,
           BOOL *speed100,
           BOOL *fullDuplex,
           BOOL *autoneg)
{
    UINT16 ctrlRegval, statRegval, regval, regval4, regval5;

    /* check the parameters */
    if ((linkUp == NULL) || 
	(speed100 == NULL) || 
	(fullDuplex == NULL) ||
	(autoneg == NULL))
    {
	return IX_FAIL;
    }

    *linkUp = FALSE;
    *speed100 = FALSE;
    *fullDuplex = FALSE;
    *autoneg = FALSE;

    if ((phyAddr < IXP425_ETH_ACC_MII_MAX_ADDR) &&
	(ixEthMiiPhyId[phyAddr] != IX_ETH_MII_INVALID_PHY_ID))
    {
	if ((ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT971_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT972_PHY_ID)	||
	    (ixEthMiiPhyId[phyAddr] == IX_ETH_MII_LXT9785_PHY_ID)
		)
	{
	    /* --------------------------------------------------*/
	    /* Retrieve information from PHY specific register   */
	    /* --------------------------------------------------*/
	    if (ixEthAccMiiReadRtn(phyAddr, 
				   IX_ETH_MII_STAT2_REG, 
				   &regval) != IX_ETH_ACC_SUCCESS)
	    {
		return IX_FAIL;
	    }
	    *linkUp = ((regval & IX_ETH_MII_SR2_LINK) != 0);
	    *speed100 = ((regval & IX_ETH_MII_SR2_100) != 0);
	    *fullDuplex = ((regval & IX_ETH_MII_SR2_FD) != 0);
	    *autoneg = ((regval & IX_ETH_MII_SR2_AUTO) != 0);
	    return IX_SUCCESS;
	} /* end of if(ixEthMiiPhyId) */
	else
	{    
	    /* ----------------------------------------------------*/
	    /* Retrieve information from status and ctrl registers */
	    /* ----------------------------------------------------*/
	    if (ixEthAccMiiReadRtn(phyAddr,  
				   IX_ETH_MII_CTRL_REG, 
				   &ctrlRegval) != IX_ETH_ACC_SUCCESS)
	    {
		return IX_FAIL;
	    }
	    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_STAT_REG, &statRegval);
	    
	    *linkUp = ((statRegval & IX_ETH_MII_SR_LINK_STATUS) != 0);
	    if (*linkUp)
	    {
		*autoneg = ((ctrlRegval & IX_ETH_MII_CR_AUTO_EN) != 0) &&
		    ((statRegval &  IX_ETH_MII_SR_AUTO_SEL) != 0) &&
		    ((statRegval & IX_ETH_MII_SR_AUTO_NEG) != 0);
		
		if (*autoneg)
		{
		    /* mask the current stat values with the capabilities */
		    ixEthAccMiiReadRtn(phyAddr, IX_ETH_MII_AN_ADS_REG, &regval4);
		    ixEthAccMiiReadRtn(phyAddr, IX_ETH_MII_AN_PRTN_REG, &regval5);
		    /* merge the flags from the 3 registers */
		    regval = (statRegval & ((regval4 & regval5) << 6));
		    /* initialise from status register values */
		    if ((regval & IX_ETH_MII_SR_TX_FULL_DPX) != 0)
		    {
			/* 100 Base X full dplx */
			*speed100 = TRUE;
			*fullDuplex = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_TX_HALF_DPX) != 0)
		    {
			/* 100 Base X half dplx */
			*speed100 = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_10T_FULL_DPX) != 0)
		    {
			/* 10 mb full dplx */
			*fullDuplex = TRUE;
			return IX_SUCCESS;
		    }
		    if ((regval & IX_ETH_MII_SR_10T_HALF_DPX) != 0)
		    {
			/* 10 mb half dplx */
			return IX_SUCCESS;
		    }
		} /* end of if(autoneg) */
		else
		{
		    /* autonegotiate not complete, return setup parameters */
		    *speed100 = ((ctrlRegval & IX_ETH_MII_CR_100) != 0);
		    *fullDuplex = ((ctrlRegval & IX_ETH_MII_CR_FDX) != 0);
		}
	    } /* end of if(linkUp) */
	} /* end of if-else(ixEthMiiPhyId) */
    } /* end of if(phyAddr) */
    else
    {
	return IX_FAIL;
    } /* end of if-else(phyAddr) */
    return IX_SUCCESS;
}

/*****************************************************************
 *
 *  Link state display functions
 */

PUBLIC IX_STATUS
ixEthMiiPhyShow (UINT32 phyAddr)
{
    BOOL linkUp, speed100, fullDuplex, autoneg;
    UINT16 cregval;
    UINT16 sregval;
    

    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_STAT_REG, &sregval);
    ixEthAccMiiReadRtn(phyAddr,  IX_ETH_MII_CTRL_REG, &cregval);

    /* get link information */
    if (ixEthMiiLinkStatus(phyAddr,
			   &linkUp,
			   &speed100,
			   &fullDuplex,
			   &autoneg) != IX_ETH_ACC_SUCCESS)
    {
	printf("PHY Status unknown\n");
	return IX_FAIL;
    }

    printf("PHY ID [phyAddr]: %8.8x\n",ixEthMiiPhyId[phyAddr]);
    printf( " Status reg:  %4.4x\n",sregval);
    printf( " control reg: %4.4x\n",cregval);
    /* display link information */
    printf("PHY Status:\n");
    printf("    Link is %s\n",
	   (linkUp ? "Up" : "Down"));
    if((sregval & IX_ETH_MII_SR_REMOTE_FAULT) != 0)
    {
	printf("    Remote fault detected\n");
    }
    printf("    Auto Negotiation %s\n",
	   (autoneg ? "Completed" : "Not Completed"));

    printf("PHY Configuration:\n");
    printf("    Speed %sMb/s\n",
	   (speed100 ? "100" : "10"));
    printf("    %s Duplex\n",
	   (fullDuplex ? "Full" : "Half"));
    printf("    Auto Negotiation %s\n",
	   (autoneg ? "Enabled" : "Disabled"));
    return IX_SUCCESS;
}

