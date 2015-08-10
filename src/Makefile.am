ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(CFLAG_VISIBILITY)
lib_LTLIBRARIES = libobject.la
libobject_la_SOURCES = murmurhash3.c murmurhash3.h object.c object_mm.c
libobject_la_CFLAGS = $(AM_CFLAGS)
libobject_la_CPPFLAGS = -DBUILDING_LIBOBJECT
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# current[:revision[:age]]
libobject_la_LDFLAGS = -no-undefined -version-info 1:2:0
include_HEADERS = object.h object_mm.h
