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

# Checks for libraries.
AC_SEARCH_LIBS([isnormal], [m])
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_SEARCH_LIBS([sigtimedwait], [rt])
AC_SEARCH_LIBS([dlopen], [dl])
AX_LIB_SOCKET_NSL # -lsocket and -lnsl

# Checks for header files.
AC_CHECK_HEADERS([ \
	ifaddrs.h mach/mach.h netinet/in6_var.h netinet6/in6_var.h \
	sys/sysctl.h \
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SSIZE_T
AC_CHECK_DECLS([sys_siglist],,,[[#include <signal.h>]])

# Checks for library functions.
AC_CHECK_FUNCS([ \
	arc4random arc4random_addrandom arc4random_stire clock_gettime \
	dup2 dup3 fopendir getauxval getenv_r getifaddrs issetugid pipe2 \
	sigtimedwait sysctl \
])

# Configure automake.
#AM_INIT_AUTOMAKE

# Configure libtool.
#LT_INIT
#AC_SUBST([LIBTOOL_DEPS])

AC_OUTPUT
