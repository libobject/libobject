# lib-object
Libobject provides an API for dynamic values. Every _type_ provided
by the library inherits from the Object structure. There are 7 types:
null, bool, long, double, string, array, and map. This is similar to the
JavaScript value system.

# What is it?
An Object* pointer is either an Array, Map, int, double, bool, or String. Every value is an Object, so it is very flexible. Meaning that you can create an array with any type of Object value. Think of what a JSON value is and that is exactly what libobject provides. 

Libobject exposes public functions to create these types.

# Data Types

- Map
	A map is an unordered mutable hash table. Each _key_ in the hash table must be and always is a string. The _value_ in the map can be any type of Object. More specifically, the value can be an instance of Array, Map, String, Long, Float, Bool, or NULL. Set, and Get type of methods for Map insert a copy, and return a copy, respectivelly for each of those methods.

- Array
	An Array is a mutable ordered collection of values whose first element starts at index 0. A value in an Array can be of any Object type. Insert type methods on an Array instance copy the value to be inserted. Getter methods return a copy of the value that was to be gotten.

- String
	A String is an immutable (by convention) ordered sequence of characters. a String has a length property.

# Examples
Creating an Array object type.

```C
#include <object.h>

int main(void)
{
  Object* array = newArray(2); // create an array with a size of 2. It grows automatically.
  
  if(!array) return 1;
  
  Object* value = newString("libobject");
  arrayPush(array, value);
  objectDestroy(value); // arrayPush makes a copy of the value, so the caller should free it.
  
  OBJECT_DUMP(array);
  
  objectDestroy(array);

  return 0;
}
```

# Installing
Only Linux systems are supported at this time. [https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html](autotools] are required to install and [http://www.gnu.org/software/libtool/](libtool).
- `./autogen.sh`
- `./configure`
- `make`
- `make install`

This will install the binary into `/usr/local/lib`, and the header file `object.h` into `/usr/local/include` You may have to execute `ldconfig` to update the linkers cache. On some systems, `/usr/local` isn't in the include path (red hat), so you might have to figure out how to make it in the include path.

# API Reference
[httup://libobject.github.io](http://libobject.github.io)
