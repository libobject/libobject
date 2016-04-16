#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_objectValueIsLessThan(void)
{

	Object* value1 = newLong(LONG_MIN);
	Object* value2 = newLong(LONG_MAX);
	
	expect(objectValueIsLessThan(value1, value2));

	expect(!objectValueIsLessThan(value1, value1));

	expect(objectValueIsLessThan(newString("a"), newString("b")));
	
	expect(!objectValueIsLessThan(newString("a"), newString("a")));
}

int main(void)
{

	test_objectValueIsLessThan();
}
