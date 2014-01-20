#include <stdarg.h> /* va_list va_start va_arg va_end */

#include <string.h> /* memset(3) */

#include <fcntl.h>  /* F_GETFD F_SETFD FD_CLOEXEC fcntl(2) open(2) */
#include <unistd.h> /* close(2) */


#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#ifndef countof
#define countof(a) (sizeof (a) / sizeof *(a))
#endif


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





#if !defined HAVE_ARC4RANDOM
#if defined __OpenBSD__ || defined __FreeBSD__ || defined __NetBSD__ || defined __MirBSD__ || defined __APPLE__
#define HAVE_ARC4RANDOM 1
#endif
#endif

#if !HAVE_ARC4RANDOM
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


typedef struct unixL_State {
	int lerrno; /* errno value from last failed syscall */

#if !HAVE_ARC4RANDOM
	unixL_Random random;
#endif
} unixL_State;


static int unixL_init(unixL_State *state) {
#if !HAVE_ARC4RANDOM
	arc4_init(&state->random);
#endif
} /* unixL_init() */


static void unixL_destroy(unixL_State *state) {
#if !HAVE_ARC4RANDOM
	arc4_destroy(&state->random);
#endif
} /* unixL_destroy() */


#if HAVE_ARC4RANDOM
#define ARC4RANDOM() arc4random()
#else
#define ARC4RANDOM() arc4_getword(&state->random)
#endif

static int unixL_arc4random(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *state = lua_touserdata(L, lua_upvalueindex(L, 1));
#endif

	lua_pushnumber(L, ARC4RANDOM());

	return 1;
} /* unixL_arc4random() */


static int unixL_arc4random_buf(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *state = lua_touserdata(L, lua_upvalueindex(L, 1));
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
} /* unixL_arc4random_buf() */


static int unixL_arc4random_uniform(lua_State *L) {
#if !HAVE_ARC4RANDOM
	unixL_State *state = lua_touserdata(L, lua_upvalueindex(L, 1));
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
} /* unixL_arc4random_uniform() */


static int unixL__gc(lua_State *L) {
	unixL_destroy(lua_touserdata(L, 1));

	return 0;
} /* unixL__gc() */


static const luaL_Reg unixL_routines[] = {
	{ "arc4random",         &unixL_arc4random },
	{ "arc4random_buf",     &unixL_arc4random_buf },
	{ "arc4random_uniform", &unixL_arc4random_uniform },
	{ NULL, NULL }
}; /* unixL_routines[] */


int luaopen_unix(lua_State *L) {
	lua_newtable(L);

	return 1;
} /* luaopen_unix() */
