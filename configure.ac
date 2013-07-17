AC_INIT([Cachetest],[1.0],[aszlavik@uwaterloo.ca])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_ARG_WITH([papi],
            AS_HELP_STRING([--with-papi=DIR],[support for PAPI]),
            [],
            [with_papi=no]
            )

PAPI_DIR=
AS_IF([test "x$with_papi" != x],
      [CPPFLAGS="$CPPFLAGS -I$with_papi/include"
       AC_SUBST([PAPI_DIR],["$with_papi"])],
      []
      )

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/util/Makefile])

AC_PROG_CC
AC_PROG_CXX
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T
AC_CHECK_HEADERS([papi.h],[found_papi=yes],[found_papi=no])

AM_CONDITIONAL([PAPI],[test "x$found_papi" = xyes ])

AC_OUTPUT
