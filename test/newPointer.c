#include <stdio.h>
#include <object.h>
#include <stdlib.h>

#include "test_common.h"

static void noop(void)
{
}

static void test_newPointer(void)
{

	Object* value = newPointer(noop);

	printf("noop<%p>\n", (void *)noop);
	OBJECT_DUMP(value);


	expect(O_TYPE(value) == IS_POINTER);
	expect(O_PTVAL(value) == noop);

	objectDestroy(value);
}

int main(void)
{

	test_newPointer();
}
