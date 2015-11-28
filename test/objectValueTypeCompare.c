#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_objectValueTypeCompare(void)
{

	Object* value1 = newLong(LONG_MAX);
	Object* value2 = newLong(LONG_MAX);

	expect(objectValueTypeCompare(value1, value2));

	
	Object* value3 = newArray(2);


	expect(!objectValueTypeCompare(value1, value3));
	
	objectDestroy(value1);
	objectDestroy(value2);
	objectDestroy(value3);
}

int main(void)
{

	test_objectValueTypeCompare();
}
