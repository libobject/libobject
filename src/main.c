/*
 * Copyright (c) 2016 Ryan McCullagh <me@ryanmccullagh.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "object.h"
#include "mm.h"

int main(void)
{
	ObjectMMNode* MM = NULL;
	
	Object* array = ObjectMM_Push(&MM, newArray(2));
	
	arrayPush(array, ObjectMM_Push(&MM, newLong(24)));
	arrayPush(array, ObjectMM_Push(&MM, newLong(25)));
	arrayPush(array, ObjectMM_Push(&MM, newLong(24)));
	arrayPush(array, ObjectMM_Push(&MM, newLong(25)));
	
	Object* node;
	ARRAY_FOREACH_VALUE(array, node) {
		ObjectMM_Push(&MM, node);
		objectEcho(1, node);
		printf("\n");	
	} ARRAY_FOREACH_END();

	size_t index;
	ARRAY_ENUMERATE(array, index, node) {
		ObjectMM_Push(&MM, node);
		printf("%zu => ", index);
		objectEcho(1, node);
		printf("\n");
	} ARRAY_ENUMERATE_END();

	Object* map = ObjectMM_Push(&MM, newMap(2));
	mapInsert(map, "name", ObjectMM_Push(&MM, newString("Ryan McCullagh")));
	mapInsert(map, "age", ObjectMM_Push(&MM, newLong(24)));
	mapInsert(map, "Ryan", ObjectMM_Push(&MM, newString("ebzqmtb")));
	mapInsert(map, "Apple", ObjectMM_Push(&MM, newString("lvdadfad")));
	mapInsert(map, "Gear", ObjectMM_Push(&MM, newString("lvdadfad")));
	mapInsert(map, "Four", ObjectMM_Push(&MM, newString("lvdadfad")));
	
	Object* key;
	Object* value;
	MAP_FOREACH_KEY_VALUE(map, key, value) {
		OBJECT_DUMP_EX(map);
		mapDelete(map, O_SVAL(key)->value);
		objectDestroy(key);
		objectDestroy(value);
	} MAP_FOREACH_END();

	OBJECT_DUMP(map);
	ObjectMM_Free(&MM);
	

	return 0;
}
