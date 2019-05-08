#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local rfd, wfd = check(unix.socketpair(unix.AF_UNIX, unix.SOCK_DGRAM))
local msg = "hello world"
check(unix.write(wfd, msg))
local rmsg = check(unix.read(rfd, #msg))
check(msg == rmsg, string.format("expected '%s', got '%s'", msg, rmsg or "?"))

say"OK"
