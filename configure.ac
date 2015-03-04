AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_AUX_DIR([mk])
AC_CONFIG_MACRO_DIR([mk/m4])
AC_CONFIG_FILES([Makeflags])
AC_CONFIG_SRCDIR([src/unix.c])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_MKDIR_P
AC_PROG_INSTALL
AC_PROG_RANLIB

# Define _GNU_SOURCE, __EXTENSIONS__, _POSIX_PTHREAD_SEMANTICS, etc.
AC_USE_SYSTEM_EXTENSIONS

# Largefile support
AC_SYS_LARGEFILE

# Checks for libraries.
AC_SEARCH_LIBS([isnormal], [m])
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_SEARCH_LIBS([sigtimedwait], [rt])
AC_SEARCH_LIBS([dlopen], [dl])
AX_LIB_SOCKET_NSL # -lsocket and -lnsl

# Checks for header files.
AC_CHECK_HEADERS([ \
	ifaddrs.h mach/mach.h mach/clock.h mach/mach_time.h \
	netinet/in6_var.h sys/feature_tests.h sys/param.h sys/sockio.h \
	sys/sysctl.h \
])
AC_CHECK_HEADERS([netinet6/in6_var.h], [], [], [/* silence autoconf */])

# Checks for library objects.
AC_CHECK_DECL([sys_siglist],[],[],[[#include <signal.h>]])
WA_CHECK_VAR([sys_siglist])
AC_CHECK_DECL([__libc_enable_secure])
WA_CHECK_VAR([__libc_enable_secure])

# Checks for library functions.
AC_CHECK_FUNCS([ \
	arc4random arc4random_addrandom arc4random_stir clock_gettime \
	dup2 dup3 fdopendir getauxval getenv_r getifaddrs issetugid pipe2 \
	sigtimedwait sysctl \
])

# Check for _Static_assert
AC_MSG_CHECKING([for _Static_assert])
AC_LINK_IFELSE([
	AC_LANG_PROGRAM([],[_Static_assert(1, "");])
], [
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE__STATIC_ASSERT], [1], [Define to 1 if have _Static_assert])
], [
	AC_MSG_RESULT([no])
])

AC_OUTPUT
