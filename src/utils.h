#pragma once

typedef enum Error {
	ERR_OK = 0,
} Error;

const char *err_to_string(Error err);
int errorf(const char *restrict format, ...);
