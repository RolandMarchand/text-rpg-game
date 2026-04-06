#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "list.h"

#define POOL_SIZE 128
#define POOL_END ((size_t)-1)

VECTOR_DEFINE(List, list, void*)
VECTOR_DEFINE(ListString, list_string, char*)

typedef struct {
	List lists[POOL_SIZE];
	size_t nextFree[POOL_SIZE];
	bool allocated[POOL_SIZE];
} Pool;

static Pool listPool;
static size_t freeListPoolHead = 0;

void initListPool(void) {
	for (size_t i = 0; i < POOL_SIZE; i++) {
		listPool.lists[i] = (List){ 0 };
		listPool.allocated[i] = false;
		listPool.nextFree[i] = i + 1;
	}

	listPool.nextFree[POOL_SIZE - 1] = POOL_END;
}

List *newList(void) {
	printf("+1\n");
	fflush(stdout);
	bool outOfSpace = freeListPoolHead == POOL_END;
	assert(!outOfSpace);
	if (outOfSpace) {
		return NULL;
	}

	listPool.lists[freeListPoolHead] = (List){ 0 };
	listPool.allocated[freeListPoolHead] = true;
	List *ret = &listPool.lists[freeListPoolHead];
	freeListPoolHead = listPool.nextFree[freeListPoolHead];

	return ret;
}

void freeList(List *list) {
	uintptr_t list_addr = (uintptr_t)list;
	uintptr_t pool_addr = (uintptr_t)&listPool;
	uintptr_t addr_diff = list_addr - pool_addr;

	/* Check for out of range or unaligned */
	bool validAddress = list_addr >= pool_addr
		&& list_addr < pool_addr + sizeof(listPool.lists)
		&& addr_diff % sizeof(List) == 0;
	assert(validAddress);
	if (!validAddress) {
		return;
	}

	size_t idx = addr_diff / sizeof(List);

	if (!listPool.allocated[idx]) {
		return;
	}

	printf("-1\n");
	fflush(stdout);
	listPool.nextFree[idx] = freeListPoolHead;
	listPool.allocated[idx] = false;
	freeListPoolHead = idx;
}

void cleanupListPool(void)
{
	for (size_t i = 0; i < POOL_SIZE; i++) {
		if (listPool.allocated[i]) {
			printf("-1\n");
			fflush(stdout);
			list_free(&listPool.lists[i]);
		}
	}
	initListPool();
}
