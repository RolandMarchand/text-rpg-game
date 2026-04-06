#pragma once

#include "vector_base.h"

VECTOR_DECLARE(List, list, void*)
VECTOR_DECLARE(ListString, list_string, char*)

/* Initialize the arena before creating new lists with `newList()` */
void initListPool(void);
/* Return a pointer to a list that will be cleaned up by `cleanupListPool()`.
 * The new list is uninitialized, so it's safe to cast to any other list type */
List *newList(void);
/* Free the list's internal memory and let the arena reclaim the list's slot */
void freeList(List *list);
/* Free every list from `newList()` and calls `initListPool()` */
void cleanupListPool(void);
