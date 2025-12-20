#pragma once

#include <stdint.h>

#include "utils.h"
#include "const.h"

#include "obstack/obstack.h"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

#if defined(__GNUC__) || defined(__clang__)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

typedef uint16_t Idx;
