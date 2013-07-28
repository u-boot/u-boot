/**
 * @file ethHash.c
 *
 * @brief Hashtable implementation
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


#include "IxEthDB_p.h"
#include "IxEthDBLocks_p.h"

/**
 * @addtogroup EthDB
 *
 * @{
 */

/**
 * @brief initializes a hash table object
 *
 * @param hashTable uninitialized hash table structure
 * @param numBuckets number of buckets to use
 * @param entryHashFunction hash function used 
 * to hash entire hash node data block (for adding)
 * @param matchFunctions array of match functions, indexed on type,
 * used to differentiate records with the same hash value
 * @param freeFunction function used to free node data blocks
 *
 * Initializes the given hash table object.
 *
 * @internal
 */
void ixEthDBInitHash(HashTable *hashTable, 
                     UINT32 numBuckets, 
                     HashFunction entryHashFunction, 
                     MatchFunction *matchFunctions, 
                     FreeFunction freeFunction)
{
    UINT32 bucketIndex;
    UINT32 hashSize = numBuckets * sizeof(HashNode *);

    /* entry hashing, matching and freeing methods */
    hashTable->entryHashFunction  = entryHashFunction;
    hashTable->matchFunctions     = matchFunctions;
    hashTable->freeFunction       = freeFunction;

    /* buckets */
    hashTable->numBuckets = numBuckets;

    /* set to 0 all buckets */
    memset(hashTable->hashBuckets, 0, hashSize);

    /* init bucket locks - note that initially all mutexes are unlocked after MutexInit()*/
    for (bucketIndex = 0 ; bucketIndex < numBuckets ; bucketIndex++)
    {
        ixOsalFastMutexInit(&hashTable->bucketLocks[bucketIndex]);
    }
}

/**
 * @brief adds an entry to the hash table
 *
 * @param hashTable hash table to add the entry to
 * @param entry entry to add
 *
 * The entry will be hashed using the entry hashing function and added to the
 * hash table, unless a locking blockage occurs, in which case the caller
 * should retry.
 *
 * @retval IX_ETH_DB_SUCCESS if adding <i>entry</i> has succeeded
 * @retval IX_ETH_DB_NOMEM if there's no memory left in the hash node pool
 * @retval IX_ETH_DB_BUSY if there's a locking failure on the insertion path
 *
 * @internal
 */
IX_ETH_DB_PUBLIC IxEthDBStatus ixEthDBAddHashEntry(HashTable *hashTable, void *entry)
{
    UINT32 hashValue   = hashTable->entryHashFunction(entry);
    UINT32 bucketIndex = hashValue % hashTable->numBuckets;
    HashNode *bucket   = hashTable->hashBuckets[bucketIndex];
    HashNode *newNode;

    LockStack locks;

    INIT_STACK(&locks);

    /* lock bucket */
    PUSH_LOCK(&locks, &hashTable->bucketLocks[bucketIndex]);

    /* lock insertion element (first in chain), if any */
    if (bucket != NULL)
    {
        PUSH_LOCK(&locks, &bucket->lock);
    }

    /* get new node */
    newNode = ixEthDBAllocHashNode();

    if (newNode == NULL)
    {
        /* unlock everything */
        UNROLL_STACK(&locks);

        return IX_ETH_DB_NOMEM;
    }

    /* init lock - note that mutexes are unlocked after MutexInit */
    ixOsalFastMutexInit(&newNode->lock);

    /* populate new link */
    newNode->data = entry;

    /* add to bucket */
    newNode->next                       = bucket;
    hashTable->hashBuckets[bucketIndex] = newNode;

    /* unlock bucket and insertion point */
    UNROLL_STACK(&locks);

    return IX_ETH_DB_SUCCESS;
}

/**
 * @brief removes an entry from the hashtable
 *
 * @param hashTable hash table to remove entry from
 * @param keyType type of record key used for matching
 * @param reference reference key used to identify the entry
 *
 * The reference key will be hashed using the key hashing function,
 * the entry is searched using the hashed value and then examined
 * against the reference entry using the match function. A positive
 * match will trigger the deletion of the entry.
 * Locking failures are reported and the caller should retry.
 *
 * @retval IX_ETH_DB_SUCCESS if the removal was successful
 * @retval IX_ETH_DB_NO_SUCH_ADDR if the entry was not found
 * @retval IX_ETH_DB_BUSY if a locking failure occured during the process
 *
 * @internal
 */
IxEthDBStatus ixEthDBRemoveHashEntry(HashTable *hashTable, int keyType, void *reference)
{
    UINT32 hashValue       = hashTable->entryHashFunction(reference);
    UINT32 bucketIndex     = hashValue % hashTable->numBuckets;
    HashNode *node         = hashTable->hashBuckets[bucketIndex];
    HashNode *previousNode = NULL;
    
    LockStack locks;

    INIT_STACK(&locks);

    while (node != NULL)
    {
        /* try to lock node */
        PUSH_LOCK(&locks, &node->lock);

        if (hashTable->matchFunctions[keyType](reference, node->data))
        {
            /* found entry */
            if (node->next != NULL)
            {
                PUSH_LOCK(&locks, &node->next->lock);
            }

            if (previousNode == NULL)
            {
                /* node is head of chain */
                PUSH_LOCK(&locks, &hashTable->bucketLocks[bucketIndex]);

                hashTable->hashBuckets[bucketIndex] = node->next;

                POP_LOCK(&locks);
            }
            else
            {
                /* relink */
                previousNode->next = node->next;
            }

            UNROLL_STACK(&locks);

            /* free node */
            hashTable->freeFunction(node->data);
            ixEthDBFreeHashNode(node);

            return IX_ETH_DB_SUCCESS;
        }
        else
        {
            if (previousNode != NULL)
            {
                /* unlock previous node */
                SHIFT_STACK(&locks);
            }

            /* advance to next element in chain */
            previousNode = node;
            node         = node->next;
        }
    }

    UNROLL_STACK(&locks);

    /* not found */
    return IX_ETH_DB_NO_SUCH_ADDR;
}

/**
 * @brief retrieves an entry from the hash table
 *
 * @param hashTable hash table to perform the search into
 * @param reference search key (a MAC address)
 * @param keyType type of record key used for matching
 * @param searchResult pointer where a reference to the located hash node 
 * is placed
 *
 * Searches the entry with the same key as <i>reference</i> and places the
 * pointer to the resulting node in <i>searchResult</i>.
 * An implicit write access lock is granted after a search, which gives the 
 * caller the opportunity to modify the entry.
 * Access should be released as soon as possible using @ref ixEthDBReleaseHashNode().
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @retval IX_ETH_DB_SUCCESS if the search was completed successfully
 * @retval IX_ETH_DB_NO_SUCH_ADDRESS if no entry with the given key was found
 * @retval IX_ETH_DB_BUSY if a locking failure has occured, in which case
 * the caller should retry
 *
 * @warning unless the return value is <b>IX_ETH_DB_SUCCESS</b> the searchResult
 * location is NOT modified and therefore using a NULL comparison test when the
 * value was not properly initialized would be an error
 *
 * @internal
 */
IxEthDBStatus ixEthDBSearchHashEntry(HashTable *hashTable, int keyType, void *reference, HashNode **searchResult)
{
    UINT32 hashValue;
    HashNode *node;

    hashValue = hashTable->entryHashFunction(reference);
    node      = hashTable->hashBuckets[hashValue % hashTable->numBuckets];

    while (node != NULL)
    {
        TRY_LOCK(&node->lock);

        if (hashTable->matchFunctions[keyType](reference, node->data))
        {
            *searchResult = node;

            return IX_ETH_DB_SUCCESS;
        }
        else
        {
            UNLOCK(&node->lock);

            node = node->next;
        }
    }

    /* not found */
    return IX_ETH_DB_NO_SUCH_ADDR;
}

/**
 * @brief reports the existence of an entry in the hash table
 *
 * @param hashTable hash table to perform the search into
 * @param reference search key (a MAC address)
 * @param keyType type of record key used for matching
 *
 * Searches the entry with the same key as <i>reference</i>.
 * No implicit write access lock is granted after a search, hence the 
 * caller cannot access or modify the entry. The result is only temporary.
 *
 * @see ixEthDBReleaseHashNode()
 *
 * @retval IX_ETH_DB_SUCCESS if the search was completed successfully
 * @retval IX_ETH_DB_NO_SUCH_ADDRESS if no entry with the given key was found
 * @retval IX_ETH_DB_BUSY if a locking failure has occured, in which case
 * the caller should retry
 *
 * @internal
 */
IxEthDBStatus ixEthDBPeekHashEntry(HashTable *hashTable, int keyType, void *reference)
{
    UINT32 hashValue;
    HashNode *node;

    hashValue = hashTable->entryHashFunction(reference);
    node      = hashTable->hashBuckets[hashValue % hashTable->numBuckets];

    while (node != NULL)
    {
        TRY_LOCK(&node->lock);

        if (hashTable->matchFunctions[keyType](reference, node->data))
        {
            UNLOCK(&node->lock);

            return IX_ETH_DB_SUCCESS;
        }
        else
        {
            UNLOCK(&node->lock);

            node = node->next;
        }
    }

    /* not found */
    return IX_ETH_DB_NO_SUCH_ADDR;
}

/**
 * @brief releases the write access lock
 *
 * @pre the node should have been obtained via @ref ixEthDBSearchHashEntry()
 *
 * @see ixEthDBSearchHashEntry()
 *
 * @internal
 */
void ixEthDBReleaseHashNode(HashNode *node)
{
    UNLOCK(&node->lock);
}

/**
 * @brief initializes a hash iterator
 *
 * @param hashTable hash table to be iterated
 * @param iterator iterator object
 *
 * If the initialization is successful the iterator will point to the
 * first hash table record (if any).
 * Testing if the iterator has not passed the end of the table should be
 * done using the IS_ITERATOR_VALID(iteratorPtr) macro.
 * An implicit write access lock is granted on the entry pointed by the iterator.
 * The access is automatically revoked when the iterator is incremented.
 * If the caller decides to terminate the iteration before the end of the table is
 * passed then the manual access release method, @ref ixEthDBReleaseHashIterator,
 * must be called.
 *
 * @see ixEthDBReleaseHashIterator()
 *
 * @retval IX_ETH_DB_SUCCESS if initialization was successful and the iterator points
 * to the first valid table node
 * @retval IX_ETH_DB_FAIL if the table is empty
 * @retval IX_ETH_DB_BUSY if a locking failure has occured, in which case the caller
 * should retry
 *
 * @warning do not use ixEthDBReleaseHashNode() on entries pointed by the iterator, as this
 * might place the database in a permanent invalid lock state
 *
 * @internal
 */
IxEthDBStatus ixEthDBInitHashIterator(HashTable *hashTable, HashIterator *iterator)
{
    iterator->bucketIndex  = 0;
    iterator->node         = NULL;
    iterator->previousNode = NULL;

    return ixEthDBIncrementHashIterator(hashTable, iterator);
}

/**
 * @brief releases the write access locks of the iterator nodes
 *
 * @warning use of this function is required only when the caller terminates an iteration
 * before reaching the end of the table
 *
 * @see ixEthDBInitHashIterator()
 * @see ixEthDBIncrementHashIterator()
 *
 * @param iterator iterator whose node(s) should be unlocked
 *
 * @internal
 */
void ixEthDBReleaseHashIterator(HashIterator *iterator)
{
    if (iterator->previousNode != NULL)
    {
        UNLOCK(&iterator->previousNode->lock);
    }

    if (iterator->node != NULL)
    {
        UNLOCK(&iterator->node->lock);
    }
}

/**
 * @brief incremenents an iterator so that it points to the next valid entry of the table
 * (if any)
 *
 * @param hashTable hash table to iterate
 * @param iterator iterator object
 *
 * @pre the iterator object must be initialized using @ref ixEthDBInitHashIterator()
 *
 * If the increment operation is successful the iterator will point to the
 * next hash table record (if any).
 * Testing if the iterator has not passed the end of the table should be
 * done using the IS_ITERATOR_VALID(iteratorPtr) macro.
 * An implicit write access lock is granted on the entry pointed by the iterator.
 * The access is automatically revoked when the iterator is re-incremented.
 * If the caller decides to terminate the iteration before the end of the table is
 * passed then the manual access release method, @ref ixEthDBReleaseHashIterator,
 * must be called.
 * Is is guaranteed that no other thread can remove or change the iterated entry until
 * the iterator is incremented successfully.
 *
 * @see ixEthDBReleaseHashIterator()
 *
 * @retval IX_ETH_DB_SUCCESS if the operation was successful and the iterator points
 * to the next valid table node
 * @retval IX_ETH_DB_FAIL if the iterator has passed the end of the table
 * @retval IX_ETH_DB_BUSY if a locking failure has occured, in which case the caller
 * should retry
 *
 * @warning do not use ixEthDBReleaseHashNode() on entries pointed by the iterator, as this
 * might place the database in a permanent invalid lock state
 *
 * @internal
 */
IxEthDBStatus ixEthDBIncrementHashIterator(HashTable *hashTable, HashIterator *iterator)
{
    /* unless iterator is just initialized... */
    if (iterator->node != NULL)
    {
        /* try next in chain */
        if (iterator->node->next != NULL)
        {
            TRY_LOCK(&iterator->node->next->lock);

            if (iterator->previousNode != NULL)
            {
                UNLOCK(&iterator->previousNode->lock);
            }

            iterator->previousNode = iterator->node;
            iterator->node         = iterator->node->next;

            return IX_ETH_DB_SUCCESS;
        }
        else
        {
            /* last in chain, prepare for next bucket */
            iterator->bucketIndex++;
        }
    }

   /* try next used bucket */
    for (; iterator->bucketIndex < hashTable->numBuckets ; iterator->bucketIndex++)
    {
        HashNode **nodePtr = &(hashTable->hashBuckets[iterator->bucketIndex]);
        HashNode *node = *nodePtr;
#if (CPU!=SIMSPARCSOLARIS) && !defined (__wince)
        if (((iterator->bucketIndex & IX_ETHDB_BUCKET_INDEX_MASK) == 0) &&
            (iterator->bucketIndex < (hashTable->numBuckets - IX_ETHDB_BUCKETPTR_AHEAD)))
        {
            /* preload next cache line (2 cache line ahead) */
            nodePtr += IX_ETHDB_BUCKETPTR_AHEAD;
            __asm__ ("pld [%0];\n": : "r" (nodePtr));
        }
#endif
        if (node != NULL)
        {
            TRY_LOCK(&node->lock);

            /* unlock last one or two nodes in the previous chain */
            if (iterator->node != NULL)
            {
                UNLOCK(&iterator->node->lock);

                if (iterator->previousNode != NULL)
                {
                    UNLOCK(&iterator->previousNode->lock);
                }
            }
            
            /* redirect iterator */
            iterator->previousNode = NULL;
            iterator->node         = node;

            return IX_ETH_DB_SUCCESS;
        }
    }

    /* could not advance iterator */
    if (iterator->node != NULL)
    {
        UNLOCK(&iterator->node->lock);

        if (iterator->previousNode != NULL)
        {
            UNLOCK(&iterator->previousNode->lock);
        }

        iterator->node = NULL;
    }

    return IX_ETH_DB_END;
}

/**
 * @brief removes an entry pointed by an iterator
 *
 * @param hashTable iterated hash table
 * @param iterator iterator object
 *
 * Removes the entry currently pointed by the iterator and repositions the iterator
 * on the next valid entry (if any). Handles locking issues automatically and
 * implicitely grants write access lock to the new pointed entry.
 * Failures due to concurrent threads having write access locks in the same region
 * preserve the state of the database and the iterator object, leaving the caller
 * free to retry without loss of access. It is guaranteed that only the thread owning
 * the iterator can remove the object pointed by the iterator.
 *
 * @retval IX_ETH_DB_SUCCESS if removal has succeeded
 * @retval IX_ETH_DB_BUSY if a locking failure has occured, in which case the caller
 * should retry
 *
 * @internal
 */
IxEthDBStatus ixEthDBRemoveEntryAtHashIterator(HashTable *hashTable, HashIterator *iterator)
{
    HashIterator nextIteratorPos;
    LockStack locks;

    INIT_STACK(&locks);

    /* set initial bucket index for next position */
    nextIteratorPos.bucketIndex = iterator->bucketIndex;

    /* compute iterator position before removing anything and lock ahead */
    if (iterator->node->next != NULL)
    {
        PUSH_LOCK(&locks, &iterator->node->next->lock);

        /* reposition on the next node in the chain */
        nextIteratorPos.node         = iterator->node->next;
        nextIteratorPos.previousNode = iterator->previousNode;
    }
    else
    {
        /* try next chain - don't know yet if we'll find anything */
        nextIteratorPos.node = NULL;

        /* if we find something it's a chain head */
        nextIteratorPos.previousNode = NULL;

        /* browse up in the buckets to find a non-null chain */
        while (++nextIteratorPos.bucketIndex < hashTable->numBuckets)
        {
            nextIteratorPos.node = hashTable->hashBuckets[nextIteratorPos.bucketIndex];

            if (nextIteratorPos.node != NULL)
            {
                /* found a non-empty chain, try to lock head */
                PUSH_LOCK(&locks, &nextIteratorPos.node->lock);

                break;
            }
        }
    }

    /* restore links over the to-be-deleted item */
    if (iterator->previousNode == NULL)
    {
        /* first in chain, lock bucket */
        PUSH_LOCK(&locks, &hashTable->bucketLocks[iterator->bucketIndex]);

        hashTable->hashBuckets[iterator->bucketIndex] = iterator->node->next;

        POP_LOCK(&locks);
    }
    else
    {
        /* relink */
        iterator->previousNode->next = iterator->node->next;

        /* unlock last remaining node in current chain when moving between chains */
        if (iterator->node->next == NULL)
        {
            UNLOCK(&iterator->previousNode->lock);
        }
    }

    /* delete entry */
    hashTable->freeFunction(iterator->node->data);
    ixEthDBFreeHashNode(iterator->node);

    /* reposition iterator */
    *iterator = nextIteratorPos;

    return IX_ETH_DB_SUCCESS;
}

/**
 * @}
 */
