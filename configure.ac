AC_INIT([libobject], [1.0.0], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([foreign])
LT_INIT([disable-static])
gl_VISIBILITY
AC_SUBST([LIBTOOL_DEPS])
AC_PROG_CC([gcc cc])
AC_PROG_CC_C99
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

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 src/libobjectconfig.h
])
AC_OUTPUT
