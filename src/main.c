#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "mm.h"

#define MM_COLLECT(pointer) ObjectMM_Push(pointer)

int main(void)
{
	ObjectMM_Init(objectDestruct);

	Object* map = newMap(2);
	ObjectMMNode* map_ref = MM_Push(map);
	
	Object* temp;
	temp = newString("Ryan McCullagh");
	ObjectMMNode* temp_ref;
	temp_ref = MM_Push(temp);
	mapInsert(map, "name", temp);
	MM_DECREF(temp_ref);

	temp = newString("NIU");
	temp_ref = MM_Push(temp);
	mapInsert(map, "name", temp);
	MM_DECREF(temp_ref);

	/*
	temp = newString("CDK");
	temp_ref = MM_Push(temp);
	mapInsert(map, "employer", temp);
	temp_ref->ref_count--;
	MM_Run();
	
	temp = newString("CDK");
	temp_ref = MM_Push(temp);
	mapInsert(map, "Ryan", temp);
	temp_ref->ref_count--;
	MM_Run();
	
	temp = newString("CDK");
	temp_ref = MM_Push(temp);
	mapInsert(map, "Gear", temp);
	temp_ref->ref_count--;
	MM_Run();
	*/
	Object* key;
	Object* value;
	MAP_FOREACH_KEY_VALUE(map, key, value) {
		char* pKey = key->toString(key);
		Object* search;
		search = mapSearch(map, pKey);
		if(search == NULL) {
			printf("mapSearch: %s not found\n", pKey);
		} else {
			printf("%s: ", pKey);
			OBJECT_DUMP(search);
			objectDestroy(search);
		}
		free(pKey);
		objectDestroy(key);
		objectDestroy(value);
	} MAP_FOREACH_END();

	MM_DECREF(map_ref);

	ObjectMM_Free();

	Object* split = stringSplit("This is a string", ' ');
	OBJECT_DUMP(split);
	return 0;
}
