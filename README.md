# lib-object
Libobject provides an API for dynamic values. Every _type_ provided
by the library inherits from the Object structure. There are 7 types:
null, bool, long, double, string, array, and map. This is similar to the
JavaScript value system.

# What is it?
An Object* pointer is either an Array, Map, int, double, bool, or String. Every value is an Object, so it is very flexible. Meaning that you can create an array with any type of Object value. Think of what a JSON value is and that is exactly what libobject provides. 

Libobject exposes public functions to create these types.

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
Only Linux systems are supported at this time. Auto-tools are required to install
- `./autogen.sh`
- `./configure`
- `make`
- `make install`

This will install the binary into `/usr/local/lib`, and the header file `object.h` into `/usr/local/include` You may have to execute `ldconfig` to update the linkers cache. On some systems, `/usr/local` isn't in the include path (red hat), so you might have to figure out how to make it in the include path.

# API Reference
[http://libobject.github.io](http://libobject.github.io)
