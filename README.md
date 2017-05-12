# About

`lunix` is a Lua bindings library module to common Unix system APIs. The
module is regularly tested on recent versions of AIX, FreeBSD, Linux/glibc,
Linux/musl, NetBSD, OpenBSD, OS X, and Solaris. The best way to describe it
is in contradistinction to luaposix, the most popular bindings module for
Unix APIs in Lua.

## Thread-safety

Unlike luaposix, it strives to be as thread-safe as possible on the host
platform. Interfaces like `strerror_r` and `O_CLOEXEC` are used throughout
where appropriate. The module even includes a novel solution for the
inherently non-thread-safe `umask` system call, where calling `umask` from
one thread might result in another thread creating a file with unsafe or
unexpected permissions.

## POSIX Extensions

Unlike `luaposix`, the library does not restrict itself to POSIX, and where
possible emulates an interface when not available natively on a supported
platform. For example, the library provides `arc4random` (absent on Linux
and Solaris), `clock_gettime` (absent on OS X), and a thread-safe `timegm`
(absent on Solaris).

## Leak-safety

Unlike luaposix, the library prefers dealing with `FILE` handles rather than
raw integer descriptors. This helps to mitigate and prevent leaks or
double-close bugs---a common source of problems in, e.g., asynchronous
applications where a descriptor is closed that has already been reassigned
to another resource. Routines like `chdir`, `stat`, and `opendir`
transparently accept string paths, `FILE` handles, `DIR` handles, and raw
integer descriptors.

# Lua Versions

`lunix` supports the Lua 5.1, 5.2, and 5.3 APIs. `lunix` supports LuaJIT wth
the caveat that when creating custom `FILE` handles using `fdopen` (directly
or indirectly through, e.g., `fdup`) the process must have read access to an
existing filesystem resource (presently `.` or `/dev/null`) in order to
create a `LUA_FILEHANDLE` object in the VM. This might not work in some
sandboxed environments.

# Installation

The usual GNU-style flags are supported, such as `CC`, `CPPFLAGS`, `CFLAGS`,
`LDFLAGS`, `SOFLAGS`, and `LIBS`. ...

TODO. See [`lunix` Userguide PDF](http://25thandclement.com/~william/projects/lunix.pdf)

# API

## Conventions

### Namespace

Unlike recent versions of `luaposix`, interfaces are not grouped into
submodules. Both Unix and POSIX evolved organically over time and neither
consistently group interfaces, either by header or any other singular
convention. And while theoretically it could done _de_ _novo_ I feel that
might add confusion and is otherwise wasted effort. In C the interfaces
exist in a single namespace. While from a C source language perspective a
small handful of interfaces technically have naming conflicts depending on
which standard is selected at compile-time, in 2016 this is largely a
theoretical problem. Where it does exist (e.g. GNU `strerror_r` vs POSIX
`strerror_r`) this can easily be smoothed over transparently by `lunix`.

### `f` prefix.

Following the example of `fopen`, `lunix` implements a sister interface for
some routines to atomically wrap a descriptor within a `FILE` handle. This
cannot be easily accomplished from Lua script because of the possibility of
memory allocation failure. (Despite a widespread myth, this can easily
happen even on systems like Linux in the presence of process resource
limits, regardless of allocation size.) For example, the `lunix` routine
`fdup` behaves just like `dup` but returns a `FILE` handle instead of an
integer descriptor. Note that both `dup` and `fdup` will accept either an
integer descriptor or `FILE` handle.

### Errors

Generally, errors with argument type or format are thrown using `lua_error`.
System errors are returned directly using the idiomatic 3-tuple Lua
convention---nil or boolean false, error string description, error integer
code.

Some internal allocation errors are returned as-if they were system errors,
particularly where they arise in the C portion of an emulated routine. In
such a case the call may return with `ENOMEM` even through neither the local
system nor the POSIX specification specify such an error. Other allocation
errors are thrown, particularly where they occur within the Lua/C preamble
of a binding.

### Arithmetic Overflow

The module takes great care to detect arithmetic overflow, including
undefined arithmetic conversions between `lua_Integer`, `lua_Number`, and
the relevant system types. When detected arithemtic overflow is generally
treated as a type error and thrown (see Errors, above).

## Constants

TODO. See [`lunix` Userguide PDF](http://25thandclement.com/~william/projects/lunix.pdf).

## Routines

All of the following routines are implemented though they may not yet be
documented. Descriptions for some interfaces may be in the
[original PDF userguide](http://25thandclement.com/~william/projects/lunix.pdf).

### alarm

### arc4random

```
arc4random : () -> (integer)
```

Returns a cryptographically strong, uniformly random 32-bit integer as a Lua
number. On Linux the `RANDOM_UUID` `sysctl` feature is used to seed the
generator if available; or on more recent Linux and Solaris kernels the
`getrandom` interface.[^sysctl_uuid] This avoids fiddling with file
descriptors, and also works in a chroot jail. On other platforms without a
native `arc4random` interface, such as Solaris 11.2 or earlier, the
implementation must resort to /dev/urandom for seeding.

Note that unlike the original implementation on OpenBSD, arc4random on some
older platforms (e.g. FreeBSD prior to 10.10) seeds itself from
/dev/urandom. This could cause problems in chroot jails.

[^sysctl_uuid]: Some Linux distributions, such as Red Hat, disable
`sysctl(2)`.

### arc4random_buf

```
arc4random_buf : (n:integer) -> (string)
```

Returns a string of length $n$ containing cryptographically strong random
octets using the same CSPRNG underlying `arc4random`.

### arc4random_stir

```
arc4random_stir : () -> (true)
```

Stir the arc4random entropy pool using the best available resources. This
normally should be unnecessary and is a noop on some systems.

### arc4random_uniform

```
arc4random_uniform : (n:integer?) -> (integer)
```

Returns a cryptographically strong uniform random integer in the interval
$[0, n-1]$ where $n \leq 2^{32}$. If $n$ is omitted the interval is
$[0, 2^{32}-1]$ and effectively behaves like `arc4random`.

### bind

### bitand

### bitor

### chdir

```
chdir : (path:string|file|fd:integer) -> (true) | (false, string, integer)
```

If $dir$ is a string, attempts to change the current working directory using
`chdir(2)`. Otherwise, if $dir$ is a FILE handle referencing a
directory, or an integer file descriptor referencing a directory, attempts
to change the current working directory using `fchdir(2)`.

Returns `true` on success, otherwise returns `false`, an error string, and
an integer system error.

### chmod

### chown

```
chown : (path:string|file|fd:integer, uid?, gid?) -> (true) | (false, string, integer)
```

$file$ may either be a string path for use with `chown(2)`, or a FILE handle
or integer file descriptor for use with `fchown(2)`. $uid$ and $gid$ may be
integer values or symbolic string names.

Returns `true` on success, otherwise returns `false`, an error string, and
an integer system error.

### chroot

```
chroot : (path:string) -> (true) | (false, string, integer)
```

Attempt to chroot to the specified string $path$.

Returns `true` on success, otherwise returns `false`, an error string, and
an integer system error.

### clock_gettime

```
clock_gettime : (id:string|id:number) -> (integer) | (nil, string, integer)
```

$id$ should be the string "realtime" or "monotonic", or the integer
constant `CLOCK_REALTIME` or `CLOCK_MONOTONIC`.

Returns a time value as a Lua floating point number, otherwise returns `nil`,
an error string, and an integer system error.

### close

### closedir

```
closedir : (DIR) -> (true) | (false, string, integer)
```

Closes the DIR handle, releasing the underlying file descriptor.

### compl

### connect

### dup

### dup2

### dup3

### execve

```
execve : (path:string, {string*}, {string*}) -> (false, string, integer)
```

Executes $path$, replacing the existing process image. $path$ should be an
absolute pathname as the `$PATH` environment variable is not used. $argv$ is
a table or ipairs--iterable object specifying the argument vector to pass to
the new process image. Traditionally the first such argument should be the
basename of $path$, but this is not enforced. If absent or empty the new
process image will be passed an empty argument vector. $env$ is a table or
ipairs--iterable object specifying the new environment. If absent or empty
the new process image will contain an empty environment.

On success never returns. On failure returns `false`, an error string, and
an integer system error.

### execl

```
execl : (path:string, string*) -> (false, string, integer)
```

Executes $path$, replacing the existing process image. The `$PATH`
environment variable is not used. Any subsequent arguments are passed to the
new process image. The new process image inherits the current environment
table.

On success never returns. On failure returns `false`, an error string, and
an integer system error.

### execlp

```
execlp : (file:string, string*) -> (false, string, integer)
```

Executes $file$, replacing the existing process image. The `$PATH`
environment variable is used to search for $file$. Any subsequent arguments
are passed to the new process image. The new process image inherits the
current environment table.

On success never returns. On failure returns `false`, an error string, and
an integer system error.

### execvp

```
execvp : (file:string, string*) -> (false, string, integer)
```

Executes $file$, replacing the existing process image. The `$PATH`
environment variable is used to search for $file$. Any subsequent arguments
are passed to the new process image. The new process image inherits the
current environment table.

On success never returns. On failure returns `false`, an error string, and an
integer system error.

### _exit

```
_exit : (status:boolean?|status:integer?) -> ()
```

Exits the process immediately without first flushing and closing open
streams, and without calling `atexit(3)` handlers. If $status$ is boolean
`true` or `false`, exits with `EXIT_SUCCESS` or `EXIT_FAILURE`,
respectively. Otherwise, $status$ is an optional integer status value which
defaults to 0 (`EXIT_SUCCESS`).

### exit

```
exit : (status:boolean?|status:integer?) -> ()
```

Like `_exit`, but first flushes and closes open streams and calls
`atexit(3)` handlers.

### fchmod
### fchown
### fcntl
### fdatasync
### fdopen
### fdopendir
### fdup
### fileno

```
fileno -> (FILE|DIR|integer) -> (integer)
```

Resolves the specified FILE handle or DIR handle to an integer file
descriptor. An integer descriptor is returned as-is.

### flockfile

```
flockfile : (fh:FILE) -> (true)
```

Locks the FILE handle $fh$, blocking the current thread if already locked.
Returns `true`.

This function only works on FILE handles and not DIR handles or integer
descriptors.

### fstat
### fsync
### ftrylockfile

```
ftrylockfile : (fd:FILE) -> (true|false)
```

Attempts to lock the FILE handle $fh$. Returns `true` on success or `false`
if $fh$ was locked by another thread.

### funlockfile

```
funlockfile : (fd:FILE) -> (true)
```

Unlocks the FILE handle $fh$. Returns `true`.

### fopen
### fopenat
### fpipe
### fork

```
fork : () -> (integer) | (nil, string, integer)
```

Forks a new process. On success returns the PID of the new process in the
parent and the integer 0 in the child. Otherwise returns `false`, an error
string, and an integer system error.

### gai_strerror
### getaddrinfo
### getegid
### geteuid
### getenv
### getmode
### getgid
### getgrnam
### getgrgid
### getgroups
### gethostname
### getifaddrs
### getopt
### getpeername
### getpgid
### getpgrp
### getpid
### getppid
### getprogname
### getpwnam
### getpwuid
### getrlimit
### getrusage
### gettimeofday
### getuid
### grantpt
### ioctl
### isatty
### issetugid
### kill
### lchown
### link
### listen
### lockf
### lseek
### lstat
### mkdir
### mkdirat
### mkfifo
### mkfifoat
### mkpath
### open
### openat
### opendir
### pipe
### poll
### posix_fadvise
### posix_fallocate
### posix_openpt
### posix_fopenpt
### pread
### ptsname
### pwrite
### raise
### read
### readdir
### recv
### recvfrom
### recvfromto
### rename
### renameat
### rewinddir
### rmdir
### S_ISBLK
### S_ISCHR
### S_ISDIR
### S_ISFIFO
### S_ISREG
### S_ISLNK
### S_ISSOCK
### send
### sendto
### sendtofrom
### setegid
### setenv
### seteuid
### setgid
### setgroups
### setlocale
### setpgid
### setrlimit
### setsockopt
### setsid
### setuid
### sigaction
### sigfillset
### sigemptyset
### sigaddset
### sigdelset
### sigismember
### sigprocmask
### sigtimedwait
### sigwait
### sleep
### socket
### stat
### strerror
### strsignal
### symlink
### tcgetpgrp
### tcsetpgrp
### timegm
### truncate
### tzset
### umask
### uname
### unlink
### unlinkat
### unlockpt
### unsetenv
### wait
### waitpid
### write
### xor

<!-- Markdeep: --><style class="fallback">body{visibility:hidden;white-space:pre;font-family:monospace}</style><script src="markdeep.min.js"></script><script src="https://casual-effects.com/markdeep/latest/markdeep.min.js"></script><script>window.alreadyProcessedMarkdeep||(document.body.style.visibility="visible")</script>
