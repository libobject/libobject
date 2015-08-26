noinst_HEADERS = libobjectconfig.h
ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libobject.la
libobject_la_SOURCES = murmurhash3.c murmurhash3.h libobjectconfig.h object.c object_mm.c
libobject_la_CFLAGS = $(AM_CFLAGS)
libobject_la_CPPFLAGS = -DBUILDING_LIBOBJECT
libobject_la_LDFLAGS = -no-undefined -version-info ${LIB_OBJECT_VERSION}
include_HEADERS = object.h object_mm.h
