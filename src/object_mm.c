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

#include "object_mm.h"
#include "object.h"

LIBOBJECT_API void ObjectMM_Free(ObjectMMNode** root)
{
	ObjectMMNode* node = *root;
	while(node != NULL) {
		ObjectMMNode* next = node->next;
		Object* value = node->value;
		objectDestroy(value);
		free(node);
		node = next;		
	}
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
LIBOBJECT_API Object* ObjectMM_Push(ObjectMMNode** list, Object* value)
{
	if(value == NULL) {
		printf("%s(): passing a NULL pointer not allowed\n", __func__);
		return value;
	}
	if(*list == NULL) {
		*list = malloc(sizeof(ObjectMMNode));
		BUG_ON_NULL(*list);
		(*list)->value = value;
		(*list)->ref_count = 1;
		(*list)->next = NULL;
	} else {
		ObjectMMNode* n = ObjectMM_Search(*list, value);
		if(n == NULL) {
			ObjectMMNode* prev = *list;
			ObjectMMNode* next = malloc(sizeof(ObjectMMNode));
			BUG_ON_NULL(next);
			next->value = value;
			next->next = prev;
			*list = next;
		} else {
			size_t new_ref_count = n->ref_count + 1;
			#ifdef DEBUG
				printf("%s(): increasing ref count for %p\n", __func__, (void *)value);
				printf("\told=%zu,current=%zu\n", n->ref_count, new_ref_count);
			#endif
			n->ref_count = new_ref_count;
		}
	}
	
	return value;
}







