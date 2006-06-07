/**
 * @file IxEthDBPortDefs.h
 *
 * @brief Public definition of the ports and port capabilities
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

/**
 * @defgroup IxEthDBPortDefs IXP400 Ethernet Database Port Definitions (IxEthDBPortDefs)
 *
 * @brief IXP400 Public definition of the ports and port capabilities
 *
 * @{
 */

#ifndef IxEthDBPortDefs_H
#define IxEthDBPortDefs_H

/** 
 * @brief Port types - currently only Ethernet NPEs are recognized as specific types 
 * All other (user-defined) ports must be specified as IX_ETH_GENERIC
 */
typedef enum
{
    IX_ETH_GENERIC = 0, /**< generic ethernet port */
    IX_ETH_NPE          /**< specific Ethernet NPE */
} IxEthDBPortType;

/** 
 * @brief Port capabilities - used by ixEthAccDatabaseMaintenance to decide whether it
 * should manually age entries or not depending on the port capabilities.
 *
 * Ethernet NPEs have aging capabilities, meaning that they will age the entries
 * automatically (by themselves).*/
typedef enum
{
    IX_ETH_NO_CAPABILITIES = 0,   /**< no aging capabilities */
    IX_ETH_ENTRY_AGING     = 0x1  /**< aging capabilities present */
} IxEthDBPortCapability;

/** 
 * @brief Port Definition - a structure contains the Port type and capabilities 
 */
typedef struct
{
    IxEthDBPortType type;
    IxEthDBPortCapability capabilities;
} IxEthDBPortDefinition;

/** 
 * @brief Port definitions structure, indexed on the port ID
 * @warning Ports 0 and 1 are used by the Ethernet access component therefore
 * it is essential to be left untouched. Port 2 here (WAN) is given as 
 * an example port. The NPE firmware also assumes the NPE B to be
 * the port 0 and NPE C to be the port 1.
 *
 * @note that only 32 ports (0..31) are supported by EthDB
 */
static const IxEthDBPortDefinition ixEthDBPortDefinitions[] = 
{
    /*    id       type              capabilities */
    {   /* 0 */    IX_ETH_NPE,       IX_ETH_NO_CAPABILITIES },    /* Ethernet NPE B */
    {   /* 1 */    IX_ETH_NPE,       IX_ETH_NO_CAPABILITIES },    /* Ethernet NPE C */
    {   /* 2 */    IX_ETH_NPE,       IX_ETH_NO_CAPABILITIES },    /* Ethernet NPE A */
    {   /* 3 */    IX_ETH_GENERIC,   IX_ETH_NO_CAPABILITIES },    /* WAN port */
};

/** 
 * @def IX_ETH_DB_NUMBER_OF_PORTS
 * @brief number of supported ports 
 */
#define IX_ETH_DB_NUMBER_OF_PORTS (sizeof (ixEthDBPortDefinitions) / sizeof (ixEthDBPortDefinitions[0]))

/**
 * @def IX_ETH_DB_UNKNOWN_PORT
 * @brief definition of an unknown port
 */
#define IX_ETH_DB_UNKNOWN_PORT (0xff)

/** 
 * @def IX_ETH_DB_ALL_PORTS
 * @brief Special port ID indicating all the ports
 * @note This port ID can be used only by a subset of the EthDB API; each
 * function specifically mentions whether this is a valid parameter as the port ID
 */
#define IX_ETH_DB_ALL_PORTS (IX_ETH_DB_NUMBER_OF_PORTS + 1)

/**
 * @def IX_ETH_DB_PORTS_ASSERTION
 * @brief catch invalid port definitions (<2) with a 
 * compile-time assertion resulting in a duplicate case error. 
 */
#define IX_ETH_DB_PORTS_ASSERTION { switch(0) { case 0 : ; case 1 : ; case IX_ETH_DB_NUMBER_OF_PORTS : ; }}

/** 
 * @def IX_ETH_DB_CHECK_PORT(portID)
 * @brief safety checks to verify whether the port is invalid or uninitialized
 */
#define IX_ETH_DB_CHECK_PORT(portID) \
{ \
  if ((portID) >= IX_ETH_DB_NUMBER_OF_PORTS) \
  { \
      return IX_ETH_DB_INVALID_PORT; \
  } \
  \
  if (!ixEthDBPortInfo[(portID)].enabled) \
  { \
      return IX_ETH_DB_PORT_UNINITIALIZED; \
  } \
}

/**
 * @def IX_ETH_DB_CHECK_PORT_ALL(portID)
 * @brief safety checks to verify whether the port is invalid or uninitialized; 
 * tolerates the use of IX_ETH_DB_ALL_PORTS
 */
#define IX_ETH_DB_CHECK_PORT_ALL(portID) \
{ \
  if ((portID) != IX_ETH_DB_ALL_PORTS) \
    IX_ETH_DB_CHECK_PORT(portID) \
}

#endif /* IxEthDBPortDefs_H */
/**
 *@}
 */
