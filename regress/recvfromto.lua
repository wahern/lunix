#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local function strname(addr)
	local ip, port = assert(unix.getnameinfo(addr, unix.NI_NUMERICHOST + unix.NI_NUMERICSERV))
	return string.format("[%s]:%d", ip, tonumber(port))
end

local function setnonblock(fd)
	local flags = assert(unix.fcntl(fd, unix.F_GETFL))
	assert(unix.fcntl(fd, unix.F_SETFL, flags + unix.O_NONBLOCK))
end

--local family = unix.AF_INET
--local addr = "0.0.0.0"
local family = unix.AF_INET6
local addr = "::"
local port = 8000

local sd = assert(unix.socket(family, unix.SOCK_DGRAM))
assert(unix.bind(sd, { family = family, addr = addr, port = port }))
setnonblock(sd)

local fd = assert(unix.socket(family, unix.SOCK_DGRAM))
setnonblock(fd)

for ifa in unix.getifaddrs() do
	if ifa.family == family then
		local to = { family = family, addr = ifa.addr, port = port }
		local from = { family = family, addr = ifa.addr, port = port + 1 }
		assert(unix.sendtofrom(fd, "hello world", 0, to, from))
		info("sendtofrom (to:%s from:%s)", strname(to), strname(from))

		local msg, from, to = assert(unix.recvfromto(sd, 512, 0))
		info("recvfromto -> (msg:%s from:%s to:%s)", msg, strname(from), strname(to))
	end
end

say"OK"
