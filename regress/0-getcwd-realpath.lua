#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

--
-- getcwd
--
local cwd = check(unix.getcwd("."))
info("current working directory is %s", cwd)

--
-- realpath
--
local cwd1 = check(unix.realpath(cwd))
local cwd2 = check(unix.realpath(cwd1))
check(cwd1 == cwd2, "real paths don't match (expected '%s', got '%s')", cwd1, cwd2)
info("current working directory realpath is %s", cwd2)

--
-- realpath (negative)
--
local ENOTDIR = check(unix.ENOTDIR, "ENOTDIR not defined")
local ENOENT = check(unix.ENOENT, "ENOENT not defined")

local function strerrno(e)
	for k,v in pairs(unix) do
		if e == v and type(k) == "string" and k:find"^E" and not k:find"_" then
			return k
		end
	end

	return tostring(e)
end

for path in ("/nonexistent/file /etc/passwd/notdirectory"):gmatch"[^%s]+" do
	local rpath, err, errno = unix.realpath(path)
	check(not rpath, "expected realpath to fail on %s, got %s", path, rpath)
	if errno == ENOTDIR or errno == ENOENT then
		info("realpath on %s failed with %s", path, strerrno(errno))
	else
		say("realpath on %s failed with odd errno (expected ENOTDIR or ENOENT, got %s)", path, strerrno(errno))
	end
end

say"OK"
