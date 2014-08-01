unix = require"unix"

io.stdout:write"blocking all signals\n"
unix.sigprocmask(unix.SIG_BLOCK, "*")

io.stdout:write"waiting on any signal... "
io.stdout:flush()
local _, _, errno = unix.sigtimedwait("*", 0.5)
assert(errno == unix.EAGAIN)
io.stdout:write"got EAGAIN as expected\n"

io.stdout:write"raising SIGTERM\n"
unix.raise(unix.SIGTERM)

io.stdout:write"waiting on any signal... "
io.stdout:flush()
assert(unix.SIGTERM == assert(unix.sigtimedwait("*", 0)), "got wrong signal")
io.stdout:write"got SIGTERM as expected\n"

local pid = unix.getpid()
if 0 == assert(unix.fork()) then
	io.stdout:write(string.format("sending %d SIGHUP in 0.5s from %d\n", pid, unix.getpid()))
	unix.sigtimedwait("*", 0.5) -- sleep 0.5s
	unix.kill(pid, unix.SIGHUP)
	unix._exit()
end

io.stdout:write"waiting on any signal... "
io.stdout:flush()
assert(unix.SIGHUP == assert(unix.sigtimedwait("*", 2.0)), "got wrong signal")
io.stdout:write"got SIGHUP as expected\n"

io.stdout:write"SUCCESS\n"
