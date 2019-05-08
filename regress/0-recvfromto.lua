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

local function monotime()
	return assert(unix.clock_gettime(unix.CLOCK_MONOTONIC))
end

local function do_recvfromto(family, port)
	local nokay, nifaddrs = 0, 0
	local sd = assert(unix.socket(family, unix.SOCK_DGRAM))
	local ok, why = setrecvaddr(sd, family)
	if not ok then
		info("address family not supported (%s) (%s)", strfamily(family), tostring(why))
		return nokay, nifaddrs
	end
	assert(unix.bind(sd, { family = family, addr = inaddr_any(family), port = port }))
	setnonblock(sd)

	local fd = assert(unix.socket(family, unix.SOCK_DGRAM))
	-- NB: FreeBSD and macOS requires binding to INADDR_ANY. macOS 10.10
	-- and earlier will actually kernel panic otherwise.
	assert(unix.bind(fd, { family = family, addr = inaddr_any(family), port = 0 }))
	setnonblock(fd)

	local function testit(to, from)
		local ok, why, errno = unix.sendtofrom(fd, "hello world", 0, to, from)
		if ok then
			info("sendtofrom (to:%s from:%s) -> OK", strname(to), strname(from))
		else
			info("sendtofrom (to:%s from:%s) (%s)", strname(to), strname(from), why)
			-- continue if possible to test recvfromto
			if not unix.sendto(fd, "hello world", 0, to) then
				return
			end
			info("sendto (to:%s) -> OK", strname(to))
		end

		local timeout = 3
		local deadline = monotime() + timeout
		local lasterr = nil
		while monotime() < deadline do
			local nr = assert(unix.poll({ [sd] = { events = unix.POLLIN } }, 1))
			info("unix.poll -> (nr:%d)", nr)
			if nr > 0 then
				local err, msg, from1, to1 = testerror(unix.recvfromto(sd, 512, 0))
				if not err and msg == "hello world" then
					info("recvfromto (from:%s to:%s) -> OK", strname(from1), strname(to1))
					return true --> SUCCESS
				elseif not err then
					info("recvfromto (from:%s to:%s) -> FAIL (expected 'hello world', got '%s')", strname(from1), strname(to1), msg or "?")
					return false
				else
					lasterr = err
					info("recvfromto (from:%s to:%s) -> FAIL (%s)", strname(from), strname(to), tostring(err))
					if err.code ~= unix.EAGAIN then
						return false
					end
				end
			else
				local errno = unix.ETIMEDOUT
				lasterr = testerror(false, unix.strerror(errno), errno)
			end
		end
		info("recvfromto (from:%s to:%s) -> FAIL (%s)", strname(from), strname(to), tostring(lasterr or "?"))
		return false
	end

	for ifa in unix.getifaddrs() do
		if ifa.family == family then
			local to = { family = family, addr = ifa.addr, port = port }
			local from = { family = family, addr = ifa.addr, port = port + 1 }

			nifaddrs = nifaddrs + 1
			if testit(to, from) then
				nokay = nokay + 1
			end
		end
	end

	unix.close(fd)
	unix.close(sd)

	return nokay, nifaddrs
end

local nokay, nifaddrs = 0, 0
for _,family in ipairs{ unix.AF_INET, unix.AF_INET6 } do
	local n, m = do_recvfromto(family, 8000)
	if n > 0 then
		info("recvfromto (%s) -> OK", strfamily(family))
	elseif m == 0 then
		info("recvfromto (%s) -> OK (no IPv6 interfaces)", strfamily(family))
	else
		info("recvfromto (%s) -> FAIL", strfamily(family))
	end

	nokay = nokay + n
	nifaddrs = nifaddrs + m
end

if nokay > 0 and nokay == nifaddrs then
	say"OK"
elseif nokay > 0 or nifaddrs == 0 then
	say"OK (rerun with -v to see exceptions)"
else
	say"FAIL (rerun with -v to see exceptions)"
end
