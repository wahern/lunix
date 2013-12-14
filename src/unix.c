#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1 /* Solaris */
#endif

#include <limits.h>     /* NL_TEXTMAX */
#include <stdarg.h>     /* va_list va_start va_arg va_end */
#include <stdint.h>     /* SIZE_MAX */
#include <stdlib.h>     /* arc4random(3) free(3) realloc(3) strtoul(3) */
#include <stdio.h>      /* fileno(3) snprintf(3) */
#include <string.h>     /* memset(3) strerror_r(3) strspn(3) strcspn(3) */
#include <signal.h>     /* sigset_t sigfillset(3) sigemptyset(3) sigprocmask(2) */
#include <ctype.h>      /* isspace(3) */
#include <time.h>       /* struct tm struct timespec gmtime_r(3) clock_gettime(3) */
#include <errno.h>      /* ENOMEM errno */

#include <sys/types.h>  /* gid_t mode_t off_t pid_t uid_t */
#include <sys/stat.h>   /* S_ISDIR() */
#include <sys/time.h>   /* struct timeval gettimeofday(2) */
#if __linux
#include <sys/sysctl.h> /* CTL_KERN KERN_RANDOM RANDOM_UUID sysctl(2) */
#endif
#include <sys/wait.h>   /* waitpid(2) */
#include <unistd.h>     /* chdir(2) chroot(2) close(2) chdir(2) chown(2) chroot(2) getpid(2) link(2) rename(2) rmdir(2) setegid(2) seteuid(2) setgid(2) setuid(2) setsid(2) symlink(2) truncate(2) umask(2) unlink(2) */
#include <fcntl.h>      /* F_GETFD F_SETFD FD_CLOEXEC fcntl(2) open(2) */
#include <pwd.h>        /* struct passwd getpwnam_r(3) */
#include <grp.h>        /* struct group getgrnam_r(3) */


#if __APPLE__
#include <mach/mach_time.h> /* mach_timebase_info() mach_absolute_time() */
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


/*
 * L U A  C O M P A T A B I L I T Y
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if LUA_VERSION_NUM < 502

#ifndef LUA_FILEHANDLE
#define LUA_FILEHANDLE "FILE*"
#endif


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

#endif


/*
 * F E A T U R E  D E T E C T I O N
 *
 * In lieu of external detection do our best to detect features using the
 * preprocessor environment.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(m, n) 0
#endif

#ifndef HAVE_ARC4RANDOM
#define HAVE_ARC4RANDOM (defined __OpenBSD__ || defined __FreeBSD__ || defined __NetBSD__ || defined __MirBSD__ || defined __APPLE__)
#endif

#ifndef HAVE_PIPE2
#define HAVE_PIPE2 (__GNUC_PREREQ(2, 9) || __FreeBSD__ >= 10)
#endif


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


#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#ifndef countof
#define countof(a) (sizeof (a) / sizeof *(a))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b))? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(i, m, n) (((i) < (m))? (m) : ((i) > (n))? (n) : (i))
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

#define u_flags_t long long


static u_error_t u_close(int *fd) {
	int error;

	if (*fd != -1)
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
	u_flags_t _flags;
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

	if (-1 == (*fd = open(path, flags, mode))) {
		if (errno != EINVAL || !(flags & U_CLOEXEC))
			goto syerr;

		if (-1 == (*fd = open(path, (flags & ~U_CLOEXEC), mode)))
			goto syerr;
	}

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
	union {
		unsigned char bytes[128];
		struct timeval tv;
		clock_t clk;
		pid_t pid;
	} rnd;
	unsigned n;

	rnd.pid = getpid();

	if (R->count > 0 && R->pid == rnd.pid && !force)
		return;

	gettimeofday(&rnd.tv, NULL);
	rnd.clk = clock();

#if __linux
	{	
		int mib[] = { CTL_KERN, KERN_RANDOM, RANDOM_UUID };
		unsigned char uuid[sizeof rnd.bytes];
		size_t count = 0, n;

		while (count < sizeof uuid) {
			n = sizeof uuid - count;

			if (0 != sysctl(mib, countof(mib), &uuid[count], &n, (void *)0, 0))
				break;

			count += n;
		}

		if (count > 0) {
			for (n = 0; n < sizeof rnd.bytes; n++) {
				rnd.bytes[n] ^= uuid[n];
			}

			if (count == sizeof uuid)
				goto stir;
		}
	}
#endif

	{
		unsigned char bytes[sizeof rnd.bytes];
		size_t count = 0;
		ssize_t n;
		int error;

		if (R->fd == -1 && (error = u_open(&R->fd, "/dev/urandom", O_RDONLY|U_CLOEXEC, 0)))
			goto stir;

		while (count < sizeof bytes) {
			n = read(R->fd, &bytes[count], sizeof bytes - count);

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

		for (n = 0; n < (ssize_t)sizeof rnd.bytes; n++) {
			rnd.bytes[n] ^= bytes[n];
		}
	}

stir:
	arc4_addrandom(R, rnd.bytes, sizeof rnd.bytes);

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
	} ts;

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


static int unixL_pusherror(lua_State *L, const char *fun NOTUSED, const char *fmt) {
	int error = errno, top = lua_gettop(L), fc;
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


static int unixL_getpwnam(lua_State *L, const char *user, struct passwd **ent) {
	unixL_State *U = unixL_getstate(L);
	int error;

	*ent = NULL;

	while (0 != getpwnam_r(user, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent)) {
		if (errno != ERANGE)
			return errno;

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

	while (0 != getpwuid_r(uid, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent)) {
		if (errno != ERANGE)
			return errno;

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

	while (0 != getgrnam_r(group, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent)) {
		if (errno != ERANGE)
			return errno;

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

	while (0 != getgrgid_r(gid, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent)) {
		if (errno != ERANGE)
			return errno;

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


static mode_t unixL_getumask(lua_State *L) {
	unixL_State *U = unixL_getstate(L);
	pid_t pid;
	mode_t mask;
	int status;
	ssize_t n;

	do {
		n = read(U->ts.fd[0], &mask, sizeof mask);
	} while (n > 0);

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
 *
 * NOTE: umask(2) is not thread-safe. The only thread-safe way I can think
 * of to query the file creation mask is to create a file with mode 0777 and
 * check which bits were masked. However, we can't rely on being able to
 * create a file at runtime. Therefore, the mode 0777 is used when the who
 * component is unspecified, rather than (0777 & umask()) as specified by
 * POSIX.
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
	FILE *fp;
	int fd;

	if (!(fp = luaL_testudata(L, 1, LUA_FILEHANDLE)))
		return def;

	luaL_argcheck(L, fp != NULL, index, "attempt to use a closed file");

	fd = fileno(fp);

	luaL_argcheck(L, fd >= 0, index, "attempt to use irregular file (no descriptor)");

	return fd;
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


static int unix_arc4random_uniform(lua_State *L) {
	if (lua_isnoneornil(L, 1)) {
		lua_pushnumber(L, unixL_random(L));
	} else {
		uint32_t n = (uint32_t)luaL_checknumber(L, 1);
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
			return unixL_pusherror(L, "chdir", "0$#");
	} else {
		const char *path = luaL_checkstring(L, 1);

		if (0 != chdir(path))
			return unixL_pusherror(L, "chdir", "0$#");
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
			return unixL_pusherror(L, "chown", "0$#");
	} else {
		const char *path = luaL_checkstring(L, 1);

		if (0 != chown(path, uid, gid))
			return unixL_pusherror(L, "chown", "0$#");
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_chown() */


static int unix_chroot(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != chroot(path))
		return unixL_pusherror(L, "chroot", "0$#");

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
			return unixL_pusherror(L, "clock_gettime", "~$#");

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
		return unixL_pusherror(L, "clock_gettime", "~$#");
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


static int unix_getmode(lua_State *L) {
	const char *fmt;
	char *end;
	mode_t omode;

	fmt = luaL_optstring(L, 2, "0777");
	omode = 07777 & strtoul(fmt, &end, 0);

	lua_pushnumber(L, unixL_optmode(L, 1, 0777, omode));

	return 1;
} /* unix_getmode() */


static int unix_getpid(lua_State *L) {
	lua_pushnumber(L, getpid());

	return 1;
} /* unix_getpid() */


static int unix_gettimeofday(lua_State *L) {
	struct timeval tv;

	if (0 != gettimeofday(&tv, NULL))
		return unixL_pusherror(L, "gettimeofday", "~$#");

	if (lua_isnoneornil(L, 1) || !lua_toboolean(L, 1)) {
		lua_pushnumber(L, (double)tv.tv_sec + ((double)tv.tv_usec / 1000000L));

		return 1;
	} else {
		lua_pushinteger(L, tv.tv_sec);
		lua_pushinteger(L, tv.tv_usec);

		return 2;
	}
} /* unix_gettimeofday() */


static int unix_link(lua_State *L) {
	const char *src = luaL_checkstring(L, 1);
	const char *dst = luaL_checkstring(L, 2);

	if (0 != link(src, dst))
		return unixL_pusherror(L, "link", "0$#");

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
		return unixL_pusherror(L, "mkdir", "0$#");

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
				return unixL_pusherror(L, "mkpath", "0$#");
		} else {
			int error = errno;
			struct stat st;

			if (0 != stat(dir, &st)) {
				errno = error;
				return unixL_pusherror(L, "mkpath", "0$#");
			}

			if (!S_ISDIR(st.st_mode)) {
				errno = ENOTDIR;
				return unixL_pusherror(L, "mkpath", "0$#");
			}
		}

		*slash = lc;
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_mkpath() */


static int unix_rename(lua_State *L) {
	const char *opath = luaL_checkstring(L, 1);
	const char *npath = luaL_checkstring(L, 2);

	if (0 != rename(opath, npath))
		return unixL_pusherror(L, "rename", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_rename() */


static int unix_rmdir(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != rmdir(path))
		return unixL_pusherror(L, "rmdir", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_rmdir() */


static int unix_setegid(lua_State *L) {
	gid_t gid = unixL_checkgid(L, 1);

	if (0 != setegid(gid))
		return unixL_pusherror(L, "setegid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setegid() */


static int unix_seteuid(lua_State *L) {
	uid_t uid = unixL_checkuid(L, 1);

	if (0 != seteuid(uid))
		return unixL_pusherror(L, "seteuid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_seteuid() */


static int unix_setgid(lua_State *L) {
	gid_t gid = unixL_checkgid(L, 1);

	if (0 != setgid(gid))
		return unixL_pusherror(L, "setgid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setgid() */


static int unix_setsid(lua_State *L) {
	pid_t pg;

	if (-1 == (pg = setsid()))
		return unixL_pusherror(L, "setsid", "~$#");

	lua_pushnumber(L, pg);

	return 1;
} /* unix_setsid() */


static int unix_setuid(lua_State *L) {
	uid_t uid = unixL_checkuid(L, 1);

	if (0 != setuid(uid))
		return unixL_pusherror(L, "setuid", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_setuid() */


static int unix_symlink(lua_State *L) {
	const char *src = luaL_checkstring(L, 1);
	const char *dst = luaL_checkstring(L, 2);

	if (0 != symlink(src, dst))
		return unixL_pusherror(L, "symlink", "0$#");

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
			return unixL_pusherror(L, "truncate", "0$#");
	} else {
		path = luaL_checkstring(L, 1);

		if (0 != truncate(path, len))
			return unixL_pusherror(L, "truncate", "0$#");
	}

	lua_pushboolean(L, 1);

	return 1;
} /* unix_truncate() */


static int unix_umask(lua_State *L) {
	mode_t cmask = unixL_getumask(L);

	if (lua_isnoneornil(L, 1)) {
		lua_pushnumber(L, cmask);
	} else {
		lua_pushnumber(L, umask(unixL_optmode(L, 1, cmask, cmask)));
	}

	return 1;
} /* unix_umask() */


static int unix_unlink(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != unlink(path))
		return unixL_pusherror(L, "unlink", "0$#");

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
	{ "arc4random_uniform", &unix_arc4random_uniform },
	{ "chdir",              &unix_chdir },
	{ "chown",              &unix_chown },
	{ "chroot",             &unix_chroot },
	{ "clock_gettime",      &unix_clock_gettime },
	{ "getmode",            &unix_getmode },
	{ "getpid",             &unix_getpid },
	{ "gettimeofday",       &unix_gettimeofday },
	{ "link",               &unix_link },
	{ "mkdir",              &unix_mkdir },
	{ "mkpath",             &unix_mkpath },
	{ "rename",             &unix_rename },
	{ "rmdir",              &unix_rmdir },
	{ "setegid",            &unix_setegid },
	{ "seteuid",            &unix_seteuid },
	{ "setgid",             &unix_setgid },
	{ "setuid",             &unix_setuid },
	{ "setsid",             &unix_setsid },
	{ "symlink",            &unix_symlink },
	{ "timegm",             &unix_timegm },
	{ "truncate",           &unix_truncate },
	{ "umask",              &unix_umask },
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
	 * insert routines into module table with unixL_State as upvalue
	 */
	lua_newtable(L);

	for (f = &unix_routines[0]; f->func; f++) {
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, f->func, 1);
		lua_setfield(L, -2, f->name);
	}

	return 1;
} /* luaopen_unix() */

