AC_INIT([libobject], [0.0.1], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC([gcc cc])
AC_PROG_CC_C99
AC_CONFIG_HEADERS([src/config.h])
AC_DEFINE(DEBUG, 1, [My description for DEBUG])
AC_CHECK_HEADER(stddef.h, , [AC_MSG_ERROR(
[Could not find stddef.h]
)])
AC_CHECK_HEADER(stdint.h, , [AC_MSG_ERROR(
[Could not find stdint.h]
)])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
