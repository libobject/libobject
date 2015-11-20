/**
 * Copyright (c) 2015 Ryan McCullagh <me@ryanmccullagh.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __TEST_COMMON_H__
#define __TEST_COMMON_H__

#include <stdio.h>
#include <string.h>

#define STRINGIFY(x) #x

#define expect(exp) do { \
	if(!(exp)) { \
		fprintf(stderr, "%s:%d:%s The expression '%s' is not true\n", __FILE__, __LINE__, __func__, STRINGIFY(exp)); \
		exit(1); \
	} \
} while(0)

static inline int str_equal(const char* a, const char* b)
{
	return strcmp(a, b) == 0;
}

#endif /* __TEST_COMMON_H__ */
