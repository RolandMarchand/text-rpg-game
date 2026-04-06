#pragma once

typedef enum Error {
	ERR_OK = 0,
	ERR_RESOURCE_LOADING_FAILED,
	ERR_OUT_OF_MEMORY,
} Error;

static inline const char *errorToString(Error err)
{
	switch (err) {
	case ERR_OK: return "ok";
	case ERR_RESOURCE_LOADING_FAILED: return "resource loading failed";
	case ERR_OUT_OF_MEMORY: return "out of memory";
	default: return "unknown";
	}
}
