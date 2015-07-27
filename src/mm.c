/*
 *  Copyright (c) 2015 Ryan McCullagh <me@ryanmccullagh.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "mm.h"
#include "object.h"

static ObjectMMNode* MMGlobal = NULL;
static ObjectMM_Destructor MMGlobal_DefaultDestructor = NULL;

void ObjectMM_Init(ObjectMM_Destructor defaultDestructor) 
{
	MMGlobal_DefaultDestructor = defaultDestructor;
}


static void ObjectMM_API_Free(ObjectMMNode* node)
{
	while(node != NULL) {
#ifdef DEBUG
		printf("%s(): freeing ObjectMMNode:%p\n", __func__, (void *)(node));
#endif
		ObjectMMNode* next = node->next;
		//Object* value = node->value;
		//node->free(value);	
		//objectDestroy(value);
		free(node);
		node = next;		
	}
}

void MM_DECREF(ObjectMMNode* node)
{
	node->ref_count = node->ref_count - 1;
	if(node->ref_count == 0 && node->value_freed == 0) {
		(void)node->free(node->value);
		node->value_freed = 1;
	}
}


static void MM_API_Run(ObjectMMNode* node)
{
	while(node != NULL) {
		ObjectMMNode* next = node->next;
		Object* value = node->value;
		if(node->ref_count <= 0 && !node->value_freed) {
#ifdef DEBUG
			printf("%s(): freeing Object:%p,ref_count=%zu\n", __func__, (void *)value, node->ref_count);
#endif
			node->free(value);
			node->value_freed = 1;	
		}
		//free(node);
		node = next;		
	}
}

void MM_Run()
{
	MM_API_Run(MMGlobal);
}

void ObjectMM_Free()
{
	ObjectMM_API_Free(MMGlobal);

}

static ObjectMMNode* ObjectMM_Search(ObjectMMNode* root, void* ptr)
{
	while(root != NULL) {
		ObjectMMNode* next = root->next;
		if(root->value == ptr) {
			return root;		
		}	
		root = next;		
	}
	return NULL;	
}

/*
 * push an Object instance onto the beginning of the linked list.
 * return the pointer to the value that was passed to it, not a copy
 * Object system requires the caller to call objectDestroy on each allocated 
 * object. Complex data structures like Map, and Array make a copy of pointer
 * This Memory Manager allows the caller to keep all allocated Objects in
 * a linked list, and requires them to only call ObjectMM__Free once to free
 * all allocated Objects. All value are inserted in constant time
 */
static Object* ObjectMM_API_Push(ObjectMMNode*** out, ObjectMMNode** list, Object* value, ObjectMM_Destructor destructor)
{
	if(value == NULL) {
		printf("%s(): passing a NULL pointer not allowed\n", __func__);
		return value;
	}
	if(list == NULL) {
		ObjectMMNode* node = calloc(1, sizeof(ObjectMMNode));
		BUG_ON_NULL(node);
		node->value = value;
		node->ref_count = 1;
		node->next = NULL;
		node->value_freed = 0;
		node->free = destructor;
		*out = &node;
		*list = node;
	} else {
		ObjectMMNode* prev = *list;
		ObjectMMNode* next = calloc(1,sizeof(ObjectMMNode));
		BUG_ON_NULL(next);
		next->value = value;
		next->next = prev;
		next->free = destructor;
		next->value_freed = 0;
		next->ref_count = 1;
		*out = &next;
		*list = next;
	}
	
	return value;
}

Object* ObjectMM_Push(Object* value)
{
	ObjectMMNode** out = NULL;
	return ObjectMM_API_Push(&out, &MMGlobal, value, MMGlobal_DefaultDestructor);
}

Object* ObjectMM_Push_Ex(ObjectMMNode** out, Object* value)
{
	return ObjectMM_API_Push(&out, &MMGlobal, value, MMGlobal_DefaultDestructor);
}

ObjectMMNode* MM_Push(Object* value)
{
	ObjectMMNode** out = NULL;
	Object* pushed = ObjectMM_API_Push(&out, &MMGlobal, value, MMGlobal_DefaultDestructor);
	return *out;
}



