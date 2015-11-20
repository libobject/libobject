AC_INIT([libobject], [1.0.2], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([foreign])
LT_INIT([disable-static])
AC_SUBST([LIBTOOL_DEPS])
LIB_OBJECT_VERSION=1:2:0
AC_SUBST(LIB_OBJECT_VERSION)
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([src/config.h])
AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug],
                             [Turn on debugging [default=no]])],[enable_debug=yes],
              [enable_debug=no])

AC_MSG_CHECKING([if we are enabling debug support])
AS_IF([test "x$enable_debug" = "xyes"], [
       AC_DEFINE(DEBUG, 1, [Wheather to enable debugging messages])
       AC_MSG_RESULT([yes])
], [ AC_MSG_RESULT([no]) ])

if test -n "$GCC"; then
  CFLAGS="$CFLAGS -g -Wall -Wextra"
fi

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 src/libobjectconfig.h
 test/Makefile
])
AC_OUTPUT

cat << EOF

Build configuration:
	CPPFLAGS = $CPPFLAGS
	CFLAGS = $CFLAGS
	CXXFLAGS = $CXXFLAGS
	LDFLAGS = $LDFLAGS
	CC = $CC
	Installation prefix: $prefix

	To run the tests use make check
	To install use make install
EOF
