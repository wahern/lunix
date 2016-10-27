#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local function strfamily(af)
	for k,v in pairs(unix) do
		if v == af and type(k) == "string" and k:match"^AF_" then
			return k
		end
	end

	return string.format("AF_%d", assert(tonumber(af)))
end

local function strname(addr)
	local ip, port = assert(unix.getnameinfo(addr, unix.NI_NUMERICHOST + unix.NI_NUMERICSERV))
	return string.format("[%s]:%d", ip, tonumber(port))
end

local function setnonblock(fd)
	local flags = assert(unix.fcntl(fd, unix.F_GETFL))
	assert(unix.fcntl(fd, unix.F_SETFL, flags + unix.O_NONBLOCK))
end

local function setrecvaddr(fd, family)
	local type, level
	if family == unix.AF_INET6 then
		level = unix.IPPROTO_IPV6
		type = unix.IPV6_RECVPKTINFO or unix.IPV6_PKTINFO
	else
		level = unix.IPPROTO_IP
		type = unix.IP_RECVDSTADDR or unix.IP_PKTINFO
	end

	if type and level then
		return unix.setsockopt(fd, level, type, true)
	else
		local errno = unix.EAFNOSUPPORT
		return false, unix.strerror(errno), errno
	end
end

local function inaddr_any(family)
	return family == unix.AF_INET6 and "::" or "0.0.0.0"
end

local function do_recvfromto(family, port)
	local sd = assert(unix.socket(family, unix.SOCK_DGRAM))
	local ok, why = setrecvaddr(sd, family)
	if not ok then
		info("address family not supported (%s) (%s)", strfamily(family), tostring(why))
		return
	end
	assert(unix.bind(sd, { family = family, addr = inaddr_any(family), port = port }))
	setnonblock(sd)

	local fd = assert(unix.socket(family, unix.SOCK_DGRAM))
	-- NB: FreeBSD and macOS requires binding to INADDR_ANY. macOS 10.10
	-- and earlier will actually kernel panic otherwise.
	assert(unix.bind(fd, { family = family, addr = inaddr_any(family), port = 0 }))
	setnonblock(fd)

	for ifa in unix.getifaddrs() do
		if ifa.family == family then
			local to = { family = family, addr = ifa.addr, port = port }
			local from = { family = family, addr = ifa.addr, port = port + 1 }
			local ok, why, errno = unix.sendtofrom(fd, "hello world", 0, to, from)
			if ok then
				info("sendtofrom (to:%s from:%s) -> OK", strname(to), strname(from))
			else
				local log = errno == unix.EAFNOSUPPORT and info or panic
				log("sendtofrom (%s) (%s)", strfamily(family), why)
				-- continue if possible to test recvfromto
				if not unix.sendto(fd, "hello world", 0, to) then
					return
				end
				info("sendto (to:%s) -> OK", strname(to))
			end

			local nr = assert(unix.poll({ [sd] = { events = unix.POLLIN } }, 3))
			info("unix.poll -> (nr:%d)", nr)
			local msg, from, to = assert(unix.recvfromto(sd, 512, 0))
			info("recvfromto -> (msg:%s from:%s to:%s)", msg, strname(from), strname(to))
		end
	end

	unix.close(fd)
	unix.close(sd)
end

for _,family in ipairs{ unix.AF_INET, unix.AF_INET6 } do
	do_recvfromto(family, 8000)
end

say"OK"
