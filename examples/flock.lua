-- POSIX fcntl file locks, not BSD flock

local unix = require"unix"

local fh = assert(io.tmpfile())

assert(unix.fcntl(fh, unix.F_SETLK))
io.stdout:write(string.format("%d has file lock\n", unix.getpid()))

io.stdout:write(string.format("forking\n"))

if 0 == unix.fork() then
	local l = assert(unix.fcntl(fh, unix.F_GETLK))
	assert(unix.getppid() == l.pid)
	io.stdout:write(string.format("F_GETLK from %d confirms %d has file lock\n", unix.getpid(), l.pid))
else
	assert(unix.waitpid())
end

