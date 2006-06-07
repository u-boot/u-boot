/**
 * @file IxEthDBDBMem.c
 *
 * @brief Memory handling routines for the MAC address database
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


#include "IxEthDB_p.h"

IX_ETH_DB_PRIVATE HashNode *nodePool     = NULL;
IX_ETH_DB_PRIVATE MacDescriptor *macPool = NULL;
IX_ETH_DB_PRIVATE MacTreeNode *treePool  = NULL;

IX_ETH_DB_PRIVATE HashNode nodePoolArea[NODE_POOL_SIZE];
IX_ETH_DB_PRIVATE MacDescriptor macPoolArea[MAC_POOL_SIZE];
IX_ETH_DB_PRIVATE MacTreeNode treePoolArea[TREE_POOL_SIZE];

IX_ETH_DB_PRIVATE IxOsalMutex nodePoolLock;
IX_ETH_DB_PRIVATE IxOsalMutex macPoolLock;
IX_ETH_DB_PRIVATE IxOsalMutex treePoolLock;

#define LOCK_NODE_POOL   { ixOsalMutexLock(&nodePoolLock, IX_OSAL_WAIT_FOREVER); }
#define UNLOCK_NODE_POOL { ixOsalMutexUnlock(&nodePoolLock); }

#define LOCK_MAC_POOL    { ixOsalMutexLock(&macPoolLock, IX_OSAL_WAIT_FOREVER); }
#define UNLOCK_MAC_POOL  { ixOsalMutexUnlock(&macPoolLock); }

#define LOCK_TREE_POOL   { ixOsalMutexLock(&treePoolLock, IX_OSAL_WAIT_FOREVER); }
#define UNLOCK_TREE_POOL { ixOsalMutexUnlock(&treePoolLock); }

/* private function prototypes */
IX_ETH_DB_PRIVATE MacDescriptor* ixEthDBPoolAllocMacDescriptor(void);
IX_ETH_DB_PRIVATE void ixEthDBPoolFreeMacDescriptor(MacDescriptor *macDescriptor);

/**
 * @addtogroup EthMemoryManagement
 *
 * @{
 */

/**
 * @brief initializes the memory pools used by the ethernet database component
 *
 * Initializes the hash table node, mac descriptor and mac tree node pools.
 * Called at initialization time by @ref ixEthDBInit().
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBInitMemoryPools(void)
{
    int local_index;

    /* HashNode pool */
    ixOsalMutexInit(&nodePoolLock);

    for (local_index = 0 ; local_index < NODE_POOL_SIZE ; local_index++)
    {
        HashNode *freeNode = &nodePoolArea[local_index];

        freeNode->nextFree = nodePool;
        nodePool           = freeNode;
    }

    /* MacDescriptor pool */
    ixOsalMutexInit(&macPoolLock);

    for (local_index = 0 ; local_index < MAC_POOL_SIZE ; local_index++)
    {
        MacDescriptor *freeDescriptor = &macPoolArea[local_index];

        freeDescriptor->nextFree = macPool;
        macPool                  = freeDescriptor;
    }

    /* MacTreeNode pool */
    ixOsalMutexInit(&treePoolLock);

    for (local_index = 0 ; local_index < TREE_POOL_SIZE ; local_index++)
    {
        MacTreeNode *freeNode = &treePoolArea[local_index];

        freeNode->nextFree = treePool;
        treePool           = freeNode;
    }
}

/**
 * @brief allocates a hash node from the pool
 *
 * Allocates a hash node and resets its value.
 *
 * @return the allocated hash node or NULL if the pool is empty
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
HashNode* ixEthDBAllocHashNode(void)
{
    HashNode *allocatedNode = NULL;

    if (nodePool != NULL)
    {
        LOCK_NODE_POOL;

        allocatedNode = nodePool;
        nodePool      = nodePool->nextFree;

        UNLOCK_NODE_POOL;

        memset(allocatedNode, 0, sizeof(HashNode));
    }

    return allocatedNode;
}

/**
 * @brief frees a hash node into the pool
 *
 * @param hashNode node to be freed
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBFreeHashNode(HashNode *hashNode)
{
    if (hashNode != NULL)
    {
        LOCK_NODE_POOL;

        hashNode->nextFree = nodePool;
        nodePool           = hashNode;

        UNLOCK_NODE_POOL;
    }
}

/**
 * @brief allocates a mac descriptor from the pool
 *
 * Allocates a mac descriptor and resets its value.
 * This function is not used directly, instead @ref ixEthDBAllocMacDescriptor()
 * is used, which keeps track of the pointer reference count.
 *
 * @see ixEthDBAllocMacDescriptor()
 * 
 * @warning this function is not used directly by any other function
 * apart from ixEthDBAllocMacDescriptor()
 *
 * @return the allocated mac descriptor or NULL if the pool is empty
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
MacDescriptor* ixEthDBPoolAllocMacDescriptor(void)
{
    MacDescriptor *allocatedDescriptor = NULL;

    if (macPool != NULL)
    {
        LOCK_MAC_POOL;

        allocatedDescriptor = macPool;
        macPool             = macPool->nextFree;

        UNLOCK_MAC_POOL;

        memset(allocatedDescriptor, 0, sizeof(MacDescriptor));
    }

    return allocatedDescriptor;
}

/**
 * @brief allocates and initializes a mac descriptor smart pointer
 *
 * Uses @ref ixEthDBPoolAllocMacDescriptor() to allocate a mac descriptor
 * from the pool and initializes its reference count.
 *
 * @see ixEthDBPoolAllocMacDescriptor()
 *
 * @return the allocated mac descriptor or NULL if the pool is empty
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacDescriptor* ixEthDBAllocMacDescriptor(void)
{
    MacDescriptor *allocatedDescriptor = ixEthDBPoolAllocMacDescriptor();

    if (allocatedDescriptor != NULL)
    {
        LOCK_MAC_POOL;

        allocatedDescriptor->refCount++;

        UNLOCK_MAC_POOL;
    }

    return allocatedDescriptor;
}

/**
 * @brief frees a mac descriptor back into the pool
 *
 * @param macDescriptor mac descriptor to be freed
 *
 * @warning this function is not to be called by anyone but
 * ixEthDBFreeMacDescriptor()
 *
 * @see ixEthDBFreeMacDescriptor()
 *
 * @internal
 */
IX_ETH_DB_PRIVATE
void ixEthDBPoolFreeMacDescriptor(MacDescriptor *macDescriptor)
{
    LOCK_MAC_POOL;

    macDescriptor->nextFree = macPool;
    macPool                 = macDescriptor;

    UNLOCK_MAC_POOL;
}

/**
 * @brief frees or reduces the usage count of a mac descriptor smart pointer
 *
 * If the reference count reaches 0 (structure is no longer used anywhere)
 * then the descriptor is freed back into the pool using ixEthDBPoolFreeMacDescriptor().
 *
 * @see ixEthDBPoolFreeMacDescriptor()
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBFreeMacDescriptor(MacDescriptor *macDescriptor)
{
    if (macDescriptor != NULL)
    {
        LOCK_MAC_POOL;

        if (macDescriptor->refCount > 0)
        {
            macDescriptor->refCount--;

            if (macDescriptor->refCount == 0)
            {
                UNLOCK_MAC_POOL;

                ixEthDBPoolFreeMacDescriptor(macDescriptor);
            }
            else
            {
                UNLOCK_MAC_POOL;
            }
        }
        else
        {
            UNLOCK_MAC_POOL;
        }
    }
}

/**
 * @brief clones a mac descriptor smart pointer
 *
 * @param macDescriptor mac descriptor to clone
 *
 * Increments the usage count of the smart pointer
 *
 * @returns the cloned smart pointer
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacDescriptor* ixEthDBCloneMacDescriptor(MacDescriptor *macDescriptor)
{
    LOCK_MAC_POOL;

    if (macDescriptor->refCount == 0)
    {
        UNLOCK_MAC_POOL;

        return NULL;
    }

    macDescriptor->refCount++;

    UNLOCK_MAC_POOL;

    return macDescriptor;
}

/**
 * @brief allocates a mac tree node from the pool
 *
 * Allocates and initializes a mac tree node from the pool.
 *
 * @return the allocated mac tree node or NULL if the pool is empty
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacTreeNode* ixEthDBAllocMacTreeNode(void)
{
    MacTreeNode *allocatedNode = NULL;

    if (treePool != NULL)
    {
        LOCK_TREE_POOL;

        allocatedNode = treePool;
        treePool      = treePool->nextFree;

        UNLOCK_TREE_POOL;

        memset(allocatedNode, 0, sizeof(MacTreeNode));
    }

    return allocatedNode;
}

/**
 * @brief frees a mac tree node back into the pool
 *
 * @param macNode mac tree node to be freed
 *
 * @warning not to be used except from ixEthDBFreeMacTreeNode().
 *
 * @see ixEthDBFreeMacTreeNode()
 *
 * @internal
 */
void ixEthDBPoolFreeMacTreeNode(MacTreeNode *macNode)
{
    if (macNode != NULL)
    {
        LOCK_TREE_POOL;

        macNode->nextFree = treePool;
        treePool          = macNode;

        UNLOCK_TREE_POOL;
    }
}

/**
 * @brief frees or reduces the usage count of a mac tree node smart pointer
 *
 * @param macNode mac tree node to free
 *
 * Reduces the usage count of the given mac node. If the usage count
 * reaches 0 the node is freed back into the pool using ixEthDBPoolFreeMacTreeNode()
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
void ixEthDBFreeMacTreeNode(MacTreeNode *macNode)
{
    if (macNode->descriptor != NULL)
    {
        ixEthDBFreeMacDescriptor(macNode->descriptor);
    }

    if (macNode->left != NULL)
    {
        ixEthDBFreeMacTreeNode(macNode->left);
    }

    if (macNode->right != NULL)
    {
        ixEthDBFreeMacTreeNode(macNode->right);
    }

    ixEthDBPoolFreeMacTreeNode(macNode);
}

/**
 * @brief clones a mac tree node
 *
 * @param macNode mac tree node to be cloned
 *
 * Increments the usage count of the node, <i>its associated descriptor 
 * and <b>recursively</b> of all its child nodes</i>.
 *
 * @warning this function is recursive and clones whole trees/subtrees, use only for
 * root nodes
 *
 * @internal
 */
IX_ETH_DB_PUBLIC
MacTreeNode* ixEthDBCloneMacTreeNode(MacTreeNode *macNode)
{
    if (macNode != NULL)
    {
        MacTreeNode *clonedMacNode = ixEthDBAllocMacTreeNode();

        if (clonedMacNode != NULL)
        {
            if (macNode->right != NULL)
            {
                clonedMacNode->right = ixEthDBCloneMacTreeNode(macNode->right);
            }

            if (macNode->left != NULL)
            {
                clonedMacNode->left = ixEthDBCloneMacTreeNode(macNode->left);
            }

            if (macNode->descriptor != NULL)
            {
                clonedMacNode->descriptor = ixEthDBCloneMacDescriptor(macNode->descriptor);
            }
        }

        return clonedMacNode;
    }
    else
    {
        return NULL;
    }
}

#ifndef NDEBUG
/* Debug statistical functions for memory usage */

extern HashTable dbHashtable;
int ixEthDBNumHashElements(void);

int ixEthDBNumHashElements(void)
{   
    UINT32 bucketIndex;
    int numElements = 0;
    HashTable *hashTable = &dbHashtable;

    for (bucketIndex = 0 ; bucketIndex < hashTable->numBuckets ; bucketIndex++)
    {
        if (hashTable->hashBuckets[bucketIndex] != NULL)
        {
            HashNode *node = hashTable->hashBuckets[bucketIndex];

            while (node != NULL)
            {
                numElements++;

                node = node->next;
            }
        }
    }

    return numElements;
}

UINT32 ixEthDBSearchTreeUsageGet(MacTreeNode *tree)
{
    if (tree == NULL)
    {
        return 0;
    }
    else
    {
        return 1 /* this node */ + ixEthDBSearchTreeUsageGet(tree->left) + ixEthDBSearchTreeUsageGet(tree->right);
    }
}

int ixEthDBShowMemoryStatus(void)
{
    MacDescriptor *mac;
    MacTreeNode *tree;
    HashNode *node;

    int macCounter  = 0;
    int treeCounter = 0;
    int nodeCounter = 0;

    int totalTreeUsage            = 0;
    int totalDescriptorUsage      = 0;
    int totalCloneDescriptorUsage = 0;
    int totalNodeUsage            = 0;

    UINT32 portIndex;

    LOCK_NODE_POOL;
    LOCK_MAC_POOL;
    LOCK_TREE_POOL;

    mac  = macPool;
    tree = treePool;
    node = nodePool;

    while (mac != NULL)
    {
        macCounter++;

        mac = mac->nextFree;

        if (macCounter > MAC_POOL_SIZE)
        {
            break;
        }
    }

    while (tree != NULL)
    {
        treeCounter++;

        tree = tree->nextFree;

        if (treeCounter > TREE_POOL_SIZE)
        {
            break;
        }
    }

    while (node != NULL)
    {
        nodeCounter++;

        node = node->nextFree;

        if (nodeCounter > NODE_POOL_SIZE)
        {
            break;
        }
    }

    for (portIndex = 0 ; portIndex < IX_ETH_DB_NUMBER_OF_PORTS ; portIndex++)
    {
        int treeUsage = ixEthDBSearchTreeUsageGet(ixEthDBPortInfo[portIndex].updateMethod.searchTree);

        totalTreeUsage            += treeUsage;
        totalCloneDescriptorUsage += treeUsage; /* each tree node contains a descriptor */
    }

    totalNodeUsage        = ixEthDBNumHashElements();
    totalDescriptorUsage += totalNodeUsage; /* each hash table entry contains a descriptor */

    UNLOCK_NODE_POOL;
    UNLOCK_MAC_POOL;
    UNLOCK_TREE_POOL;

    printf("Ethernet database memory usage stats:\n\n");

    if (macCounter <= MAC_POOL_SIZE)
    {
        printf("\tMAC descriptor pool  : %d free out of %d entries (%d%%)\n", macCounter, MAC_POOL_SIZE, macCounter * 100 / MAC_POOL_SIZE);
    }
    else
    {
        printf("\tMAC descriptor pool  : invalid state (ring within the pool), normally %d entries\n", MAC_POOL_SIZE);
    }

    if (treeCounter <= TREE_POOL_SIZE)
    {
	printf("\tTree node pool       : %d free out of %d entries (%d%%)\n", treeCounter, TREE_POOL_SIZE, treeCounter * 100 / TREE_POOL_SIZE);
    }
    else
    {
        printf("\tTREE descriptor pool  : invalid state (ring within the pool), normally %d entries\n", TREE_POOL_SIZE);
    }

    if (nodeCounter <= NODE_POOL_SIZE)
    {
	printf("\tHash node pool       : %d free out of %d entries (%d%%)\n", nodeCounter, NODE_POOL_SIZE, nodeCounter * 100 / NODE_POOL_SIZE);
    }
    else
    {
        printf("\tNODE descriptor pool  : invalid state (ring within the pool), normally %d entries\n", NODE_POOL_SIZE);
    }

    printf("\n");
    printf("\tMAC descriptor usage : %d entries, %d cloned\n", totalDescriptorUsage, totalCloneDescriptorUsage);
    printf("\tTree node usage      : %d entries\n", totalTreeUsage);
    printf("\tHash node usage      : %d entries\n", totalNodeUsage);
    printf("\n");

    /* search for duplicate nodes in the mac pool */
    {
        MacDescriptor *reference = macPool;

        while (reference != NULL)
        {
            MacDescriptor *comparison = reference->nextFree;

            while (comparison != NULL)
            {
                if (reference == comparison)
                {
                    printf("Warning: reached a duplicate (%p), invalid MAC pool state\n", reference);

                    return 1;
                }

                comparison = comparison->nextFree;
            }

            reference = reference->nextFree;
        }
    }

    printf("No duplicates found in the MAC pool (sanity check ok)\n");

    return 0;
}

#endif /* NDEBUG */

/**
 * @} EthMemoryManagement
 */
