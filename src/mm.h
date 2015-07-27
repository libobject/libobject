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
#ifndef __OBJECT_MM_H
#define __OBJECT_MM_H

#include "object.h"

typedef void(*ObjectMM_Destructor)(void*);
typedef struct ObjectMMNode {
	size_t			value_freed;
	size_t			ref_count;
	Object* 		value;
	ObjectMM_Destructor     free;
	struct ObjectMMNode* 	next;
} ObjectMMNode;

void MM_Run();
void ObjectMM_Init();
void ObjectMM_Free();
Object* ObjectMM_Push(Object*);
Object* ObjectMM_Push_Ex(ObjectMMNode**, Object*);
ObjectMMNode* MM_Push(Object* value);
void MM_DECREF(ObjectMMNode*);

#endif

