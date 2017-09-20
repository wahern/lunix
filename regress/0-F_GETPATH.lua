#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

if unix.F_GETPATH then
	local fh = check(io.open("/etc/passwd", "r"))
	local path = check(unix.fcntl(fh, unix.F_GETPATH))
	info("opened '/etc/passwd', got '%s'", path)
elseif unix.uname"sysname" == "Darwin" then
	panic("expected F_GETPATH definition")
else
	info("F_GETPATH not available")
end

say"OK"
