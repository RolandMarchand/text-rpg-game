#pragma once

typedef enum Error {
	ERR_OK = 0,
} Error;

static inline const char *errorToString(Error err)
{
	switch (err) {
	case ERR_OK: return "ok";
	default: return "unknown";
	}
}
