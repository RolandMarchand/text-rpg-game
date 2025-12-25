#include <stdarg.h>
#include <stdio.h>

#include "utils.h"

const char *err_to_string(Error err)
{
	switch (err) {
	case ERR_OK: return "ok";
	default: return "unknown";
	}
}

int errorf(const char *restrict format, ...)
{
	va_list args;
	va_start(args, format);

	int ret = vfprintf(stderr, format, args);

	va_end(args);
	return ret;
}
