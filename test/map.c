#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"


static void test_map(void)
{
	Object *map = newMap(2);
	Object *name = newString("Ryan McCullagh");

	mapInsertEx(map, "name", name);
	object_print_stats(name);
	mapInsertEx(map, "name", newString("name"));

	object_print_stats(name);

}

int main(void)
{

	test_map();
}
