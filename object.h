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

#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>
#include <stdint.h>

/* Forward declaration. See below for definition */
typedef struct Object Object;
typedef struct Object IntegerObject;
typedef struct Object ArrayObject;

typedef enum ObjectType {
	IS_LONG,
	IS_DOUBLE,
	IS_STRING,
	IS_ARRAY,
	IS_MAP,
	IS_OBJECT,
	IS_FUNCTION
} ObjectType;

extern const char *const ObjectPrettyTypeLiteral[];

#define O_PRETTY_TYPE(i) ObjectPrettyTypeLiteral[i]
 
typedef struct String {
	size_t		length;
	char*		value;
} String;

typedef struct Array {
	size_t		capacity;
	size_t		size;
	size_t 		nextIndex;
	Object**	table;
} Array;

typedef struct Bucket {
	String*		key;
	Object*		value;
	size_t		__notUsed;
	size_t		hash;
	struct 		Bucket*  next;
} Bucket;

typedef struct Map {
	size_t		capacity;
	size_t		size;
	Bucket**	buckets;
} Map;

/*
 * 16 byte Object object
 */
struct Object {
	ObjectType	type;
	int		marked;
	union {
		long 		longValue;
		double 		doubleValue;
		String* 	stringValue;
		Array*		arrayValue;
		Map*		mapValue;
		void*		functionValue;
	} value;	
};

#define O_TYPE(o) (o)->type
#define O_MRKD(o) (o)->marked
#define O_LVAL(o) (o)->value.longValue
#define O_DVAL(o) (o)->value.doubleValue
#define O_SVAL(o) (o)->value.stringValue
#define O_AVAL(o) (o)->value.arrayValue
#define O_MVAL(o) (o)->value.mapValue
#define O_FVAL(o) (o)->value.functionValue
/*
 * Scalar public API methods 
 */
Object* 	newLong(long);
Object*		newDouble(double);
/*
 * Public API methods for a Map 
 */
Object*		newMap(size_t);
size_t		mapInsert(Object*, const char*, Object*);
size_t		mapSize(Object*);
size_t		mapCapacity(Object*);
Bucket*		mapGetBucket(Object*, size_t);
Object*		mapSearch(Object*, const char*);
Object*		mapGetValueByHash(Object*, size_t);
/*
 * Public API methods for String
 */
Object*		newString(const char*);
/*
 * Public API methods for Function
 */
Object*		newFunction(void*);
/*
 * Public API methods for Array
 */
Object*		newArray(size_t);
void		arrayPush(Object*, Object*);
size_t		arrayPop(Object*, Object**);
Object*		arrayGet(Object* object, size_t);
size_t		arraySize(Object*);
void		__arrayMultiPush(Object*, size_t, ...);

size_t stringLength(const char* source);
size_t stringHash(const char* source, size_t length);


#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,N,...) N

#define arrayMultiPush(o, ...) \
__arrayMultiPush(o, VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

/*
 * Public debugging methods
 */
void objectEcho(size_t length, ...);
void objectDump(Object*, Object*, size_t);

#define OBJECT_DUMP(o) objectDump(o, NULL, 0)

#endif /* OBJECT_H */
