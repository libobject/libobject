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

#include <stdio.h> /* For FILE* */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#if BUILDING_LIBOBJECT && HAVE_VISIBILITY
#define LIBOBJECT_API __attribute__((__visibility__("default")))
#elif BUILDING_LIBOBJECT && defined _MSC_VER
#define LIBOBJECT_API __declspec(dllexport)
#elif defined _MSC_VER
#define LIBOBJECT_API __declspec(dllimport)
#else
#define LIBOBJECT_API
#endif

typedef struct Object Object;

typedef enum ObjectType {
	IS_NULL,
	IS_BOOL,
	IS_LONG,
	IS_DOUBLE,
	IS_STRING,
	IS_ARRAY,
	IS_MAP,
	IS_OBJECT,
	IS_FUNCTION,
	IS_PAIR
} ObjectType;

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
	int		__notUsed;
	uint32_t	hash;
	struct 		Bucket* next;
} Bucket;

typedef struct Map {
	uint32_t	capacity;
	uint32_t	size;
	Bucket**	buckets;
} Map;

typedef struct Pair {
	Object* first;
	Object* second;
} Pair;
struct Object {
	ObjectType	type;
	int		marked;
	union {
		long		nullValue;
		long		boolValue;
		long 		longValue;
		double 		doubleValue;
		String* 	stringValue;
		Array*		arrayValue;
		Map*		mapValue;
		void*		functionValue;
		Pair*	  pairValue;
	} value;	
};

#define O_TYPE(o) (o)->type
#define O_MRKD(o) (o)->marked
#define O_NVAL(o) (o)->value.nullValue
#define O_BVAL(o) (o)->value.boolValue
#define O_LVAL(o) (o)->value.longValue
#define O_DVAL(o) (o)->value.doubleValue
#define O_SVAL(o) (o)->value.stringValue
#define O_AVAL(o) (o)->value.arrayValue
#define O_MVAL(o) (o)->value.mapValue
#define O_FVAL(o) (o)->value.functionValue
#define O_PVAL(o) (o)->value.pairValue

extern LIBOBJECT_API int         setDebuggingOutFile(FILE*);
extern LIBOBJECT_API const char* libObjectVersion(void);
extern LIBOBJECT_API char*       objectToString(Object*);
extern LIBOBJECT_API Object*     newPair(Object*, Object*);
extern LIBOBJECT_API Object*     newNull(void);
extern LIBOBJECT_API Object*     newBool(int);
extern LIBOBJECT_API Object*     newLong(long);
extern LIBOBJECT_API Object*     newDouble(double);
extern LIBOBJECT_API Object*     newMap(uint32_t);
extern LIBOBJECT_API uint32_t    mapInsert(Object*, const char*, Object*);
extern LIBOBJECT_API uint32_t    mapSize(Object*);
extern LIBOBJECT_API uint32_t    mapCapacity(Object*);
extern LIBOBJECT_API Bucket*     mapGetBucket(Object*, uint32_t);
extern LIBOBJECT_API Object*     mapSearch(Object*, const char*);
extern LIBOBJECT_API Object*     mapGetValueByHash(Object*, uint32_t);
extern LIBOBJECT_API void        mapDelete(Object*, const char*);
extern LIBOBJECT_API Object*     newString(const char*);
extern LIBOBJECT_API Object*     newStringFromSequence(const char*, size_t);
extern LIBOBJECT_API Object*     newStringFromSubstr(Object*, size_t, size_t);
extern LIBOBJECT_API Object*     newFunction(void*);
extern LIBOBJECT_API Object*     newArray(size_t);
extern LIBOBJECT_API size_t      arrayPush(Object*, Object*);
extern LIBOBJECT_API Object*     arrayGet(Object* object, size_t);
extern LIBOBJECT_API size_t      arraySize(Object*);
extern LIBOBJECT_API uint32_t    stringHash(const char* source, size_t length);
extern LIBOBJECT_API Object*     stringSplit(const char*, char);
extern LIBOBJECT_API void        objectEcho(Object*);
extern LIBOBJECT_API void        objectDump(Object*, Object*, size_t);
extern LIBOBJECT_API void        objectDumpEx(Object*, Object*, size_t);
extern LIBOBJECT_API void        objectSafeDestroy(Object*, Object*);
extern LIBOBJECT_API Object*     copyObject(Object*);
extern LIBOBJECT_API char*       objectToJson(Object*, int pretty, size_t* length);

#define objectDestroy(o) objectSafeDestroy(o, NULL)

#define OBJECT_DUMP(o) objectDump(o, NULL, 0)

#define OBJECT_DUMP_EX(o) objectDumpEx(o, NULL, 0)

#define ARRAY_FOREACH(_ar) do { \
	size_t _i; \
	for(_i = 0; _i < arraySize(_ar); _i++) { \
		Object* _value = arrayGet(_ar, _i); \

#define ARRAY_FOREACH_VALUE(_ar, _val) \
	ARRAY_FOREACH(_ar) \
	_val = _value; \

#define ARRAY_ENUMERATE(_ar, _in, _val) \
	ARRAY_FOREACH(_ar) \
	_in = _i; \
	_val = _value; \

#define ARRAY_FOREACH_END() \
	} \
} while(0)

#define ARRAY_ENUMERATE_END() \
	} \
} while(0)

#define MAP_FOREACH(_ht) do { \
	uint32_t _index; \
	for(_index = 0; _index < mapCapacity(_ht); _index++) { \
		Bucket* _b = (O_MVAL(_ht))->buckets[_index]; \
		if(_b != NULL) { \
			while(_b != NULL) { \
				Bucket* _next = _b->next; \
				Object* _value = copyObject(_b->value); \
				Object* _key = newString(_b->key->value); \

#define MAP_FOREACH_KEY_VALUE(_ht, _k, _val) \
	MAP_FOREACH(_ht) \
	_k = _key; \
	_val = _value; \

#define MAP_FOREACH_END() \
				_b = _next; \
			} \
		} \
	} \
} while(0)

#endif /* OBJECT_H */
