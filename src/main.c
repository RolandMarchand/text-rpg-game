#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "common.h"

struct obstack arena;

Error init(void)
{
	obstack_init(&arena);
	return ERR_OK;
}

Error cleanup(void)
{
	obstack_free(&arena, NULL);
	return ERR_OK;
}

int main(void)
{
	Error err = init();
	if (err != ERR_OK) {
		exit(EXIT_FAILURE);
	}

	err = cleanup();
	if (err != ERR_OK) {
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
