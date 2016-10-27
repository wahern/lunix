#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -j- "$0" "$@"
]]

require"regress".export".*"
local unix = require"unix"

local dt = os.date()
local fh = check(io.tmpfile())

check(fh:write(dt, "\n"))
check(fh:flush())

-- fopen of existing file descriptors not supported universally
local fh2 = check(unix.fdup(fh, unix.O_CLOEXEC))
local ln = check(fh2:seek"set") and check(fh2:read())

check(dt == ln, "could not read back line (%s ~= %s)", dt, tostring(ln))
check(unix.fileno(fh) ~= unix.fileno(fh2), "descriptor numbers not different")
check(unix.O_CLOEXEC == unix.bitand(unix.fcntl(fh2, unix.F_GETFL), unix.O_CLOEXEC), "O_CLOEXEC flag not set")


say"OK"
