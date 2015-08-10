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

typedef struct ObjectMMNode {
	size_t			ref_count;
	Object* 		value;
	struct ObjectMMNode* 	next;
} ObjectMMNode;

extern LIBOBJECT_API void ObjectMM_Free(ObjectMMNode**);
extern LIBOBJECT_API Object* ObjectMM_Push(ObjectMMNode**, Object*);

#endif /* __OBJECT_MM_H */

