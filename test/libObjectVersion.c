#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void test_libObjectVersion(void)
{

	const char* value = libObjectVersion();
	printf("%s\n", value);
}

int main(void)
{

	test_libObjectVersion();
}
