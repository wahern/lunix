# Redefine AH_TEMPLATE so that feature macros can be overridden from CPPFLAGS
m4_define([AH_TEMPLATE],
[AH_VERBATIM([$1],
	m4_text_wrap([$2 */], [   ], [/* ])[
@%:@ifndef ]_m4_expand([$1])[
@%:@undef ]_m4_expand([$1])[
@%:@endif])])

# Initialize autoconf
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
AC_SEARCH_LIBS([ldexp], [m])
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_SEARCH_LIBS([sigtimedwait], [rt])
AC_SEARCH_LIBS([posix_fadvise], [rt])
AC_SEARCH_LIBS([posix_fallocate], [rt])
AC_SEARCH_LIBS([dlopen], [dl])
AX_LIB_SOCKET_NSL # -lsocket and -lnsl

# Checks for header files.
AC_CHECK_HEADERS([ \
	ifaddrs.h mach/mach.h mach/clock.h mach/mach_time.h \
	netinet/in6_var.h sys/feature_tests.h sys/param.h sys/sockio.h \
	sys/sysctl.h \
])
AC_CHECK_HEADERS([netinet6/in6_var.h], [], [], [/* silence autoconf */])

# Checks for structure members
AC_CHECK_MEMBERS([struct stat.st_rdev, struct stat.st_blksize, struct stat.st_blocks])
AC_CHECK_MEMBERS([struct stat.st_atim, struct stat.st_mtim, struct stat.st_ctim])
AC_CHECK_MEMBERS([struct stat.st_atimespec, struct stat.st_mtimespec, struct stat.st_ctimespec])

# Checks for library objects.
AC_CHECK_DECLS([sys_siglist], [], [], [[#include <signal.h>]])
WA_CHECK_VAR([sys_siglist])
AC_CHECK_DECLS([__libc_enable_secure])
WA_CHECK_VAR([__libc_enable_secure])
AC_CHECK_DECLS([program_invocation_short_name])
WA_CHECK_VAR([program_invocation_short_name])

# Checks for library functions.
AC_CHECK_FUNCS([ \
	arc4random arc4random_addrandom arc4random_stir clock_gettime \
	dup2 dup3 fdopendir getauxval getenv_r getexecname getifaddrs \
	getprogname issetugid pipe2 posix_fadvise posix_fallocate \
	sigtimedwait sigwait sysctl \
])

# Check for strerror_r and variant
AC_FUNC_STRERROR_R

# Check for static_assert and _Static_assert
WA_CHECK_STATIC_ASSERT

# Check for _Generic
WA_C__GENERIC

# Check for statement expression construct ({})
WA_C_STATEMENT_EXPRESSION

# Check for __extension__ annotation
WA_C___EXTENSION__

# Both Solaris and AIX have <sys/procfs.h>.
#
# Check for size and offset of pr_nlwp on Solaris. If compiling in 32-bit
# mode with Large File Support, we can't include <sys/procfs.h> directly. So
# compute the information we need in order to read pr_nlwp from
# /proc/$$/status.
AC_CHECK_HEADERS([sys/procfs.h], [
	AC_CHECK_MEMBERS([struct pstatus.pr_nlwp], [
		WA_CHECK_SIZEOF_MEMBERS([struct pstatus.pr_nlwp], [
			#if HAVE_SYS_PROCFS_H
			#if defined __sun
			#undef _FILE_OFFSET_BITS
			#define _STRUCTURED_PROC 1
			#endif
			#include <sys/procfs.h>
			#endif
		])

		WA_CHECK_OFFSETOF_MEMBERS([struct pstatus.pr_nlwp], [
			#if HAVE_SYS_PROCFS_H
			#if defined __sun
			#undef _FILE_OFFSET_BITS
			#define _STRUCTURED_PROC 1
			#endif
			#include <sys/procfs.h>
			#endif
		])
	], [], [
		#if HAVE_SYS_PROCFS_H
		#if defined __sun
		#undef _FILE_OFFSET_BITS
		#define _STRUCTURED_PROC 1
		#endif
		#include <sys/procfs.h>
		#endif
	])

	AC_CHECK_MEMBERS([struct psinfo.pr_fname, struct psinfo.pr_nlwp], [], [], [
		#if HAVE_SYS_PROCFS_H
		#include <sys/procfs.h>
		#endif
	])
], [], [
	#if defined __sun
	#undef _FILE_OFFSET_BITS
	#define _STRUCTURED_PROC 1
	#endif
])

AC_OUTPUT
