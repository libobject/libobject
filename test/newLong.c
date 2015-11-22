#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_newLong(void)
{

	Object* value = newLong(LONG_MAX);

	expect(O_LVAL(value) == LONG_MAX);
	
	objectDestroy(value);
}

int main(void)
{

	test_newLong();
}
