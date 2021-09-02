#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

require"regress".export".*"
local unix = require"unix"

local function addrinfo(func, fd, rfd, saddr)
	local family = tostring(saddr and saddr.family or "?")
	local path = saddr.path or "?"
	if rfd then
		info("%s(%d) -> fd:%d family:%s path:%s", func, fd, rfd, family, path)
	else
		info("%s(%d) -> family:%s path:%s", func, fd, family, path)
	end
end

local function setnonblock(fd)
	local flags = assert(unix.fcntl(fd, unix.F_GETFL))
	assert(unix.fcntl(fd, unix.F_SETFL, flags + unix.O_NONBLOCK))
	return fd
end

local tmpdir = check(mkdtemp())
local path = string.format("%s/%s", tmpdir, "socket")

local sd = setnonblock(assert(unix.socket(unix.AF_UNIX, unix.SOCK_STREAM)))
assert(unix.bind(sd, { family = unix.AF_UNIX, path =  path }))
assert(unix.listen(sd, unix.SOMAXCONN))
local saddr = assert(unix.getsockname(sd))
check(saddr.family == unix.AF_UNIX, "wrong address family")
addrinfo("getsockname", sd, nil, saddr)

local fd = setnonblock(assert(unix.socket(unix.AF_UNIX, unix.SOCK_STREAM)))
assert(unix.connect(fd, saddr))

assert(unix.poll({ [sd] = { events = unix.POLLIN } }, 3))
local fd1, saddr = assert(unix.accept(sd))
addrinfo("accept", sd, fd1, saddr)

local saddr = assert(unix.getsockname(fd1))
addrinfo("getsockname", fd1, nil, saddr)

local saddr = assert(unix.getpeername(fd))
addrinfo("getpeername", fd, nil, saddr)

local saddr = assert(unix.getsockname(fd))
addrinfo("getsockname", fd, nil, saddr)

say"OK"
