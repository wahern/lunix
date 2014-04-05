/*
 * P O R T A B L E  S Y S T E M  I N C L U D E S
 *
 * Try to include as much as we can here for documentation purposes.
 * Includes which are spread around makes it difficult to determine which
 * headers are being used, and can make dependency ordering issues more
 * troublesome. (Localized includes makes it easier to determine why they're
 * being used, so it's a trade off.)
 *
 * Non-portable headers are included after the feature detection section.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1 /* Solaris */
#endif

#include <limits.h>       /* INT_MAX NL_TEXTMAX */
#include <stdarg.h>       /* va_list va_start va_arg va_end */
#include <stdint.h>       /* SIZE_MAX */
#include <stdlib.h>       /* arc4random(3) calloc(3) free(3) realloc(3) strtoul(3) */
#include <stdio.h>        /* fileno(3) snprintf(3) */
#include <string.h>       /* memset(3) strerror_r(3) strspn(3) strcspn(3) */
#include <signal.h>       /* sigset_t sigfillset(3) sigemptyset(3) sigprocmask(2) */
#include <ctype.h>        /* isspace(3) */
#include <time.h>         /* struct tm struct timespec gmtime_r(3) clock_gettime(3) tzset(3) */
#include <errno.h>        /* ENOMEM ERANGE errno */
#include <assert.h>       /* static_assert */

#include <sys/param.h>    /* __NetBSD_Version__ __OpenBSD_Version__ __FreeBSD_version */
#include <sys/types.h>    /* gid_t mode_t off_t pid_t uid_t */
#include <sys/resource.h> /* RUSAGE_SELF struct rusage getrusage(2) */
#include <sys/socket.h>   /* AF_INET AF_INET6 SOCK_DGRAM struct sockaddr socket(2) */
#include <sys/stat.h>     /* S_ISDIR() */
#include <sys/time.h>     /* struct timeval gettimeofday(2) */
#if __linux
#include <sys/sysctl.h>   /* CTL_KERN KERN_RANDOM RANDOM_UUID sysctl(2) */
#endif
#include <sys/utsname.h>  /* uname(2) */
#include <sys/wait.h>     /* waitpid(2) */
#include <sys/ioctl.h>    /* SIOCGIFCONF SIOCGIFFLAGS SIOCGIFNETMASK SIOCGIFDSTADDR SIOCGIFBRDADDR SIOCGLIFADDR ioctl(2) */
#include <net/if.h>       /* struct ifconf struct ifreq */
#include <unistd.h>       /* _PC_NAME_MAX chdir(2) chroot(2) close(2) chdir(2) chown(2) chroot(2) dup2(2) fpathconf(3) getegid(2) geteuid(2) getgid(2) getpid(2) getuid(2) issetugid(2) link(2) rename(2) rmdir(2) setegid(2) seteuid(2) setgid(2) setuid(2) setsid(2) symlink(2) truncate(2) umask(2) unlink(2) */
#include <fcntl.h>        /* F_DUPFD_CLOEXEC F_GETFD F_SETFD FD_CLOEXEC fcntl(2) open(2) */
#include <pwd.h>          /* struct passwd getpwnam_r(3) */
#include <grp.h>          /* struct group getgrnam_r(3) */
#include <dirent.h>       /* closedir(3) fdopendir(3) opendir(3) readdir_r(3) rewinddir(3) */
#include <netinet/in.h>   /* __KAME__ */
#include <netdb.h>        /* NI_MAXHOST gai_strerror(3) getnameinfo(3) */

#if __sun
#include <sys/feature_tests.h> /* _DTRACE_VERSION */
#include <sys/sockio.h>   /* SIOCGIFCONF SIOCGIFFLAGS SIOCGIFNETMASK SIOCGIFDSTADDR SIOCGIFBRDADDR */
#endif

#if __APPLE__
#include <mach/mach_time.h> /* mach_timebase_info() mach_absolute_time() */
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


/*
 * F E A T U R E  D E T E C T I O N
 *
 * In lieu of external detection do our best to detect features using the
 * preprocessor environment.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if __sun
#include <sys/feature_tests.h> /* _DTRACE_VERSION */
#endif

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(M, m) 0
#endif

#ifndef __has_feature
#define __has_feature(...) 0
#endif

#ifndef __NetBSD_Prereq__
#define __NetBSD_Prereq__(M, m, p) 0
#endif

#define GNUC_PREREQ(M, m) __GNUC_PREREQ(M, m)

#define NETBSD_PREREQ(M, m) __NetBSD_Prereq__(M, m, 0)

#define FREEBSD_PREREQ(M, m) (__FreeBSD_version >= ((M) * 100000) + ((m) * 1000))

#define SUNOS_PREREQ_5_10 (defined __sun && defined _DTRACE_VERSION)
#define SUNOS_PREREQ_5_11 (defined __sun && defined F_DUPFD_CLOEXEC)
#define SUNOS_PREREQ(M, m) SUNOS_PREREQ_ ## M ## _ ## m

#ifndef HAVE_ARC4RANDOM
#define HAVE_ARC4RANDOM (defined __OpenBSD__ || defined __FreeBSD__ || defined __NetBSD__ || defined __MirBSD__ || defined __APPLE__)
#endif

#ifndef HAVE_PIPE2
#define HAVE_PIPE2 (GNUC_PREREQ(2,9) || FREEBSD_PREREQ(10,0) || NETBSD_PREREQ(6,0))
#endif

#ifndef HAVE_DUP3
#define HAVE_DUP3 (GNUC_PREREQ(2,9) || FREEBSD_PREREQ(10,0) || NETBSD_PREREQ(6,0))
#endif

#ifndef HAVE_FDOPENDIR
#define HAVE_FDOPENDIR (!defined __APPLE__ && (!defined __NetBSD__ || NETBSD_PREREQ(6,0)))
#endif

#ifndef HAVE_ISSETUGID
#define HAVE_ISSETUGID (!defined __linux && !defined _AIX)
#endif

#ifndef HAVE_GETAUXVAL
#define HAVE_GETAUXVAL GNUC_PREREQ(2,16)
#endif

#ifndef HAVE_IFADDRS_H
#define HAVE_IFADDRS_H (!defined _AIX && (!defined __sun || SUNOS_PREREQ(5,11)))
#endif

#ifndef HAVE_GETIFADDRS
#define HAVE_GETIFADDRS HAVE_IFADDRS_H
#endif

#ifndef HAVE_SOCKADDR_SA_LEN
#define HAVE_SOCKADDR_SA_LEN (!defined __linux && !defined __sun)
#endif

#ifndef HAVE_NETINET_IN6_VAR_H
#define HAVE_NETINET_IN6_VAR_H defined _AIX
#endif

#ifndef HAVE_NETINET6_IN6_VAR_H
#define HAVE_NETINET6_IN6_VAR_H defined __KAME__
#endif


/*
 * N O N - P O R T A B L E  S Y S T E M  I N C L U D E S
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if HAVE_IFADDRS_H
#include <ifaddrs.h> /* struct ifaddrs getifaddrs(3) freeifaddrs(3) */
#endif

#if HAVE_NETINET_IN6_VAR_H
#include <netinet/in6_var.h> /* SIOCGIFADDR6 SIOCGIFNETMASK6 SIOCGIFDSTADDR6 struct in6_ifreq */
#endif

/*
 * NOTE: Include only if we lack <ifaddrs.h>. FreeBSD requires
 * <net/if_var.h>, and who knows what other header dependency issues we
 * could run into.
 */ 
#if HAVE_NETINET6_IN6_VAR_H && !HAVE_IFADDRS_H
#include <netinet6/in6_var.h> /* SIOCGIFADDR_IN6 SIOCGIFNETMASK_IN6 SIOCGIFDSTADDR_IN6 struct in6_ifreq */
#endif


/*
 * L U A  C O M P A T A B I L I T Y
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if LUA_VERSION_NUM < 502

#ifndef LUA_FILEHANDLE
#define LUA_FILEHANDLE "FILE*"
#endif


/*
 * Lua 5.1 userdata is a simple FILE *, while LuaJIT is a struct with the
 * first member a FILE *, similar to Lua 5.2.
 */
typedef struct luaL_Stream {
	FILE *f;
} luaL_Stream;


static int lua_absindex(lua_State *L, int index) {
	return (index > 0 || index <= LUA_REGISTRYINDEX)? index : lua_gettop(L) + index + 1;
} /* lua_absindex() */


static void *luaL_testudata(lua_State *L, int index, const char *tname) {
	void *p = lua_touserdata(L, index);
	int eq;

	if (!p || !lua_getmetatable(L, index))
		return 0;

	luaL_getmetatable(L, tname);
	eq = lua_rawequal(L, -2, -1);
	lua_pop(L, 2);

	return (eq)? p : 0;
} /* luaL_testudate() */


static void luaL_setmetatable(lua_State *L, const char *tname) {
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
} /* luaL_setmetatable() */


static void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
	int i, t = lua_absindex(L, -1 - nup);

	for (; l->name; l++) {
		for (i = 0; i < nup; i++)
			lua_pushvalue(L, -nup);
		lua_pushcclosure(L, l->func, nup);
		lua_setfield(L, t, l->name);
	}

	lua_pop(L, nup);
} /* luaL_setfuncs() */


#define luaL_newlibtable(L, l) \
	lua_createtable(L, 0, (sizeof (l) / sizeof *(l)) - 1)

#define luaL_newlib(L, l) \
	(luaL_newlibtable((L), (l)), luaL_setfuncs((L), (l), 0))

#endif /* LUA_VERSION_NUM < 502 */


/*
 * C O M P I L E R  A N N O T A T I O N S
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef NOTUSED
#if __GNUC__
#define NOTUSED __attribute__((unused))
#else
#define NOTUSED
#endif
#endif

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif


/*
 * M I S C  &  C O M P A T  R O U T I N E S
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#ifndef countof
#define countof(a) (sizeof (a) / sizeof *(a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b))? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b))? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(i, m, n) (((i) < (m))? (m) : ((i) > (n))? (n) : (i))
#endif

#ifndef XPASTE
#define PASTE(x, y) x##y
#define XPASTE(x, y) PASTE(x, y)
#endif

#ifndef STRINGIFY
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#endif

#if defined static_assert
#define u_static_assert(cond, msg) static_assert(cond, msg)
#elif GNUC_PREREQ(4,6) || (__clang__ && __has_feature(c_static_assert))
#define u_static_assert(cond, msg) _Static_assert(cond, msg)
#else
#define u_static_assert(cond, msg) char XPASTE(assert_, __LINE__)[sizeof (int[1 - 2*!(cond)])]
#endif


static size_t u_power2(size_t i) {
#if defined SIZE_MAX
	i--;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
#if SIZE_MAX != 0xffffffffu
	i |= i >> 32;
#endif
	return ++i;
#else
#error No SIZE_MAX defined
#endif
} /* u_power2() */


#define u_error_t int

static u_error_t u_realloc(char **buf, size_t *size, size_t minsiz) {
	void *tmp;
	size_t tmpsiz;

	if (*size == (size_t)-1)
		return ENOMEM;

	if (*size > ~((size_t)-1 >> 1)) {
		tmpsiz = (size_t)-1;
	} else {
		tmpsiz = u_power2(*size + 1);
		tmpsiz = MIN(tmpsiz, minsiz);
	}

	if (!(tmp = realloc(*buf, tmpsiz)))
		return errno;

	*buf = tmp;
	*size = tmpsiz;

	return 0;
} /* u_realloc() */


static void *u_memjunk(void *buf, size_t bufsiz) {
	struct {
		pid_t pid;
		struct timeval tv;
		struct rusage ru;
#if __APPLE__
		uint64_t mt;
#else
		struct timespec mt;
#endif
		struct utsname un;
		uintptr_t aslr;
	} junk;
	struct { const unsigned char *const buf; size_t size, p; } src = { (void *)&junk, sizeof junk, 0 };
	struct { unsigned char *const buf; size_t size, p; } dst = { buf, bufsiz, 0 };

	junk.pid = getpid();
	gettimeofday(&junk.tv, NULL);
	getrusage(RUSAGE_SELF, &junk.ru);
#if __APPLE__
	junk.mt = mach_absolute_time();
#else
	clock_gettime(CLOCK_MONOTONIC, &junk.mt);
#endif
	uname(&junk.un);
	junk.aslr = (uintptr_t)&strcpy ^ (uintptr_t)&u_memjunk;

	while (src.p < src.size || dst.p < dst.size) {
		dst.buf[dst.p % dst.size] ^= src.buf[src.p % src.size];
		++src.p;
		++dst.p;
	}

	return buf;
} /* u_memjunk() */


static socklen_t u_sa_len(const struct sockaddr *sa) {
#if defined SA_LEN
	return SA_LEN(sa);
#elif HAVE_SOCKADDR_SA_LEN
	return sa->sa_len;
#else
	switch (sa->sa_family) {
	case AF_INET:
		return sizeof (struct sockaddr_in);
	case AF_INET6:
		return sizeof (struct sockaddr_in6);
	default:
		return sizeof (struct sockaddr);
	}
#endif
} /* u_sa_len() */


/* derived from KAME source */
static void u_in6_prefixlen2mask(struct in6_addr *mask, unsigned prefixlen) {
	unsigned octets, bits, i;

	if (prefixlen > 128)
		return;

	memset(mask, 0, sizeof *mask);

	octets = prefixlen / 8;
	bits = prefixlen % 8;

	u_static_assert(sizeof mask->s6_addr == 16, "strange s6_addr data type");
	memset(mask, 0xff, octets);

	if (bits)
		mask->s6_addr[octets] = (0xff00 >> bits) & 0xff;
} /* u_in6_prefixlen2mask() */


#ifndef IN6_IS_SCOPE_LINKLOCAL
#define IN6_IS_SCOPE_LINKLOCAL(in6) (IN6_IS_ADDR_LINKLOCAL(in6) || IN6_IS_ADDR_MC_LINKLOCAL(in6))
#endif

#ifndef IPV6_ADDR_MC_SCOPE
#define IPV6_ADDR_MC_SCOPE(in6) ((in6)->s6_addr[1] & 0x0f)
#endif

#ifndef IPV6_ADDR_SCOPE_INTFACELOCAL
#define IPV6_ADDR_SCOPE_INTFACELOCAL 0x01
#endif

#ifndef IN6_IS_ADDR_MC_INTFACELOCAL
#define IN6_IS_ADDR_MC_INTFACELOCAL(in6) (IN6_IS_ADDR_MULTICAST(in6) || IPV6_ADDR_MC_SCOPE(in6) == IPV6_ADDR_SCOPE_INTFACELOCAL)
#endif

static int u_in6_clearscope(struct in6_addr *in6) {
	int modified = 0;

	if (IN6_IS_SCOPE_LINKLOCAL(in6) || IN6_IS_ADDR_MC_INTFACELOCAL(in6)) {
		modified = (in6->s6_addr[2] || in6->s6_addr[3]);
		in6->s6_addr[2] = 0;
		in6->s6_addr[3] = 0;
	}

	return modified;
} /* u_in6_clearscope() */


/*
 * T H R E A D - S A F E  I / O  O P E R A T I O N S
 *
 * Principally we're concerned with atomically setting the
 * FD_CLOEXEC/O_CLOEXEC flag. O_CLOEXEC was added to POSIX 2008 and the BSDs
 * took awhile to catch up. But POSIX only defined it for open(2). Some
 * systems have non-portable extensions to support O_CLOEXEC for pipe
 * and socket creation.
 *
 * Also, very old systems do not support modern O_NONBLOCK semantics on
 * open. As it's easy to cover this case we do, otherwise such old systems
 * are beyond our purview.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef O_CLOEXEC
#define U_CLOEXEC (1LL << 32)
#else
#define U_CLOEXEC (O_CLOEXEC)
#endif

#define U_SYSFLAGS ((1LL << 32) - 1)

#define u_flags_t long long


static u_error_t u_close(int *fd) {
	int error;

	if (*fd == -1)
		return errno;

	error = errno;

	(void)close(*fd);
	*fd = -1;

	errno = error;

	return error;
} /* u_close() */


static u_error_t u_setflag(int fd, u_flags_t flag, int enable) {
	int flags;

	if (flag & U_CLOEXEC) {
		if (-1 == (flags = fcntl(fd, F_GETFD)))
			return errno;

		if (enable)
			flags |= FD_CLOEXEC;
		else
			flags &= ~FD_CLOEXEC;

		if (0 != fcntl(fd, F_SETFD, flags))
			return errno;
	} else {
		if (-1 == (flags = fcntl(fd, F_GETFL)))
			return errno;

		if (enable)
			flags |= flag;
		else
			flags &= ~flag;

		if (0 != fcntl(fd, F_SETFL, flags))
			return errno;
	}

	return 0;
} /* u_setflag() */


static u_error_t u_getflags(int fd, u_flags_t *flags) {
	int _flags;

	if (-1 == (_flags = fcntl(fd, F_GETFL)))
		return errno;

	*flags = _flags;

	if (!(*flags & U_CLOEXEC)) {
		if (-1 == (_flags = fcntl(fd, F_GETFD)))
			return errno;

		if (_flags & FD_CLOEXEC)
			*flags |= U_CLOEXEC;
	}

	return 0;
} /* u_getflags() */


static u_error_t u_fixflags(int fd, u_flags_t flags) {
	u_flags_t _flags = 0;
	int error;

	if ((flags & U_CLOEXEC) || (flags & O_NONBLOCK)) {
		if ((error = u_getflags(fd, &_flags)))
			return error;

		if ((flags & U_CLOEXEC) && !(_flags & U_CLOEXEC)) {
			if ((error = u_setflag(fd, U_CLOEXEC, 1)))
				return error;
		}

		if ((flags & O_NONBLOCK) && !(_flags & O_NONBLOCK)) {
			if ((error = u_setflag(fd, O_NONBLOCK, 1)))
				return error;
		}
	}

	return 0;
} /* u_fixflags() */


static u_error_t u_open(int *fd, const char *path, u_flags_t flags, mode_t mode) {
	u_flags_t _flags;
	int error;

	if (-1 == (*fd = open(path, (U_SYSFLAGS & flags), mode)))
		goto syerr;

	if ((error = u_fixflags(*fd, flags)))
		goto error;

	return 0;
syerr:
	error = errno;
error:
	u_close(fd);

	return error;
} /* u_open() */


static u_error_t u_pipe(int *fd, u_flags_t flags) {
#if HAVE_PIPE2
	if (0 != pipe2(fd, flags)) {
		fd[0] = -1;
		fd[1] = -1;

		return errno;
	}

	return 0;
#else
	int i, error;

	if (0 != pipe(fd)) {
		fd[0] = -1;
		fd[1] = -1;

		return errno;
	}

	for (i = 0; i < 2; i++) {
		if ((error = u_fixflags(fd[i], flags))) {
			u_close(&fd[0]);
			u_close(&fd[1]);

			return error;
		}
	}

	return 0;
#endif
} /* u_pipe() */


static u_error_t u_dup3(int fd, int fd2, u_flags_t flags) {
#if HAVE_DUP3
	if (-1 == dup3(fd, fd2, flags))
		return errno;

	return 0;
#else
	int close2 = (0 != fstat(fd2, &(struct stat){ 0 }));
	int error;

	if (-1 == dup2(fd, fd2))
		return errno;

	if ((error = u_fixflags(fd2, flags))) {
		if (close2)
			u_close(&fd2);

		return error;
	}

	return 0;
#endif
} /* u_dup3() */


static u_error_t u_socket(int *fd, int family, int type, int proto, u_flags_t flags) {
	int error;

#if defined SOCK_CLOEXEC
	if (flags & O_CLOEXEC)
		proto |= SOCK_CLOEXEC;
#endif

#if defined SOCK_NONBLOCK
	if (flags & O_NONBLOCK)
		proto |= SOCK_NONBLOCK;
#endif

	if (-1 == (*fd = socket(family, type, proto)))
		return errno;

	if ((error = u_fixflags(*fd, flags))) {
		u_close(fd);

		return error;
	}

	return 0;
} /* u_socket() */


static u_error_t u_fdopendir(DIR **dp, int fd) {
#if HAVE_FDOPENDIR
	int error;

	if ((error = u_setflag(fd, U_CLOEXEC, 1)))
		return error;

	if (!(*dp = fdopendir(fd)))
		return errno;

	return 0;
#else
	struct stat st;
	int fd2, error;

	if (0 != fstat(fd, &st))
		goto syerr;

	if (!S_ISDIR(st.st_mode)) {
		error = ENOTDIR;

		goto error;
	}

	if (!(*dp = opendir(".")))
		goto syerr;

	if (-1 == (fd2 = dirfd(*dp)))
		goto syerr;

	if ((error = u_dup3(fd, fd2, U_CLOEXEC)))
		goto error;

	if (-1 == lseek(fd2, 0, SEEK_SET))
		goto syerr;

	u_close(&fd);

	return 0;
syerr:
	error = errno;
error:
	if (*dp) {
		closedir(*dp);
		*dp = NULL;
	}

	return error;
#endif
} /* u_fdopendir() */


#if HAVE_GETIFADDRS

#define u_ifaddrs ifaddrs

static u_error_t u_getifaddrs(struct u_ifaddrs **ifs) {
	return (0 == getifaddrs(ifs))? 0 : errno;
} /* u_getifaddrs() */

static void u_freeifaddrs(struct u_ifaddrs *ifs) {
	freeifaddrs(ifs);
} /* u_freeifaddrs() */

#else

#undef ifa_dstaddr

struct u_ifaddrs {
	struct u_ifaddrs *ifa_next;
	char ifa_name[sizeof ((struct ifreq *)0)->ifr_name];
	unsigned int ifa_flags;
	struct sockaddr *ifa_addr;
	struct sockaddr *ifa_netmask;
	struct sockaddr *ifa_dstaddr;

	struct sockaddr_storage ifa_ss[3];
}; /* struct u_ifaddrs */


static void u_freeifaddrs(struct u_ifaddrs *ifs) {
	struct u_ifaddrs *ifa, *nxt;

	for (ifa = ifs; ifa; ifa = nxt) {
		nxt = ifa->ifa_next;
		free(ifa);
	}
} /* if_freeifaddrs() */


#define U_IFREQ_MAXSIZE (sizeof (struct ifreq) - sizeof (struct sockaddr) + sizeof (struct sockaddr_storage))

static u_error_t u_getifconf(struct ifconf *ifc, int fd) {
	char *buf = NULL;
	size_t bufsiz;
	int error;

	ifc->ifc_buf = NULL;
	ifc->ifc_len = 0;

	do {
		bufsiz = (size_t)ifc->ifc_len + U_IFREQ_MAXSIZE;

		/* check for arithmetic overflow when adding sizeof sockaddr_storage */
		if (bufsiz < U_IFREQ_MAXSIZE)
			goto range;

		if ((error = u_realloc(&buf, &bufsiz, MAX(256, bufsiz))))
			goto error;

		/* ifc->ifc_len is usually an int; be careful of undefined conversion */
		if (bufsiz > INT_MAX)
			goto range;

		memset(buf, 0, bufsiz);

		ifc->ifc_buf = (void *)buf;
		ifc->ifc_len = bufsiz;

		if (-1 == ioctl(fd, SIOCGIFCONF, (void *)ifc))
			goto syerr;
	} while (bufsiz - U_IFREQ_MAXSIZE < (size_t)ifc->ifc_len);

	return 0;
range:
	error = ERANGE;
	goto error;
syerr:
	error = errno;
error:
	free(buf);
	ifc->ifc_buf = NULL;
	ifc->ifc_len = 0;

	return error;
} /* u_getifconf() */


#if HAVE_SOCKADDR_SA_LEN && !defined __NetBSD__
/*
 * On most systems with sa_len struct ifreq objects are variable length.
 */
#define U_SIZEOF_ADDR_IFREQ(ifr) /* from OS X <net/if.h> */ \
	(((ifr)->ifr_addr.sa_len > sizeof (struct sockaddr)) \
		? (sizeof (struct ifreq) - sizeof (struct sockaddr) + (ifr)->ifr_addr.sa_len) \
		: (sizeof (struct ifreq)))
#else
/*
 * On systems without sa_len ioctl(SIOCGIFCONF) only returns AF_INET
 * addresses, which always fits within a struct sockaddr.
 *
 * On NetBSD struct ifreq can fit addresses of any type and isn't variable
 * length.
 */
#define U_SIZEOF_ADDR_IFREQ(ifr) (sizeof (struct ifreq))
#endif


static void *u_sa_copy(struct sockaddr_storage *ss, const struct sockaddr *sa) {
	return memcpy(ss, sa, u_sa_len(sa));
} /* u_sa_copy() */


#if defined SIOCGIFADDR6 && !defined u_getif6

#define u_getif6 u_getif6_aix

static u_error_t u_getif6_aix(struct u_ifaddrs *ifa, int fd, const struct ifreq *ifr) {
	struct in6_ifreq ifr6 = { 0 };

	u_static_assert(sizeof ifr6.ifr_name == sizeof ifr->ifr_name, "sizeof ifr6_name != sizeof ifr_name");
	memcpy(ifr6.ifr_name, ifr->ifr_name, MIN(sizeof ifr6.ifr_name, sizeof ifr->ifr_name));
	memcpy(&ifr6.ifr_Addr, ifa->ifa_addr, sizeof ifr6.ifr_Addr);

	if (-1 != ioctl(fd, SIOCGIFNETMASK6, &ifr6)) {
		ifr6.ifr_Addr.sin6_family = AF_INET6; /* not set on AIX */
		ifa->ifa_netmask = u_sa_copy(&ifa->ifa_ss[1], (struct sockaddr *)&ifr6.ifr_Addr);
	}

	if (-1 != ioctl(fd, SIOCGIFDSTADDR6, &ifr6)) {
		ifr6.ifr_Addr.sin6_family = AF_INET6; /* unable to test if AIX sets family; see above */
		ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], (struct sockaddr *)&ifr6.ifr_Addr);
	}

	return 0;
} /* u_getif6_aix() */

#endif


#if defined SIOCGIFADDR_IN6 && !defined u_getif6

#define u_getif6 u_getif6_kame_in6

static u_error_t u_getif6_kame_in6(struct u_ifaddrs *ifa, int fd, const struct ifreq *ifr) {
	struct in6_ifreq ifr6 = { 0 };

	u_static_assert(sizeof ifr6.ifr_name == sizeof ifr->ifr_name, "sizeof ifr6_name != sizeof ifr_name");
	memcpy(ifr6.ifr_name, ifr->ifr_name, MIN(sizeof ifr6.ifr_name, sizeof ifr->ifr_name));
	memcpy(&ifr6.ifr_addr, ifa->ifa_addr, sizeof ifr6.ifr_addr);

	if (-1 != ioctl(fd, SIOCGIFNETMASK_IN6, &ifr6))
		ifa->ifa_netmask = u_sa_copy(&ifa->ifa_ss[1], (struct sockaddr *)&ifr6.ifr_addr);

	if (-1 != ioctl(fd, SIOCGIFDSTADDR_IN6, &ifr6))
		ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], (struct sockaddr *)&ifr6.ifr_addr);

	return 0;
} /* u_getif6_kame_in6() */

#endif


/*
 * Solaris uses struct lifreq with SIOCGLIFADDR. See u_getif6_sun_glif.
 */
#if defined SIOCGLIFADDR && !defined __sun && !defined u_getif6

#define u_getif6 u_getif6_kame_glif

static u_error_t u_getif6_kame_glif(struct u_ifaddrs *ifa, int fd, const struct ifreq *ifr) {
	struct if_laddrreq iflr = { 0 };

	u_static_assert(sizeof iflr.iflr_name == sizeof ifr->ifr_name, "sizeof iflr_name != sizeof ifr_name");
	memcpy(iflr.iflr_name, ifr->ifr_name, MIN(sizeof iflr.iflr_name, sizeof ifr->ifr_name));
	u_sa_copy(&iflr.addr, ifa->ifa_addr);

	/*
	 * NOTE: To get the same [shortest] prefixlen as SIOCGIFNETMASK_IN6
	 * or ifconfig(1) for link-local addresses we must request a prefix
	 * match.
	 *
	 * See SIOCGLIFADDR cases in KAME netinet6/in6.c:in6_lifaddr_ioctl.
	 */
#if defined IFLR_PREFIX
	iflr.flags = IFLR_PREFIX;
	iflr.prefixlen = 128;
	u_in6_clearscope(&((struct sockaddr_in6 *)&iflr.addr)->sin6_addr);
#endif

	if (-1 != ioctl(fd, SIOCGLIFADDR, &iflr)) {
		struct sockaddr_in6 *mask = (struct sockaddr_in6 *)&ifa->ifa_ss[1];
#if HAVE_SOCKADDR_SA_LEN
		mask->sin6_len = sizeof *mask;
#endif
		mask->sin6_family = AF_INET6;
		u_in6_prefixlen2mask(&mask->sin6_addr, iflr.prefixlen);
		ifa->ifa_netmask = (struct sockaddr *)mask;

		if (iflr.dstaddr.ss_family == AF_INET6)
			ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], (struct sockaddr *)&iflr.dstaddr);
	}

	return 0;
} /* u_getif6_kame_glif() */
#endif


#if defined SIOCGLIFADDR && defined __sun && !defined u_getif6

#define u_getif6 u_getif6_sun_glif

static u_error_t u_getif6_sun_glif(struct u_ifaddrs *ifa, int fd, const struct ifreq *ifr) {
	struct lifreq lifr = { 0 };

	u_static_assert(sizeof lifr.lifr_name == sizeof ifr->ifr_name, "sizeof iflr_name != sizeof ifr_name");
	memcpy(lifl.iflr_name, ifr->ifr_name, MIN(sizeof lifr.lifr_name, sizeof ifr->ifr_name));
	u_sa_copy(&lifr.lifr_addr, ifa->ifa_addr);

	if (-1 != ioctl(fd, SIOCGLIFNETMASK, &lifr)) {
		ifa->ifa_netmask = u_sa_copy(&ifa->ifa_ss[1], (struct sockaddr *)&lifr.lifr_dstaddr);
	}

	if (-1 != ioctl(fd, SIOCGLIFDSTADDR, &lifr)) {
		ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], (struct sockaddr *)&lifr.lifr_dstaddr);
	}

	if (-1 != ioctl(fd, SIOCGLIFBRDADDR, &lifr)) {
		ifa->ifa_broadaddr = u_sa_copy(&ifa->ifa_ss[2], (struct sockaddr *)&lifr.lifr_broadaddr);
	}

	return 0;
} /* u_getif6_sun_glif() */
#endif


static u_error_t u_getifaddrs(struct u_ifaddrs **ifs) {
	int fd = -1, fd6 = -1;
	struct ifconf ifc = { 0 };
	struct ifreq *ifr, *end;
	struct u_ifaddrs *ifa, *prv;
	size_t ifrsiz;
	int error;

	*ifs = NULL;

	if ((error = u_socket(&fd, AF_INET, SOCK_DGRAM, PF_UNSPEC, U_CLOEXEC)))
		goto error;

	if ((error = u_getifconf(&ifc, fd)))
		goto error;

	ifr = (struct ifreq *)ifc.ifc_buf;
	end = (struct ifreq *)((char *)ifc.ifc_buf + ifc.ifc_len);

	prv = NULL;

	while (ifr < end) {
		ifrsiz = U_SIZEOF_ADDR_IFREQ(ifr);

		if (!(ifa = calloc(1, sizeof *ifa)))
			goto syerr;

		u_static_assert(sizeof ifa->ifa_name == sizeof ifr->ifr_name, "sizeof ifa_name != sizeof ifr_name");
		memcpy(ifa->ifa_name, ifr->ifr_name, MIN(sizeof ifa->ifa_name, sizeof ifr->ifr_name));

		ifa->ifa_addr = u_sa_copy(&ifa->ifa_ss[0], &ifr->ifr_addr);

		if (-1 != ioctl(fd, SIOCGIFFLAGS, ifr))
			ifa->ifa_flags = ifr->ifr_flags;

		if (ifa->ifa_addr->sa_family == AF_INET6) {
#if defined u_getif6
			//fprintf(stderr, "u_getif6:%s\n", STRINGIFY(u_getif6));
			if (fd6 == -1 && (error = u_socket(&fd6, AF_INET6, SOCK_DGRAM, PF_UNSPEC, U_CLOEXEC)))
				goto error;

			if ((error = u_getif6(ifa, fd6, ifr)))
				goto error;
#endif
		} else {
			if (-1 != ioctl(fd, SIOCGIFNETMASK, ifr) && ifr->ifr_addr.sa_family == ifa->ifa_addr->sa_family)
				ifa->ifa_netmask = u_sa_copy(&ifa->ifa_ss[1], &ifr->ifr_addr);

			if (-1 != ioctl(fd, SIOCGIFDSTADDR, ifr) && ifr->ifr_addr.sa_family == ifa->ifa_addr->sa_family)
				ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], &ifr->ifr_addr);
			else if (-1 != ioctl(fd, SIOCGIFBRDADDR, ifr) && ifr->ifr_addr.sa_family == ifa->ifa_addr->sa_family)
				ifa->ifa_dstaddr = u_sa_copy(&ifa->ifa_ss[2], &ifr->ifr_addr);
		}

		*((prv)? &prv->ifa_next : ifs) = ifa;
		prv = ifa;

		ifr = (struct ifreq *)((char *)ifr + ifrsiz);
	}

	u_close(&fd6);
	u_close(&fd);

	return 0;
syerr:
	error = errno;
error:
	u_close(&fd6);
	u_close(&fd);

	free(ifc.ifc_buf);

	u_freeifaddrs(*ifs);
	*ifs = NULL;

	return error;
} /* u_getifaddrs() */

#endif


#if !HAVE_ARC4RANDOM

#define UNIXL_RANDOM_INITIALIZER { .fd = -1, }

typedef struct unixL_Random {
	int fd;

	unsigned char s[256];
	unsigned char i, j;
	int count;

	pid_t pid;
} unixL_Random;


static void arc4_init(unixL_Random *R) {
	unsigned i;

	memset(R, 0, sizeof *R);

	R->fd = -1;

	for (i = 0; i < sizeof R->s; i++) {
		R->s[i] = i;
	}
} /* arc4_init() */


static void arc4_destroy(unixL_Random *R) {
	u_close(&R->fd);
} /* arc4_destroy() */


static void arc4_addrandom(unixL_Random *R, unsigned char *src, size_t len) {
	unsigned char si;
	int n;

	--R->i;

	for (n = 0; n < 256; n++) {
		++R->i;
		si = R->s[R->i];
		R->j += si + src[n % len];
		R->s[R->i] = R->s[R->j];
		R->s[R->j] = si;
	}

	R->j = R->i;
} /* arc4_addrandom() */


static int arc4_getbyte(unixL_Random *R) {
	unsigned char si, sj;

	++R->i;
	si = R->s[R->i];
	R->j += si;
	sj = R->s[R->j];
	R->s[R->i] = sj;
	R->s[R->j] = si;

	return R->s[(si + sj) & 0xff];
} /* arc4_getbyte() */


static void arc4_stir(unixL_Random *R, int force) {
	unsigned char bytes[128];
	size_t count = 0, n;

	if (R->count > 0 && R->pid == getpid() && !force)
		return;

#if __linux
	{	
		int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_UUID };

		while (count < sizeof bytes) {
			n = sizeof bytes - count;

			if (0 != sysctl(mib, countof(mib), &bytes[count], &n, (void *)0, 0))
				break;

			count += n;
		}

		if (count == sizeof bytes)
			goto stir;
	}
#endif

	{
		if (R->fd == -1 && 0 != u_open(&R->fd, "/dev/urandom", O_RDONLY|U_CLOEXEC, 0))
			goto stir;

		while (count < sizeof bytes) {
			ssize_t n = read(R->fd, &bytes[count], sizeof bytes - count);

			if (n == -1) {
				if (errno == EINTR)
					continue;
				break;
			} else if (n == 0) {
				u_close(&R->fd);

				break;
			}

			count += n;
		}
	}

stir:
	arc4_addrandom(R, bytes, sizeof bytes);

	if (count < sizeof bytes) {
		arc4_addrandom(R, u_memjunk(bytes, sizeof bytes), sizeof bytes);
	}

	for (n = 0; n < 1024; n++)
		arc4_getbyte(R);

	R->count = 1600000;
	R->pid = getpid();
} /* arc4_stir() */


static uint32_t arc4_getword(unixL_Random *R) {
	uint32_t r;

	R->count -= 4;

	arc4_stir(R, 0);

	r = (uint32_t)arc4_getbyte(R) << 24;
	r |= (uint32_t)arc4_getbyte(R) << 16;
	r |= (uint32_t)arc4_getbyte(R) << 8;
	r |= (uint32_t)arc4_getbyte(R);

	return r;
} /* arc4_getword() */

#endif /* !HAVE_ARC4RANDOM */


/*
 * Extends luaL_newmetatable by adding all the relevant fields to the
 * metatable using the standard pattern (placing all the methods in the
 * __index metafield). Leaves the metatable on the stack.
 */
static int unixL_newmetatable(lua_State *L, const char *name, const luaL_Reg *methods, const luaL_Reg *metamethods, int nup) {
	int i, n;

	if (!luaL_newmetatable(L, name))
		return 0;

	/* add metamethods */
	for (i = 0; i < nup; i++)
		lua_pushvalue(L, -1 - nup);

	luaL_setfuncs(L, metamethods, nup);

	/* add methods */
	for (n = 0; methods[n].name; n++)
		;;
	lua_createtable(L, 0, n);

	for (i = 0; i < nup; i++)
		lua_pushvalue(L, -2 - nup);

	luaL_setfuncs(L, methods, nup);

	lua_setfield(L, -2, "__index");

	return 1;
} /* unixL_newmetatable() */


typedef struct unixL_State {
	int error; /* errno value from last failed syscall */

	char errmsg[MIN(NL_TEXTMAX, 512)]; /* NL_TEXTMAX == INT_MAX for glibc */

	struct {
		struct passwd ent;
		char *buf;
		size_t bufsiz;
	} pw;

	struct {
		struct group ent;
		char *buf;
		size_t bufsiz;
	} gr;

	struct {
		int fd[2];
		pid_t pid;
	} ts;

	struct {
		DIR *dp;
		struct dirent *ent;
		size_t bufsiz;
	} dir;

#if !HAVE_ARC4RANDOM
	unixL_Random random;
#endif

#if __APPLE__
	mach_timebase_info_data_t timebase;
#endif
} unixL_State;

static const unixL_State unixL_initializer = {
	.ts = { { -1, -1 } },
#if !HAVE_ARC4RANDOM
	.random = UNIXL_RANDOM_INITIALIZER,
#endif
};


static int unixL_init(unixL_State *U) {
	int error;

	if ((error = u_pipe(U->ts.fd, O_NONBLOCK|U_CLOEXEC)))
		return error;

	U->ts.pid = getpid();

#if !HAVE_ARC4RANDOM
	arc4_init(&U->random);
#endif

#if __APPLE__
	if (KERN_SUCCESS != mach_timebase_info(&U->timebase))
		return (errno)? errno : ENOTSUP;
#endif

	return 0;
} /* unixL_init() */


static void unixL_destroy(unixL_State *U) {
#if !HAVE_ARC4RANDOM
	arc4_destroy(&U->random);
#endif

	free(U->dir.ent);
	U->dir.ent = NULL;
	U->dir.bufsiz = 0;
	U->dir.dp = NULL;

	free(U->gr.buf);
	U->gr.buf = NULL;
	U->gr.bufsiz = 0;

	free(U->pw.buf);
	U->pw.buf = NULL;
	U->pw.bufsiz = 0;

	u_close(&U->ts.fd[0]);
	u_close(&U->ts.fd[1]);
} /* unixL_destroy() */


static unixL_State *unixL_getstate(lua_State *L) {
	return lua_touserdata(L, lua_upvalueindex(1));
} /* unixL_getstate() */


#if !HAVE_ARC4RANDOM
static uint32_t unixL_random(lua_State *L) {
	return arc4_getword(&(unixL_getstate(L))->random);
}
#else
static uint32_t unixL_random(lua_State *L NOTUSED) {
	return arc4random();
}
#endif


static const char *unixL_strerror3(lua_State *L, unixL_State *U, int error) {
	if (0 != strerror_r(error, U->errmsg, sizeof U->errmsg) || U->errmsg[0] == '\0') {
		if (0 > snprintf(U->errmsg, sizeof U->errmsg, "%s: %d", ((error)? "Unknown error" : "Undefined error"), error))
			luaL_error(L, "snprintf failure");
	}

	return U->errmsg;
} /* unixL_strerror3() */


static const char *unixL_strerror(lua_State *L, int error) {
	unixL_State *U = unixL_getstate(L);

	return unixL_strerror3(L, U, error);
} /* unixL_strerror() */


static int unixL_pusherror(lua_State *L, int error, const char *fun NOTUSED, const char *fmt) {
	int top = lua_gettop(L), fc;
	unixL_State *U = unixL_getstate(L);

	U->error = error;

	while ((fc = *fmt++)) {
		switch (fc) {
		case '~':
			lua_pushnil(L);

			break;
		case '#':
			lua_pushnumber(L, error);

			break;
		case '$':
			lua_pushstring(L, unixL_strerror(L, error));

			break;
		case '0':
			lua_pushboolean(L, 0);

			break;
		default:
			break;
		}
	}

	return lua_gettop(L) - top;
} /* unixL_pusherror() */


static u_error_t unixL_readdir(lua_State *L, DIR *dp, struct dirent **ent) {
	unixL_State *U = unixL_getstate(L);

	if (U->dir.dp != dp) {
		long namemax = fpathconf(dirfd(dp), _PC_NAME_MAX);
		size_t bufsiz;

		if (namemax == -1)
			return errno;

		bufsiz = sizeof (struct dirent) + namemax + 1;

		if (bufsiz > U->dir.bufsiz) {
			void *entbuf = realloc(U->dir.ent, bufsiz);

			if (!entbuf)
				return errno;

			U->dir.ent = entbuf;
			U->dir.bufsiz = bufsiz;
		}

		U->dir.dp = dp;
	}

#if _AIX
	/*
	 * AIX sets *ent to NULL and returns EBADF on end-of-directory.
	 * Otherwise on error it doesn't set *ent.
	 *
	 * Saner implementations set *ent to NULL return 0 on
	 * end-of-directory.
	 */
	struct dirent tmp;
	int error;

	*ent = &tmp;

	if ((error = readdir_r(dp, U->dir.ent, ent))) {
		if (error == EBADF && *ent == NULL)
			error = 0;
	}

	return error;
#else
	return readdir_r(dp, U->dir.ent, ent);
#endif
} /* unixL_readdir() */


static u_error_t unixL_closedir(lua_State *L, DIR **dp) {
	unixL_State *U = unixL_getstate(L);
	int error = 0;

	if (*dp) {
		if (U->dir.dp == *dp)
			U->dir.dp = NULL;

		if (0 != closedir(*dp))
			error = errno;

		*dp = NULL;
	}

	return error;
} /* unixL_closedir() */


static int unixL_getpwnam(lua_State *L, const char *user, struct passwd **ent) {
	unixL_State *U = unixL_getstate(L);
	int error;

	*ent = NULL;

	while ((error = getpwnam_r(user, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent))) {
		if (error != ERANGE)
			return error;

		if ((error = u_realloc(&U->pw.buf, &U->pw.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getpwnam() */


static int unixL_getpwuid(lua_State *L, uid_t uid, struct passwd **ent) {
	unixL_State *U = unixL_getstate(L);
	int error;

	*ent = NULL;

	while ((error = getpwuid_r(uid, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent))) {
		if (error != ERANGE)
			return error;

		if ((error = u_realloc(&U->pw.buf, &U->pw.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getpwuid() */


static int unixL_getgrnam(lua_State *L, const char *group, struct group **ent) {
	unixL_State *U = unixL_getstate(L);
	int error;

	*ent = NULL;

	while ((error = getgrnam_r(group, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent))) {
		if (error != ERANGE)
			return error;

		if ((error = u_realloc(&U->gr.buf, &U->gr.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getgrnam() */


static int unixL_getgruid(lua_State *L, gid_t gid, struct group **ent) {
	unixL_State *U = unixL_getstate(L);
	int error;

	*ent = NULL;

	while ((error = getgrgid_r(gid, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent))) {
		if (error != ERANGE)
			return error;

		if ((error = u_realloc(&U->gr.buf, &U->gr.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getgruid() */


static uid_t unixL_optuid(lua_State *L, int index, uid_t def) {
	const char *user;
	struct passwd *pw;
	int error;

	if (lua_isnoneornil(L, index))
		return def;

	if (lua_isnumber(L, index))
		return lua_tonumber(L, index);

	user = luaL_checkstring(L, index);

	if ((error = unixL_getpwnam(L, user, &pw)))
		return luaL_error(L, "%s: %s", user, unixL_strerror(L, error));

	if (!pw)
		return luaL_error(L, "%s: no such user", user);

	return pw->pw_uid;
} /* unixL_optuid() */


static uid_t unixL_checkuid(lua_State *L, int index) {
	luaL_checkany(L, index);

	return unixL_optuid(L, index, -1);
} /* unixL_checkuid() */


static gid_t unixL_optgid(lua_State *L, int index, gid_t def) {
	const char *group;
	struct group *gr;
	int error;

	if (lua_isnoneornil(L, index))
		return def;

	if (lua_isnumber(L, index))
		return lua_tonumber(L, index);

	group = luaL_checkstring(L, index);

	if ((error = unixL_getgrnam(L, group, &gr)))
		return luaL_error(L, "%s: %s", group, unixL_strerror(L, error));

	if (!gr)
		return luaL_error(L, "%s: no such group", group);

	return gr->gr_gid;
} /* unixL_optgid() */


static uid_t unixL_checkgid(lua_State *L, int index) {
	luaL_checkany(L, index);

	return unixL_optgid(L, index, -1);
} /* unixL_checkgid() */


static int ts_reset(unixL_State *U) {
	if (!U->ts.pid || U->ts.pid != getpid()) {
		int error;

		u_close(&U->ts.fd[0]);
		u_close(&U->ts.fd[1]);
		U->ts.pid = 0;

		if ((error = u_pipe(U->ts.fd, O_NONBLOCK|U_CLOEXEC)))
			return error;

		U->ts.pid = getpid();
	} else {
		mode_t mask;

		while (read(U->ts.fd[0], &mask, sizeof mask) > 0)
			;;
	}

	return 0;
} /* ts_reset() */

static mode_t unixL_getumask(lua_State *L) {
	unixL_State *U = unixL_getstate(L);
	pid_t pid;
	mode_t mask;
	int error, status;
	ssize_t n;

	if ((error = ts_reset(U)))
		return luaL_error(L, "getumask: %s", unixL_strerror(L, error));

	switch ((pid = fork())) {
	case -1:
		return luaL_error(L, "getumask: %s", unixL_strerror(L, errno));
	case 0:
		mask = umask(0777);

		if (sizeof mask != write(U->ts.fd[1], &mask, sizeof mask))
			_Exit(1);

		_Exit(0);

		break;
	default:
		while (-1 == waitpid(pid, &status, 0)) {
			if (errno == ECHILD)
				break; /* somebody else caught it */
			else if (errno == EINTR)
				continue;

			return luaL_error(L, "getumask: %s", unixL_strerror(L, errno));
		}

		if (sizeof mask != (n = read(U->ts.fd[0], &mask, sizeof mask)))
			return luaL_error(L, "getumask: %s", (n == -1)? unixL_strerror(L, errno) : "short read");

		return mask;
	}

	return 0;
} /* unixL_getumask() */


/*
 * Rough attempt to match POSIX chmod(2) semantics.
 */
static mode_t unixL_optmode(lua_State *L, int index, mode_t def, mode_t omode) {
	const char *fmt;
	char *end;
	mode_t svtx, cmask, omask, mask, perm, mode;
	int op;

	if (lua_isnoneornil(L, index))
		return def;

	fmt = luaL_checkstring(L, index);

	mode = 07777 & strtoul(fmt, &end, 0);

	if (*end == '\0' && end != fmt)
		return mode;

	svtx = (S_ISDIR(omode))? 01000 : 0000;
	cmask = 0;
	mode = 0;
	mask = 0;

	while (*fmt) {
		omask = ~01000 & mask;
		mask = 0;
		op = 0;
		perm = 0;

		for (; *fmt; ++fmt) {
			switch (*fmt) {
			case 'u':
				mask |= 04700;

				continue;
			case 'g':
				mask |= 02070;

				continue;
			case 'o':
				mask |= 00007; /* no svtx/sticky bit */

				continue;
			case 'a':
				mask |= 06777 | svtx;

				continue;
			case '+':
			case '-':
			case '=':
				op = *fmt++;

				goto perms;
			case ',':
				omask = 0;

				continue;
			default:
				continue;
			} /* switch() */
		} /* for() */

perms:
		for (; *fmt; ++fmt) {
			switch (*fmt) {
			case 'r':
				perm |= 00444;

				continue;
			case 'w':
				perm |= 00222;

				continue;
			case 'x':
				perm |= 00111;

				continue;
			case 'X':
				if (S_ISDIR(omode) || (omode & 00111))
					perm |= 00111;

				continue;
			case 's':
				perm |= 06000;

				continue;
			case 't':
				perm |= 01000;

				continue;
			case 'u':
				perm |= (00700 & omode);
				perm |= (00700 & omode) >> 3;
				perm |= (00700 & omode) >> 6;

				continue;
			case 'g':
				perm |= (00070 & omode) << 3;
				perm |= (00070 & omode);
				perm |= (00070 & omode) >> 3;

				continue;
			case 'o':
				perm |= (00007 & omode);
				perm |= (00007 & omode) << 3;
				perm |= (00007 & omode) << 6;

				continue;
			default:
				if (isspace((unsigned char)*fmt))
					continue;

				goto apply;
			} /* switch() */
		} /* for() */

apply:
		if (!mask) {
			if (!omask) {
				if (!cmask) {
					/* only query once */
					cmask = 01000 | (0777 & unixL_getumask(L));
				}

				omask = 0777 & ~(~01000 & cmask);
			}

			mask = svtx | omask;
		}

		switch (op) {
		case '+':
			mode |= mask & perm;

			break;
		case '-':
			mode &= ~(mask & perm);

			break;
		case '=':
			mode = mask & perm;

			break;
		default:
			break;
		}
	} /* while() */

	return mode;
} /* unixL_optmode() */


static int unixL_optfileno(lua_State *L, int index, int def) {
	luaL_Stream *fh;
	DIR **dp;
	int fd;

	if ((fh = luaL_testudata(L, index, LUA_FILEHANDLE))) {
		luaL_argcheck(L, fh->f != NULL, index, "attempt to use a closed file");

		fd = fileno(fh->f);

		luaL_argcheck(L, fd >= 0, index, "attempt to use irregular file (no descriptor)");

		return fd;
	}

	if ((dp = luaL_testudata(L, index, "DIR*"))) {
		luaL_argcheck(L, *dp != NULL, index, "attempt to use a closed directory");

		fd = dirfd(*dp);

		luaL_argcheck(L, fd >= 0, index, "attempt to use irregular directory (no descriptor)");

		return fd;
	}

	return def;
} /* unixL_optfileno() */


static int unixL_optfint(lua_State *L, int index, const char *name, int def) {
	int i;

	lua_getfield(L, index, name);
	i = (lua_isnil(L, -1))? def : luaL_checkint(L, -1);
	lua_pop(L, 1);

	return i;
} /* unixL_optfint() */


static struct tm *unixL_checktm(lua_State *L, int index, struct tm *tm) {
	luaL_checktype(L, 1, LUA_TTABLE);

	tm->tm_year = unixL_optfint(L, index, "year", tm->tm_year + 1900) - 1900;
	tm->tm_mon = unixL_optfint(L, index, "month", tm->tm_mon + 1) - 1;
	tm->tm_mday = unixL_optfint(L, index, "day", tm->tm_mday);
	tm->tm_hour = unixL_optfint(L, index, "hour", tm->tm_hour);
	tm->tm_min = unixL_optfint(L, index, "min", tm->tm_min);
	tm->tm_sec = unixL_optfint(L, index, "sec", tm->tm_sec);
	tm->tm_wday = unixL_optfint(L, index, "wday", tm->tm_wday + 1) - 1;
	tm->tm_yday = unixL_optfint(L, index, "yday", tm->tm_yday + 1) - 1;

	lua_getfield(L, 1, "isdst");
	if (!lua_isnil(L, -1)) {
		tm->tm_isdst = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);

	return tm;
} /* unixL_checktm() */


static struct tm *unixL_opttm(lua_State *L, int index, const struct tm *def, struct tm *tm) {
	if (lua_isnoneornil(L, index)) {
		if (def) {
			*tm = *def;
		} else {
			time_t now = time(NULL);

			gmtime_r(&now, tm);
		}

		return tm;
	} else {
		return unixL_checktm(L, index, tm);
	}
} /* unixL_opttm() */


#if __APPLE__
#define U_CLOCK_REALTIME  1
#define U_CLOCK_MONOTONIC 2
#else
#define U_CLOCK_REALTIME  CLOCK_REALTIME
#define U_CLOCK_MONOTONIC CLOCK_MONOTONIC
#endif

static int unixL_optclockid(lua_State *L, int index, int def) {
	const char *id;

	if (lua_isnoneornil(L, index))
		return def;
	if (lua_isnumber(L, index))
		return luaL_checkint(L, index);

	id = luaL_checkstring(L, index);

	switch (*((*id == '*')? id+1 : id)) {
	case 'r':
		return U_CLOCK_REALTIME;
	case 'm':
		return U_CLOCK_MONOTONIC;
	default:
		return luaL_argerror(L, index, lua_pushfstring(L, "%s: invalid clock", id));
	}
} /* unixL_optclockid() */


static int unix_arc4random(lua_State *L) {
	lua_pushnumber(L, unixL_random(L));

	return 1;
} /* unix_arc4random() */


static int unix_arc4random_buf(lua_State *L) {
	size_t count = luaL_checkinteger(L, 1), n = 0;
	union {
		uint32_t r[16];
		unsigned char c[16 * sizeof (uint32_t)];
	} tmp;
	luaL_Buffer B;

	luaL_buffinit(L, &B);

	while (n < count) {
		size_t m = MIN((size_t)(count - n), sizeof tmp.c);
		size_t i = howmany(m, sizeof tmp.r);

		while (i-- > 0) {
			tmp.r[i] = unixL_random(L);
		}

		luaL_addlstring(&B, (char *)tmp.c, m);
		n += m;
	}

	luaL_pushresult(&B);

	return 1;
} /* unix_arc4random_buf() */


static int unix_arc4random_stir(lua_State *L) {
#if HAVE_ARC4RANDOM
#if __APPLE__ || (__FreeBSD__ && !FREEBSD_PREREQ(10,0))
	/*
	 * Apple's arc4random always uses /dev/urandom, whereas the BSDs
	 * support a chroot-safe sysctl method.
	 */
	char junk[128];
	arc4random_addrandom(u_memjunk(junk, sizeof junk), sizeof junk);
#endif
	arc4random_stir();
#else
	arc4_stir(&(unixL_getstate(L))->random, 1);
#endif

	lua_pushboolean(L, 1);

	return 1;
} /* unix_arc4random_stir() */


static int unix_arc4random_uniform(lua_State *L) {
	lua_Number modn = luaL_optnumber(L, 1, 4294967296.0);

	if (modn >= 4294967296.0) {
		lua_pushnumber(L, unixL_random(L));
	} else {
		uint32_t n = (uint32_t)modn;
		uint32_t r, min;

		min = -n % n;

		for (;;) {
			r = unixL_random(L);

			if (r >= min)
				break;
		}

		lua_pushnumber(L, r % n);
	}

	return 1;
} /* unix_arc4random_uniform() */


static int unix_chdir(lua_State *L) {
	int fd;

	if (-1 != (fd = unixL_optfileno(L, 1, -1))) {
		if (0 != fchdir(fd))
			return unixL_pusherror(L, errno, "chdir", "0$#");
	} else {
		const char *path = luaL_checkstring(L, 1);

		if (0 != chdir(path))
			return unixL_pusherror(L, errno, "chdir", "0$#");
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_chdir() */


static int unix_chown(lua_State *L) {
	uid_t uid = unixL_optuid(L, 2, -1);
	gid_t gid = unixL_optgid(L, 3, -1);
	int fd;

	if (-1 != (fd = unixL_optfileno(L, 1, -1))) {
		if (0 != fchown(fd, uid, gid))
			return unixL_pusherror(L, errno, "chown", "0$#");
	} else {
		const char *path = luaL_checkstring(L, 1);

		if (0 != chown(path, uid, gid))
			return unixL_pusherror(L, errno, "chown", "0$#");
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_chown() */


static int unix_chroot(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != chroot(path))
		return unixL_pusherror(L, errno, "chroot", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_chroot() */


static int unix_clock_gettime(lua_State *L) {
#if __APPLE__
	unixL_State *U = unixL_getstate(L);
	int id = unixL_optclockid(L, 1, U_CLOCK_REALTIME);
	struct timeval tv;
	struct timespec ts;
	uint64_t abt;

	switch (id) {
	case U_CLOCK_REALTIME:
		if (0 != gettimeofday(&tv, NULL))
			return unixL_pusherror(L, errno, "clock_gettime", "~$#");

		TIMEVAL_TO_TIMESPEC(&tv, &ts);

		break;
	case U_CLOCK_MONOTONIC:
		abt = mach_absolute_time();
		abt = abt * U->timebase.numer / U->timebase.denom;

		ts.tv_sec = abt / 1000000000L;
		ts.tv_nsec = abt % 1000000000L;

		break;
	default:
		return luaL_argerror(L, 1, "invalid clock");
	}
#else
	int id = unixL_optclockid(L, 1, U_CLOCK_REALTIME);
	struct timespec ts;

	if (0 != clock_gettime(id, &ts))
		return unixL_pusherror(L, errno, "clock_gettime", "~$#");
#endif

	if (lua_isnoneornil(L, 2) || !lua_toboolean(L, 2)) {
		lua_pushnumber(L, (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000L));

		return 1;
	} else {
		lua_pushinteger(L, ts.tv_sec);
		lua_pushinteger(L, ts.tv_nsec);

		return 2;
	}
} /* unix_clock_gettime() */


static int dir_close(lua_State *);

static int unix_closedir(lua_State *L) {
	return dir_close(L);
} /* unix_closedir() */


static int unix_getegid(lua_State *L) {
	lua_pushnumber(L, getegid());

	return 1;
} /* unix_getegid() */


static int unix_geteuid(lua_State *L) {
	lua_pushnumber(L, geteuid());

	return 1;
} /* unix_geteuid() */


static int unix_getmode(lua_State *L) {
	const char *fmt;
	char *end;
	mode_t omode;

	fmt = luaL_optstring(L, 2, "0777");
	omode = 07777 & strtoul(fmt, &end, 0);

	lua_pushnumber(L, unixL_optmode(L, 1, 0777, omode));

	return 1;
} /* unix_getmode() */


static int unix_getgid(lua_State *L) {
	lua_pushnumber(L, getgid());

	return 1;
} /* unix_getgid() */


static void gr_pushmem(lua_State *L, char **list, int create) {
	if (list) {
		int i;

		for (i = 0; list[i]; i++)
			;;

		if (create)
			lua_createtable(L, i, 0);

		for (i = 0; list[i]; i++) {
			lua_pushstring(L, list[i]);
			lua_rawseti(L, -2, i + 1);
		}
	} else {
		if (create)
			lua_createtable(L, 0, 0);
	}
} /* gr_pushmem() */

static int unix_getgrnam(lua_State *L) {
	struct group *ent;
	int error;

	if (lua_isnumber(L, 1)) {
		error = unixL_getgruid(L, luaL_checkint(L, 1), &ent);
	} else {
		error = unixL_getgrnam(L, luaL_checkstring(L, 1), &ent);
	}

	if (error) {
		return unixL_pusherror(L, error, "getgrnam", "~$#");
	} else if (!ent) {
		lua_pushnil(L);
		lua_pushstring(L, "no such group");

		return 2;
	}

	if (lua_isnoneornil(L, 2)) {
		lua_createtable(L, 0, 4);

		if (ent->gr_name) {
			lua_pushstring(L, ent->gr_name);
			lua_setfield(L, -2, "name");
		}

		if (ent->gr_passwd) {
			lua_pushstring(L, ent->gr_passwd);
			lua_setfield(L, -2, "passwd");
		}

		lua_pushinteger(L, ent->gr_gid);
		lua_setfield(L, -2, "gid");

		gr_pushmem(L, ent->gr_mem, 0);

		return 1;
	} else {
		static const char *opts[] = {
			"name", "passwd", "gid", "mem", "members", NULL,
		};
		int i, n = 0, top = lua_gettop(L);

		for (i = 2; i <= top; i++) {
			switch (luaL_checkoption(L, i, NULL, opts)) {
			case 0: /* name */
				if (ent->gr_name)
					lua_pushstring(L, ent->gr_name);
				else
					lua_pushnil(L);
				++n;

				break;
			case 1: /* passwd */
				if (ent->gr_passwd)
					lua_pushstring(L, ent->gr_passwd);
				else
					lua_pushnil(L);
				++n;

				break;
			case 2: /* gid */
				lua_pushinteger(L, ent->gr_gid);
				++n;

				break;
			case 3: /* mem */
			case 4: /* members */
				gr_pushmem(L, ent->gr_mem, 1);
				++n;

				break;
			}
		}

		return n;
	}
} /* unix_getgrnam() */


enum ifs_field {
	IF_NAME,
	IF_FLAGS,
	IF_ADDR,
	IF_NETMASK,
	IF_DSTADDR,
	IF_BROADADDR,
	IF_DATA,
	IF_FAMILY,
}; /* enum ifs_field */

static const char *ifs_field[] = { "name", "flags", "addr", "netmask", "dstaddr", "broadaddr", "data", "family", NULL };


static int ifs_pushaddr(lua_State *L, struct sockaddr *sa) {
	char host[NI_MAXHOST + 1];
	int error;

	if ((error = getnameinfo(sa, u_sa_len(sa), host, sizeof host, NULL, 0, NI_NUMERICHOST))) {
		//return luaL_error(L, "getnameinfo: %s", gai_strerror(error));
		lua_pushnil(L);
	} else {
		lua_pushstring(L, host);
	}

	return 1;
} /* ifs_pushaddr() */


static void ifs_pushfield(lua_State *L, const struct u_ifaddrs *ifa, enum ifs_field type) {
	switch (type) {
	case IF_NAME:
		lua_pushstring(L, ifa->ifa_name);
		break;
	case IF_FLAGS:
		lua_pushinteger(L, ifa->ifa_flags);
		break;
	case IF_ADDR:
		if (ifa->ifa_addr) {
			ifs_pushaddr(L, ifa->ifa_addr);
		} else {
			lua_pushnil(L);
		}
		break;
	case IF_NETMASK:
		if (ifa->ifa_netmask) {
			ifs_pushaddr(L, ifa->ifa_netmask);
		} else {
			lua_pushnil(L);
		}
		break;
	case IF_BROADADDR:
		/* FALL THROUGH */
	case IF_DSTADDR:
		if (ifa->ifa_dstaddr) {
			ifs_pushaddr(L, ifa->ifa_dstaddr);
		} else {
			lua_pushnil(L);
		}
		break;
	case IF_DATA:
		lua_pushnil(L);
		break;
	case IF_FAMILY:
		if (ifa->ifa_addr) {
			lua_pushinteger(L, ifa->ifa_addr->sa_family);
		} else {
			lua_pushnil(L);
		}
		break;
	default:
		lua_pushnil(L);
		break;
	}
} /* ifs_pushfield() */


static void ifs_pushtable(lua_State *L, const struct u_ifaddrs *ifa) {
	lua_createtable(L, 0, 7);

	ifs_pushfield(L, ifa, IF_NAME);
	lua_setfield(L, -2, "name");

	ifs_pushfield(L, ifa, IF_FLAGS);
	lua_setfield(L, -2, "flags");

	ifs_pushfield(L, ifa, IF_ADDR);
	lua_setfield(L, -2, "addr");

	ifs_pushfield(L, ifa, IF_NETMASK);
	lua_setfield(L, -2, "netmask");

#if defined IFF_BROADCAST
	if (ifa->ifa_flags & IFF_BROADCAST) {
		ifs_pushfield(L, ifa, IF_BROADADDR);
		lua_setfield(L, -2, "broadaddr");
	} else {
		ifs_pushfield(L, ifa, IF_DSTADDR);
		lua_setfield(L, -2, "dstaddr");
	}
#else
	ifs_pushfield(L, ifa, IF_DSTADDR);
	lua_setfield(L, -2, "dstaddr");
#endif

	ifs_pushfield(L, ifa, IF_DATA);
	lua_setfield(L, -2, "data");

	ifs_pushfield(L, ifa, IF_FAMILY);
	lua_setfield(L, -2, "family");
} /* ifs_pushtable() */


static int ifs_nextif(lua_State *L) {
	struct u_ifaddrs *ifa = lua_touserdata(L, lua_upvalueindex(2));

	if (!ifa)
		return 0;

	lua_pushlightuserdata(L, ifa->ifa_next);
	lua_replace(L, lua_upvalueindex(2));

	if (lua_isnone(L, lua_upvalueindex(3))) {
		ifs_pushtable(L, ifa);

		return 1;
	} else {
		int i;

		for (i = 3; !lua_isnone(L, lua_upvalueindex(i)); i++) {
			ifs_pushfield(L, ifa, luaL_checkoption(L, lua_upvalueindex(i), NULL, ifs_field));
		}

		return i - 3;
	}
} /* ifs_nextif() */


static int unix_getifaddrs(lua_State *L) {
	struct u_ifaddrs **ifs;
	int error;

	ifs = lua_newuserdata(L, sizeof *ifs);
	*ifs = NULL;
	luaL_setmetatable(L, "struct ifaddrs*");

	if ((error = u_getifaddrs(ifs)))
		return unixL_pusherror(L, error, "getifaddrs", "~$#");

	lua_insert(L, 1);

	lua_pushlightuserdata(L, *ifs);
	lua_insert(L, 2);

	lua_pushcclosure(L, &ifs_nextif, lua_gettop(L));

	return 1;
} /* unix_getifaddrs() */


static int ifs__gc(lua_State *L) {
	struct u_ifaddrs **ifs = luaL_checkudata(L, 1, "struct ifaddrs*");

	if (*ifs) {
		u_freeifaddrs(*ifs);
		*ifs = NULL;
	}

	return 0;
} /* ifs__gc() */


static const luaL_Reg ifs_methods[] = {
	{ NULL, NULL }
}; /* ifs_methods[] */


static const luaL_Reg ifs_metamethods[] = {
	{ "__gc", &ifs__gc },
	{ NULL,   NULL }
}; /* ifs_metamethods[] */


static int unix_getpid(lua_State *L) {
	lua_pushnumber(L, getpid());

	return 1;
} /* unix_getpid() */


static int unix_getpwnam(lua_State *L) {
	struct passwd *ent;
	int error;

	if (lua_isnumber(L, 1)) {
		error = unixL_getpwuid(L, luaL_checkint(L, 1), &ent);
	} else {
		error = unixL_getpwnam(L, luaL_checkstring(L, 1), &ent);
	}

	if (error) {
		return unixL_pusherror(L, error, "getpwnam", "~$#");
	} else if (!ent) {
		lua_pushnil(L);
		lua_pushstring(L, "no such user");

		return 2;
	}

	if (lua_isnoneornil(L, 2)) {
		lua_createtable(L, 0, 7);

		if (ent->pw_name) {
			lua_pushstring(L, ent->pw_name);
			lua_setfield(L, -2, "name");
		}

		if (ent->pw_passwd) {
			lua_pushstring(L, ent->pw_passwd);
			lua_setfield(L, -2, "passwd");
		}

		lua_pushinteger(L, ent->pw_uid);
		lua_setfield(L, -2, "uid");

		lua_pushinteger(L, ent->pw_gid);
		lua_setfield(L, -2, "gid");

		if (ent->pw_dir) {
			lua_pushstring(L, ent->pw_dir);
			lua_setfield(L, -2, "dir");
		}

		if (ent->pw_shell) {
			lua_pushstring(L, ent->pw_shell);
			lua_setfield(L, -2, "shell");
		}

		if (ent->pw_gecos) {
			lua_pushstring(L, ent->pw_gecos);
			lua_setfield(L, -2, "gecos");
		}

		return 1;
	} else {
		static const char *opts[] = {
			"name", "passwd", "uid", "gid", "dir", "shell", "gecos", NULL,
		};
		int i, n = 0, top = lua_gettop(L);

		for (i = 2; i <= top; i++) {
			switch (luaL_checkoption(L, i, NULL, opts)) {
			case 0:
				if (ent->pw_name)
					lua_pushstring(L, ent->pw_name);
				else
					lua_pushnil(L);
				++n;

				break;
			case 1:
				if (ent->pw_passwd)
					lua_pushstring(L, ent->pw_passwd);
				else
					lua_pushnil(L);
				++n;

				break;
			case 2:
				lua_pushinteger(L, ent->pw_uid);
				++n;

				break;
			case 3:
				lua_pushinteger(L, ent->pw_gid);
				++n;

				break;
			case 4:
				if (ent->pw_dir)
					lua_pushstring(L, ent->pw_dir);
				else
					lua_pushnil(L);
				++n;

				break;
			case 5:
				if (ent->pw_shell)
					lua_pushstring(L, ent->pw_shell);
				else
					lua_pushnil(L);
				++n;

				break;
			case 6:
				if (ent->pw_gecos)
					lua_pushstring(L, ent->pw_gecos);
				else
					lua_pushnil(L);
				++n;

				break;
			}
		}

		return n;
	}
} /* unix_getpwnam() */


static int unix_gettimeofday(lua_State *L) {
	struct timeval tv;

	if (0 != gettimeofday(&tv, NULL))
		return unixL_pusherror(L, errno, "gettimeofday", "~$#");

	if (lua_isnoneornil(L, 1) || !lua_toboolean(L, 1)) {
		lua_pushnumber(L, (double)tv.tv_sec + ((double)tv.tv_usec / 1000000L));

		return 1;
	} else {
		lua_pushinteger(L, tv.tv_sec);
		lua_pushinteger(L, tv.tv_usec);

		return 2;
	}
} /* unix_gettimeofday() */


static int unix_getuid(lua_State *L) {
	lua_pushnumber(L, getuid());

	return 1;
} /* unix_getuid() */


#if HAVE_GETAUXVAL
#include <sys/auxv.h>
#endif

static int unix_issetugid_other(lua_State *L) {
	lua_pushboolean(L, (geteuid() != getuid()) || (getegid() != getgid()));

	return 0;
} /* unix_issetugid_other() */

static int unix_issetugid_linux(lua_State *L) {
#if HAVE_GETAUXVAL && defined AT_SECURE
	unsigned long auxval;

	errno = 0;
	auxval = getauxval(AT_SECURE);

	if (auxval != 0 || errno != ENOENT) {
		lua_pushboolean(L, !auxval);

		return 1;
	}
#endif

#if GNUC_PREREQ(2, 1) /* __libc_enable_secure added between 2.0.98 and 2.0.99 */
	extern int __libc_enable_secure;

	lua_pushboolean(L, __libc_enable_secure);

	return 1;
#else
	return unix_issetugid_other(L);
#endif
} /* unix_issetugid_linux() */

static int unix_issetugid(lua_State *L) {
#if HAVE_ISSETUGID
	lua_pushboolean(L, issetugid());

	return 1;
#elif __linux
	return unix_issetugid_linux(L);
#else
	return unix_issetugid_other(L);
#endif
} /* unix_issetugid() */


static int unix_link(lua_State *L) {
	const char *src = luaL_checkstring(L, 1);
	const char *dst = luaL_checkstring(L, 2);

	if (0 != link(src, dst))
		return unixL_pusherror(L, errno, "link", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_link() */


/*
 * Emulate mkdir except with well-defined SUID, SGID, SVTIX behavior. If you
 * want to set bits restricted by the umask you must manually use chmod.
 */
static int unix_mkdir(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	mode_t cmask, mode;

	cmask = unixL_getumask(L);
	mode = 0777 & ~cmask;
	mode = unixL_optmode(L, 2, mode, mode) & ~cmask;

	if (0 != mkdir(path, 0700 & mode) || 0 != chmod(path, mode))
		return unixL_pusherror(L, errno, "mkdir", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_mkdir() */


/*
 * Patterned after the mkpath routine from BSD mkdir implementations for
 * POSIX mkdir(1). The basic idea is to mimic a recursive mkdir(2) call.
 *
 * Differences from BSD mkpath:
 *
 * 1) On BSD intermediate permissions are always (0300 | (0777 & ~umask())).
 *    But see #2. Whereas here we obey any specified intermediate mode
 *    value.
 *
 * 2) On BSD if the SUID, SGID, or SVTIX bit is set in the target mode
 *    value, the target directory is chmod'd using that mode value,
 *    unaltered by the umask. On OpenBSD intermediate directories are also
 *    chmod'd with that mode value.
 */
static int unix_mkpath(lua_State *L) {
	size_t len;
	const char *path = luaL_checklstring(L, 1, &len);
	mode_t cmask, mode, imode, _mode;
	char *dir, *slash;
	int lc;

	cmask = unixL_getumask(L);
	mode = 0777 & ~cmask;
	imode = 0300 | mode;

	mode = unixL_optmode(L, 2, mode, mode) & ~cmask;
	imode = unixL_optmode(L, 3, imode, imode) & ~cmask;

	dir = lua_newuserdata(L, len + 1);
	memcpy(dir, path, len + 1);

	slash = dir + len;
	while (--slash > dir && *slash == '/')
		*slash = '\0';

	slash = dir;

	while (*slash) {
		slash += strspn(slash, "/");
		slash += strcspn(slash, "/");

		lc = *slash;
		*slash = '\0';

		_mode = (lc == '\0')? mode : imode;

		if (0 == mkdir(dir, 0700 & _mode)) {
			if (0 != chmod(dir, _mode))
				return unixL_pusherror(L, errno, "mkpath", "0$#");
		} else {
			int error = errno;
			struct stat st;

			if (0 != stat(dir, &st))
				return unixL_pusherror(L, error, "mkpath", "0$#");

			if (!S_ISDIR(st.st_mode))
				return unixL_pusherror(L, ENOTDIR, "mkpath", "0$#");
		}

		*slash = lc;
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_mkpath() */



static DIR *dir_checkself(lua_State *L, int index) {
	DIR **dp = luaL_checkudata(L, index, "DIR*");

	luaL_argcheck(L, *dp != NULL, index, "attempt to use a closed directory");

	return *dp;
} /* dir_checkself() */


enum dir_field {
	DF_NAME,
	DF_INO,
	DF_TYPE
}; /* enum dir_field */

static const char *dir_field[] = { "name", "ino", "type", NULL };


static void dir_pushfield(lua_State *L, struct dirent *ent, enum dir_field type) {
	switch (type) {
	case DF_NAME:
		lua_pushstring(L, ent->d_name);
		break;
	case DF_INO:
		lua_pushinteger(L, ent->d_ino);
		break;
	case DF_TYPE:
#if defined DTTOIF
		lua_pushinteger(L, DTTOIF(ent->d_type));
#else
		lua_pushnil(L);
#endif
		break;
	default:
		lua_pushnil(L);
		break;
	} /* switch() */
} /* dir_pushfield() */


static void dir_pushtable(lua_State *L, struct dirent *ent) {
	lua_createtable(L, 0, 3);

	dir_pushfield(L, ent, DF_NAME);
	lua_setfield(L, -2, "name");

	dir_pushfield(L, ent, DF_INO);
	lua_setfield(L, -2, "ino");

	dir_pushfield(L, ent, DF_TYPE);
	lua_setfield(L, -2, "type");
} /* dir_pushtable() */


static int dir_read(lua_State *L) {
	DIR *dp = dir_checkself(L, 1);
	struct dirent *ent = NULL;
	int error;

	if ((error = unixL_readdir(L, dp, &ent)))
		return unixL_pusherror(L, error, "readdir", "~$#");

	if (!ent)
		return 0;

	if (lua_isnoneornil(L, 2)) {
		dir_pushtable(L, ent);

		return 1;
	} else {
		int i, n = 0, top = lua_gettop(L);

		for (i = 2; i <= top; i++, n++) {
			dir_pushfield(L, ent, luaL_checkoption(L, i, NULL, dir_field));
		}

		return n;
	}
} /* dir_read() */


static int dir_nextent(lua_State *L) {
	DIR *dp = dir_checkself(L, lua_upvalueindex(2));
	int nup = lua_tointeger(L, lua_upvalueindex(3));
	struct dirent *ent = NULL;
	int i, error;

	if ((error = unixL_readdir(L, dp, &ent)))
		return luaL_error(L, "readdir: %s", unixL_strerror(L, error));

	if (!ent)
		return 0;

	if (nup < 4) {
		dir_pushtable(L, ent);

		return 1;
	} else {
		int i, n = 0;

		for (i = 4; i <= nup; i++, n++) {
			dir_pushfield(L, ent, luaL_checkoption(L, lua_upvalueindex(i), NULL, dir_field));
		}

		return n;
	}
} /* dir_nextent() */


static int dir_files(lua_State *L) {
	DIR *dp = dir_checkself(L, 1);
	int i, top = lua_gettop(L), nup = top + 2;

	lua_pushvalue(L, lua_upvalueindex(1)); /* unixL_State */
	lua_pushvalue(L, 1);
	lua_pushinteger(L, nup);

	for (i = 2; i <= top; i++) {
		lua_pushvalue(L, i);
	}

	lua_pushcclosure(L, &dir_nextent, nup);

	return 1;
} /* dir_files() */


static int dir_rewind(lua_State *L) {
	DIR *dp = dir_checkself(L, 1);

	rewinddir(dp);

	lua_pushboolean(L, 1);

	return 1;
} /* dir_rewind() */


static int dir_close(lua_State *L) {
	DIR **dp = luaL_checkudata(L, 1, "DIR*");
	int error;

	if ((error = unixL_closedir(L, dp)))
		return luaL_error(L, "closedir: %s", unixL_strerror(L, error));

	lua_pushboolean(L, 1);

	return 1;
} /* dir_close() */


static const luaL_Reg dir_methods[] = {
	{ "read",   &dir_read },
	{ "files",  &dir_files },
	{ "rewind", &dir_rewind },
	{ "close",  &dir_close },
	{ NULL,     NULL }
}; /* dir_methods[] */


static const luaL_Reg dir_metamethods[] = {
	{ "__gc", &dir_close },
	{ NULL,   NULL }
}; /* dir_metamethods[] */


static int unix_opendir(lua_State *L) {
	DIR **dp;
	int fd, fd2 = -1, error;

	lua_settop(L, 1);

	dp = lua_newuserdata(L, sizeof *dp);
	*dp = NULL;
	luaL_setmetatable(L, "DIR*");

	if (-1 != (fd = unixL_optfileno(L, 1, -1))) {
		/*
		 * There's no simple way to just duplicate the descriptor
		 * and atomically set O_CLOEXEC. The dup3() extension
		 * requires a valid target descriptor.
		 */
		 if ((error = u_open(&fd2, ".", O_RDONLY|U_CLOEXEC, 0)))
		 	goto error;

		if ((error = u_dup3(fd, fd2, U_CLOEXEC)))
			goto error;

		if (-1 == lseek(fd2, 0, SEEK_SET))
			goto syerr;

		if ((error = u_fdopendir(dp, fd2)))
			goto error;
	} else {
		const char *path = luaL_checkstring(L, 1);

		if (!(*dp = opendir(path)))
			goto syerr;
	}

	return 1;
syerr:
	error = errno;
error:
	u_close(&fd2);

	return unixL_pusherror(L, error, "opendir", "~$#");
} /* unix_opendir() */


static int unix_readdir(lua_State *L) {
	return dir_read(L);
} /* unix_readdir() */


static int unix_rename(lua_State *L) {
	const char *opath = luaL_checkstring(L, 1);
	const char *npath = luaL_checkstring(L, 2);

	if (0 != rename(opath, npath))
		return unixL_pusherror(L, errno, "rename", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_rename() */


static int unix_rewinddir(lua_State *L) {
	return dir_rewind(L);
} /* unix_rewinddir() */


#define unix_S_IFTEST(L, test) do { \
	int mode = luaL_optinteger(L, 1, 0); \
	lua_pushboolean(L, test(mode)); \
	return 1; \
} while (0)

static int unix_S_ISBLK(lua_State *L) {
	unix_S_IFTEST(L, S_ISBLK);
} /* unix_S_ISBLK() */


static int unix_S_ISCHR(lua_State *L) {
	unix_S_IFTEST(L, S_ISCHR);
} /* unix_S_ISCHR() */


static int unix_S_ISDIR(lua_State *L) {
	unix_S_IFTEST(L, S_ISDIR);
} /* unix_S_ISDIR() */


static int unix_S_ISFIFO(lua_State *L) {
	unix_S_IFTEST(L, S_ISFIFO);
} /* unix_S_ISFIFO() */


static int unix_S_ISREG(lua_State *L) {
	unix_S_IFTEST(L, S_ISREG);
} /* unix_S_ISREG() */


static int unix_S_ISLNK(lua_State *L) {
	unix_S_IFTEST(L, S_ISLNK);
} /* unix_S_ISLNK() */


static int unix_S_ISSOCK(lua_State *L) {
	unix_S_IFTEST(L, S_ISSOCK);
} /* unix_S_ISSOCK() */


static int unix_rmdir(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != rmdir(path))
		return unixL_pusherror(L, errno, "rmdir", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_rmdir() */


static int unix_setegid(lua_State *L) {
	gid_t gid = unixL_checkgid(L, 1);

	if (0 != setegid(gid))
		return unixL_pusherror(L, errno, "setegid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setegid() */


static int unix_seteuid(lua_State *L) {
	uid_t uid = unixL_checkuid(L, 1);

	if (0 != seteuid(uid))
		return unixL_pusherror(L, errno, "seteuid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_seteuid() */


static int unix_setgid(lua_State *L) {
	gid_t gid = unixL_checkgid(L, 1);

	if (0 != setgid(gid))
		return unixL_pusherror(L, errno, "setgid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setgid() */


static int unix_setsid(lua_State *L) {
	pid_t pg;

	if (-1 == (pg = setsid()))
		return unixL_pusherror(L, errno, "setsid", "~$#");

	lua_pushnumber(L, pg);

	return 1;
} /* unix_setsid() */


static int unix_setuid(lua_State *L) {
	uid_t uid = unixL_checkuid(L, 1);

	if (0 != setuid(uid))
		return unixL_pusherror(L, errno, "setuid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setuid() */


static int unix_symlink(lua_State *L) {
	const char *src = luaL_checkstring(L, 1);
	const char *dst = luaL_checkstring(L, 2);

	if (0 != symlink(src, dst))
		return unixL_pusherror(L, errno, "symlink", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_symlink() */


static int yr_isleap(int year) {
	if (year >= 0)
		return !(year % 4) && ((year % 100) || !(year % 400));
	else
		return yr_isleap(-(year + 1));
} /* yr_isleap() */


static int tm_yday(const struct tm *tm) {
	static const int past[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	int yday;

	if (tm->tm_yday)
		return tm->tm_yday;
	
	yday = past[CLAMP(tm->tm_mon, 0, 11)] + CLAMP(tm->tm_mday, 1, 31) - 1;

	return yday + (tm->tm_mon > 1 && yr_isleap(1900 + tm->tm_year));
} /* tm_yday() */


static int yr_nleaps(int year) {
	if (year >= 0)
		return (year / 400) + (year / 4) - (year / 100);
	else
		return -(yr_nleaps(-(year + 1)) + 1);
} /* yr_nleaps() */


static double tm2unix(const struct tm *tm) {
	int year = tm->tm_year + 1900;
	double ts;

	ts = 86400.0 * 365.0 * (year - 1970);
	ts += 86400.0 * (yr_nleaps(year - 1) - yr_nleaps(1969));
	ts += 86400 * tm_yday(tm);
	ts += 3600 * tm->tm_hour;
	ts += 60 * tm->tm_min;
	ts += CLAMP(tm->tm_sec, 0, 59);

	return ts;
} /* tm2unix() */


static int unix_timegm(lua_State *L) {
	struct tm tm = { 0 };

	unixL_opttm(L, 1, NULL, &tm);

	lua_pushnumber(L, tm2unix(&tm));

	return 1;
} /* unix_timegm() */


static int unix_truncate(lua_State *L) {
	const char *path;
	int fd;
	off_t len;

	/* TODO: check overflow */
	len = (off_t)luaL_optnumber(L, 2, 0);

	if (-1 != (fd = unixL_optfileno(L, 1, -1))) {
		if (0 != ftruncate(fd, len))
			return unixL_pusherror(L, errno, "truncate", "0$#");
	} else {
		path = luaL_checkstring(L, 1);

		if (0 != truncate(path, len))
			return unixL_pusherror(L, errno, "truncate", "0$#");
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_truncate() */


static int unix_tzset(lua_State *L) {
	tzset();

	lua_pushboolean(L, 1);

	return 1;
} /* unix_tzset() */


static int unix_umask(lua_State *L) {
	mode_t cmask = unixL_getumask(L);

	if (lua_isnoneornil(L, 1)) {
		lua_pushnumber(L, cmask);
	} else {
		lua_pushnumber(L, umask(unixL_optmode(L, 1, cmask, cmask)));
	}

	return 1;
} /* unix_umask() */


static int unix_uname(lua_State *L) {
	struct utsname name;

	if (-1 == uname(&name))
		return unixL_pusherror(L, errno, "uname", "~$#");

	if (lua_isnoneornil(L, 1)) {
		lua_createtable(L, 0, 5);

		lua_pushstring(L, name.sysname);
		lua_setfield(L, -2, "sysname");

		lua_pushstring(L, name.nodename);
		lua_setfield(L, -2, "nodename");

		lua_pushstring(L, name.release);
		lua_setfield(L, -2, "release");

		lua_pushstring(L, name.version);
		lua_setfield(L, -2, "version");

		lua_pushstring(L, name.machine);
		lua_setfield(L, -2, "machine");

		return 1;
	} else {
		static const char *opts[] = {
			"sysname", "nodename", "release", "version", "machine", NULL
		};
		int i, n = 0, top = lua_gettop(L);

		for (i = 1; i <= top; i++) {
			switch (luaL_checkoption(L, i, NULL, opts)) {
			case 0:
				lua_pushstring(L, name.sysname);
				++n;

				break;
			case 1:
				lua_pushstring(L, name.nodename);
				++n;

				break;
			case 2:
				lua_pushstring(L, name.release);
				++n;

				break;
			case 3:
				lua_pushstring(L, name.version);
				++n;

				break;
			case 4:
				lua_pushstring(L, name.machine);
				++n;

				break;
			}
		}

		return n;
	}
} /* unix_uname() */


static int unix_unlink(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != unlink(path))
		return unixL_pusherror(L, errno, "unlink", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_unlink() */


static int unix__gc(lua_State *L) {
	unixL_destroy(lua_touserdata(L, 1));

	return 0;
} /* unix__gc() */


static const luaL_Reg unix_routines[] = {
	{ "arc4random",         &unix_arc4random },
	{ "arc4random_buf",     &unix_arc4random_buf },
	{ "arc4random_stir",    &unix_arc4random_stir },
	{ "arc4random_uniform", &unix_arc4random_uniform },
	{ "chdir",              &unix_chdir },
	{ "chown",              &unix_chown },
	{ "chroot",             &unix_chroot },
	{ "clock_gettime",      &unix_clock_gettime },
	{ "closedir",           &unix_closedir },
	{ "getegid",            &unix_getegid },
	{ "geteuid",            &unix_geteuid },
	{ "getmode",            &unix_getmode },
	{ "getgid",             &unix_getgid },
	{ "getgrnam",           &unix_getgrnam },
	{ "getgruid",           &unix_getgrnam },
	{ "getifaddrs",         &unix_getifaddrs },
	{ "getpid",             &unix_getpid },
	{ "getpwnam",           &unix_getpwnam },
	{ "getpwuid",           &unix_getpwnam },
	{ "gettimeofday",       &unix_gettimeofday },
	{ "getuid",             &unix_getuid },
	{ "issetugid",          &unix_issetugid },
	{ "link",               &unix_link },
	{ "mkdir",              &unix_mkdir },
	{ "mkpath",             &unix_mkpath },
	{ "opendir",            &unix_opendir },
	{ "readdir",            &unix_readdir },
	{ "rename",             &unix_rename },
	{ "rewinddir",          &unix_rewinddir },
	{ "rmdir",              &unix_rmdir },
	{ "S_ISBLK",            &unix_S_ISBLK },
	{ "S_ISCHR",            &unix_S_ISCHR },
	{ "S_ISDIR",            &unix_S_ISDIR },
	{ "S_ISFIFO",           &unix_S_ISFIFO },
	{ "S_ISREG",            &unix_S_ISREG },
	{ "S_ISLNK",            &unix_S_ISLNK },
	{ "S_ISSOCK",           &unix_S_ISSOCK },
	{ "setegid",            &unix_setegid },
	{ "seteuid",            &unix_seteuid },
	{ "setgid",             &unix_setgid },
	{ "setuid",             &unix_setuid },
	{ "setsid",             &unix_setsid },
	{ "symlink",            &unix_symlink },
	{ "timegm",             &unix_timegm },
	{ "truncate",           &unix_truncate },
	{ "tzset",              &unix_tzset },
	{ "umask",              &unix_umask },
	{ "uname",              &unix_uname },
	{ "unlink",             &unix_unlink },
	{ NULL,                 NULL }
}; /* unix_routines[] */


int luaopen_unix(lua_State *L) {
	unixL_State *U;
	int error;
	const luaL_Reg *f;

	/*
	 * setup unixL_State context
	 */
	U = lua_newuserdata(L, sizeof *U);
	*U = unixL_initializer;

	lua_newtable(L);
	lua_pushcfunction(L, &unix__gc);
	lua_setfield(L, -2, "__gc");

	lua_setmetatable(L, -2);

	if ((error = unixL_init(U)))
		return luaL_error(L, "%s", unixL_strerror3(L, U, error));

	/*
	 * add struct ifaddrs* class
	 */
	lua_pushvalue(L, -1);
	unixL_newmetatable(L, "struct ifaddrs*", ifs_methods, ifs_metamethods, 1);
	lua_pop(L, 1);

	/*
	 * add DIR* class
	 */
	lua_pushvalue(L, -1);
	unixL_newmetatable(L, "DIR*", dir_methods, dir_metamethods, 1);
	lua_pop(L, 1);

	/*
	 * insert unix routines into module table with unixL_State as upvalue
	 */
	luaL_newlibtable(L, unix_routines);
	lua_pushvalue(L, -2);
	luaL_setfuncs(L, unix_routines, 1);

	return 1;
} /* luaopen_unix() */

