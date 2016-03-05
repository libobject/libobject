#include <stdio.h>
#include <object.h>
#include <stdlib.h>
#include <limits.h>

#include "test_common.h"

static void test_mapSearchEx(void)
{
	Object *symbol_table = newMap(1);

	Object *name = newString("Ryan McCullagh");

	mapInsertEx(symbol_table, "name", name);

	Object *found = mapSearchEx(symbol_table, "name");

	expect(found == name);
}

int main(void)
{

	test_mapSearchEx();
}
