#include <stdio.h>
#include "object.h"

int main(void)
{
	Object* array = newArray(4);
	
	Object* a = newLong(24);
	Object* b = newLong(25);
	arrayPush(array, a);
	arrayPush(array, b);

	objectDestroy(a);
	objectDestroy(b);

	OBJECT_DUMP_EX(array);

	Object* map = newMap(8);
	mapInsert(map, "digits", array);
	objectDestroy(array);

	OBJECT_DUMP(map);
	objectDestroy(map);

	return 0;
}
