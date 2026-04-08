#include "arena.h"

#include <assert.h>

struct obstack arena;

/* Note: obstacks abort on allocation errors, no error management needed */

Error initArena(void)
{
	obstack_init(&arena);
	return ERR_OK;
}

Error cleanupArena(void)
{
	obstack_free(&arena, NULL);
	return ERR_OK;
}

char *duplicateString(const char *str)
{
	assert(str != NULL);

	return obstack_copy0(&arena, str, strlen(str));
}

void *arenaAlloc(size_t size)
{
	return obstack_alloc(&arena, size);
}
