#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_objectValueCompare(void)
{

	Object* value1 = newLong(LONG_MAX);
	Object* value2 = newLong(LONG_MAX);
	Object* value3 = newBool(1);
	Object* value4 = newBool(0);
	Object* value5 = newString("Ryan");
	Object* value6 = newString("McCullagh");
	
	expect(objectValueCompare(value1, value2));

	expect(!objectValueCompare(value1, value3));

	expect(objectValueCompare(value3, value4) == 0);


	expect(!objectValueCompare(value5, value6));

	expect(objectValueCompare(value5, value5));
	
	objectDestroy(value1);
	objectDestroy(value2);
	objectDestroy(value3);
	objectDestroy(value4);
	objectDestroy(value5);
	objectDestroy(value6);
}

int main(void)
{

	test_objectValueCompare();
}
