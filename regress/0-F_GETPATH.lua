#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local unsafe = require"unix.unsafe"
local regress = require"regress".export".*"

if unix.F_GETPATH then
	local fh = check(io.open("/etc/passwd", "r"))
	local path = check(unix.fcntl(fh, unix.F_GETPATH))
	info("opened '/etc/passwd', got '%s'", path)

	local MAXPATHLEN = check(unix.MAXPATHLEN, "MAXPATHLEN not defined")
	local n, obuf = check(unsafe.fcntl(fh, unix.F_GETPATH, string.rep(".", MAXPATHLEN)))
	local path2 = obuf:match"^[\001-\255]*"
	check(path == path2, string.format("expected '%s', got '%s'", path, path2))
	info("unsafe.fcntl gives same result '%s'", path2)
elseif unix.uname"sysname" == "Darwin" then
	panic("expected F_GETPATH definition")
else
	info("F_GETPATH not available")
end

say"OK"
