#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1 /* Solaris */
#endif

#include <limits.h>    /* NL_TEXTMAX */
#include <stdarg.h>    /* va_list va_start va_arg va_end */
#include <stdint.h>    /* SIZE_MAX */
#include <stdlib.h>    /* arc4random(3) free(3) realloc(3) */
#include <stdio.h>     /* snprintf(3) */
#include <string.h>    /* memset(3) strerror_r(3) */

#include <sys/types.h> /* gid_t pid_t uid_t */
#include <unistd.h>    /* chdir(2) chroot(2) close(2) getpid(3) setegid(2) seteuid(2) setgid(2) setuid(2) */
#include <fcntl.h>     /* F_GETFD F_SETFD FD_CLOEXEC fcntl(2) open(2) */
#include <pwd.h>       /* struct passwd getpwnam_r(3) */
#include <grp.h>       /* struct group getgrnam_r(3) */


#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#ifndef countof
#define countof(a) (sizeof (a) / sizeof *(a))
#endif



static size_t power2(size_t i) {
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
} /* power2() */


static int growbuf(char **buf, size_t *size, size_t minsiz) {
	void *tmp;
	size_t tmpsiz;

	if (*size == (size_t)-1)
		return ENOMEM;

	if (*size > ~((size_t)-1 >> 1)) {
		tmpsiz = (size_t)-1;
	} else {
		tmpsiz = power2(*size + 1);
		tmpsiz = MIN(tmpsiz, minsiz);
	}

	if (!(tmp = realloc(*buf, tmpsiz)))
		return errno;

	*buf = tmp;
	*size = tmpsiz;

	return 0;
} /* growbuf() */


static void closefd(int *fd) {
	if (*fd != -1) {
		int error = errno;

		(void)close(*fd);
		*fd = -1;

		errno = error;
	}
} /* closefd() */


static int open_cloexec(const char *path, int flags, ...) {
	mode_t mode = 0;
	int fd;

	if (flags & O_CREAT) {
		va_list ap;

		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

#if defined O_CLOEXEC
	flags |= O_CLOEXEC;
#endif

	fd = open(path, flags, mode);

#if !defined O_CLOEXEC
	if (fd != -1) {
		int fflags;

		if (-1 == (fflags = fcntl(fd, F_GETFD))) {
			closefd(&fd);

			return -1;
		}

		fflags |= FD_CLOEXEC;

		if (0 != fcntl(fd, F_SETFD, fflags)) {
			closefd(&fd);

			return -1;
		}
	}
#endif

	return fd;
} /* open_cloexec() */


#ifndef HAVE_ARC4RANDOM
#define HAVE_ARC4RANDOM (defined __OpenBSD__ || defined __FreeBSD__ || defined __NetBSD__ || defined __MirBSD__ || defined __APPLE__)
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
	closefd(&R->fd);
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
	++R->j;
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

		if (R->fd == -1) {
			if (-1 == (R->fd = open_cloexec("/dev/urandom", O_RDONLY)))
				goto stir;
		}

		while (count < sizeof bytes) {
			n = read(R->fd, &bytes[count], sizeof bytes - count);

			if (n == -1) {
				if (errno == EINTR)
					continue;
				break;
			} else if (n == 0) {
				closefd(&R->fd);

				break;
			}

			count += n;
		}

		for (n = 0; n < (ssize_t)sizeof(rnd.bytes); n++) {
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


#define UNIXL_STATE_INITIALIZER { .random = UNIXL_RANDOM_INITIALIZER, }

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

#if !HAVE_ARC4RANDOM
	unixL_Random random;
#endif
} unixL_State;


static int unixL_init(unixL_State *U) {
#if !HAVE_ARC4RANDOM
	arc4_init(&U->random);
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
} /* unixL_destroy() */


static const char *unixL_strerror3(lua_State *L, unixL_State *U, int error) {
	if (0 != strerror_r(error, U->errmsg, sizeof U->errmsg) || U->errmsg[0] == '\0') {
		if (0 > snprintf(U->errmsg, sizeof U->errmsg, "%s: %d", ((error)? "Unknown error" : "Undefined error"), error))
			luaL_error(L, "snprintf failure");
	}

	return L->errmsg;
} /* unixL_strerror3() */


static const char *unixL_strerror(lua_State *L, int error) {
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));

	return unixL_strerror3(L, U, error);
} /* unixL_strerror() */


static int unixL_pusherror(lua_State *L, const char *fun, const char *fmt) {
	int error = errno, top = lua_gettop(L), fc;
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));

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
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
	int error;

	*ent = NULL;

	while (0 != getpwnam_r(user, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent))) {
		if (errno != ERANGE)
			return errno;

		if ((error = growbuf(&U->pw.buf, &U->pw.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getpwnam() */


static int unixL_getpwuid(lua_State *L, uid_t uid, struct passwd **ent) {
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
	int error;

	*ent = NULL;

	while (0 != getpwuid_r(uid, &U->pw.ent, U->pw.buf, U->pw.bufsiz, ent))) {
		if (errno != ERANGE)
			return errno;

		if ((error = growbuf(&U->pw.buf, &U->pw.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getpwuid() */


static int unixL_getgrnam(lua_State *L, const char *group, struct group **ent) {
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
	int error;

	*ent = NULL;

	while (0 != getgrnam_r(group, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent))) {
		if (errno != ERANGE)
			return errno;

		if ((error = growbuf(&U->gr.buf, &U->gr.bufsiz, 128)))
			return error;

		*ent = NULL;
	}

	return 0;
} /* unixL_getgrnam() */


static int unixL_getgruid(lua_State *L, gid_t gid, struct group **ent) {
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
	int error;

	*ent = NULL;

	while (0 != getgruid_r(gid, &U->gr.ent, U->gr.buf, U->gr.bufsiz, ent))) {
		if (errno != ERANGE)
			return errno;

		if ((error = growbuf(&U->gr.buf, &U->gr.bufsiz, 128)))
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
		return luaL_error(L, "%s: %s", user, unixL_strerror(L, error)), -1;

	if (!ent)
		return luaL_error(L, "%s: no such user", user), -1;

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
		return luaL_error(L, "%s: %s", group, unixL_strerror(L, error)), -1;

	if (!ent)
		return luaL_error(L, "%s: no such group", group), -1;

	return gr->gr_gid;
} /* unixL_optgid() */


static uid_t unixL_checkgid(lua_State *L, int index) {
	luaL_checkany(L, index);

	return unixL_optgid(L, index, -1);
} /* unixL_checkgid() */


#if HAVE_ARC4RANDOM
#define ARC4RANDOM() arc4random()
#else
#define ARC4RANDOM() arc4_getword(&U->random)
#endif

static int unix_arc4random(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
#endif

	lua_pushnumber(L, ARC4RANDOM());

	return 1;
} /* unix_arc4random() */


static int unix_arc4random_buf(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
#endif
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
			tmp.r[i] = ARC4RANDOM();
		}

		luaL_addlstring(&B, tmp.c, m);
		n += m;
	}

	luaL_pushresult(&B);

	return 1;
} /* unix_arc4random_buf() */


static int unix_arc4random_uniform(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *U = lua_touserdata(L, lua_upvalueindex(L, 1));
#endif

	if (lua_isnoneornil(L, 1)) {
		lua_pushnumber(L, ARC4RANDOM());
	} else {
		uint32_t n = (uint32_t)luaL_checknumber(L, 1);
		uint32_t r, min;

		min = -n % n;

		for (;;) {
			r = ARC4RANDOM();

			if (r >= min)
				break;
		}

		lua_pushnumber(L, r % n);
	}

	return 1;
} /* unix_arc4random_uniform() */


static int unix_chdir(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);

	if (0 != chdir(path))
		return unixL_pusherror(L, "chdir", "0$#");

	lua_pushboolean(L, 1);

	return 1;
} /* unix_chdir() */


static int unix_chown(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	uid_t uid = unixL_optuid(L, 2, -1);
	gid_t gid = unixL_optgid(L, 3, -1);

	if (0 != chown(path, uid, gid))
		return unixL_pusherror(L, "chown", "0$#");

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


static int unix_getpid(lua_State *L) {
	lua_pushnumber(L, getpid());

	return 1;
} /* unix_getpid() */


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
	{ "getpid",             &unix_getpid },
	{ "setegid",            &unix_setegid },
	{ "seteuid",            &unix_seteuid },
	{ "setgid",             &unix_setgid },
	{ "setuid",             &unix_setuid },
	{ "setsid",             &unix_setsid },
	{ NULL,                 NULL }
}; /* unix_routines[] */


int luaopen_unix(lua_State *L) {
	static const U_init = UNIXL_STATE_INITIALIZER;
	unixL_State *U;
	int error;
	luaL_Reg *f;

	/*
	 * setup unixL_State context
	 */
	U = lua_newuserdata(L, sizeof *U);
	*U = U_init;

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
		lua_setfield(L, -2, "unix_routines", f->name);
	}

	return 1;
} /* luaopen_unix() */
