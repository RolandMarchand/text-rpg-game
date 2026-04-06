#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define LAZ_UTILS_IMPLEMENTATION
#include "laz_utils.h"

#include "common.h"
#include "view.h"
#include "list/list.h"

/* When true, exit at the end of the current frame */
bool shouldExitGameLoop = false;

Error init(void)
{
	/* Memory allocators */
	initListPool();
	initArena();

	/* Components */
	initView();

	return ERR_OK;
}

Error gameLoop(void)
{
	Step steps[] = {
		updateView
	};

	while (shouldExitGameLoop == 0) {
		for (size_t i = 0; i < ARRAY_LENGTH(steps); i++) {
			Error err = steps[i]();
			if (err != ERR_OK) {
				return err;
			}
		}
	}

	return ERR_OK;
}

Error cleanup(void)
{
	/* Components */
	cleanupView();

	/* Memory allocators */
	cleanupArena();
	cleanupListPool();

	return ERR_OK;
}

void exitGameLoop(void)
{
	shouldExitGameLoop = true;
}

int main(void)
{
	Step steps[] = {
		init,
		gameLoop,
		cleanup
	};

	for (size_t i = 0; i < ARRAY_LENGTH(steps); i++) {
		Error err = steps[i]();
		if (err != ERR_OK) {
			errorf("%s\n", errorToString(err));
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}
