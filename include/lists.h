#ifndef _LISTS_H_
#define _LISTS_H_

#define LIST_START	-1      /* Handy Constants that substitute for item positions */
#define LIST_END	0       /* END_OF_LIST means one past current length of list when */
				/* inserting. Otherwise it refers the last item in the list. */

typedef struct
    {
    void            *ptr;
    unsigned int    size;
    } HandleRecord;

typedef void **Handle;

typedef int (*CompareFunction)(void *data1, void *data2) ;

typedef struct ListStructTag
    {
    int signature;              /* debugging aid */
    int percentIncrease;        /* %of current size to increase by when list is out of space */
    int minNumItemsIncrease;    /* fixed number of items to increase by when list is out of space */
    int listSize;               /* number of items than can fit in the currently allocated memory */
    int itemSize;               /* the size of each item in the list (same for every item) */
    int numItems;               /* number of items currently in the list */
    unsigned char itemList[1];  /* resizable array of list elements */
    } ListStruct;

typedef struct ListStructTag **list_t;        /* The list abstract data type */
typedef int ( * ListApplicationFunc)(int index, void *ptrToItem, void *callbackData);

/* Basic List Operations */
list_t	ListCreate(int elementSize);
int     ListNumItems(list_t list);
int     ListInsertItem(list_t list, void *ptrToItem, int itemPosition);
int     ListInsertItems(list_t list, void *ptrToItems, int firstItemPosition, int numItemsToInsert);
void    ListDispose(list_t list);
void    *ListGetPtrToItem(list_t list, int itemPosition);
void    ListRemoveItem(list_t list, void *itemDestination, int itemPosition);
void    ListRemoveItems(list_t list, void *itemsDestination, int firstItemPosition, int numItemsToRemove);

#if 0	/* rarely ever used; kept here for reference just in case ... */
void    ListDisposePtrList(list_t list);
void    ListGetItem(list_t list, void *itemDestination, int itemPosition);
void    ListReplaceItem(list_t list, void *ptrToItem, int itemPosition);
void    ListRemoveItem(list_t list, void *itemDestination, int itemPosition);
void    ListGetItems(list_t list, void *itemsDestination, int firstItemPosition, int numItemsToGet);
void    ListReplaceItems(list_t list, void *ptrToItems, int firstItemPosition, int numItemsToReplace);
void    ListRemoveItems(list_t list, void *itemsDestination, int firstItemPosition, int numItemsToRemove);
list_t  ListCopy(list_t originalList);
int     ListAppend(list_t list1, list_t list2);
void    ListClear(list_t list);
int     ListEqual(list_t list1, list_t list2);
int     ListInsertInOrder(list_t list, void *ptrToItem, CompareFunction compareFunction);
void    *ListGetDataPtr(list_t list);
int     ListApplyToEach(list_t list, int ascending, ListApplicationFunc funcToApply, void *callbackData);

/* List Searching and Sorting */
int     ListFindItem(list_t list, void *ptrToItem, int startingPosition, CompareFunction compareFunction);
void    ListRemoveDuplicates(list_t list, CompareFunction compareFunction);
int     ListBinSearch(list_t list, void *itemPtr, CompareFunction compareFunction);
void    ListQuickSort(list_t list, CompareFunction compareFunction);
void    ListHeapSort(list_t list, CompareFunction compareFunction);
void    ListInsertionSort(list_t list, CompareFunction compareFunction);
int     ListIsSorted(list_t list, CompareFunction compareFunction);

/*  Advanced List Functions */
void	ListSetAllocationPolicy(list_t list, int minItemsPerAlloc, int percentIncreasePerAlloc);
void    ListCompact(list_t list);
int     ListPreAllocate(list_t list, int numItems);
int     ListGetItemSize(list_t list);
int     GetIntListFromParmInfo(va_list parmInfo, int numIntegers, list_t *integerList);
int     ListInsertAfterItem(list_t list, void *ptrToItem, void *ptrToItemToInsertAfter, CompareFunction compareFunction);
int     ListInsertBeforeItem(list_t list, void *ptrToItem, void *ptrToItemToInsertBefore, CompareFunction compareFunction);
#endif /* 0 */

#endif	/* _LISTS_H_ */
