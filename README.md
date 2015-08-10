# libobject
`libobject` is a C library that provides dynamic copy on write data structures.

# allocating objects
allocating an object is a multi step process for the internal API. The userland
API functions make this easier.
To allocate an Array object instance, call `newArray(<size>)`.
Inside of `newArray`, `newObject` is called. `newObject` tried to allocate 
sizeof(Object) to the heap. If it fails to do that, it returns a NULL pointer.
That return value of newObject is checked by newArray, and if it is NULL, newArray
returns NULL. If the NULL check is false, then newArray will call newArrayInstance. 
If it returns NULL, it will free the return value of `newObject`, and finally
return NULL. The caller should the return value of newArray.

##ownership
Ownership of allocated objects belongs to the caller. Data structures such as Map, Array, all make copies
of the value argument.

##requirements
- c99 compiler

##compiling
- autoreconf --install
- ./configure
- make
