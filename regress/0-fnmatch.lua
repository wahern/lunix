#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local function files_fnmatch(rootdir, patt) 
	return coroutine.wrap(function ()
		local dir = assert(unix.opendir(rootdir))
		for name, type in dir:files("name", "type") do
			type = type or assert(unix.fstatat(dir, name, unix.AT_SYMLINK_NOFOLLOW, "mode"))
			local _, matched = assert(unix.fnmatch(patt, name))
			if matched and unix.S_ISREG(type) then
				coroutine.yield(name)
			end
		end
	end)
end

local function files_shfind(rootdir, patt) 
	local function quote(s)
		return string.format("'%s'", s:gsub("'", "'\"'\"'"))
	end
	return coroutine.wrap(function ()
		local fh = io.popen(string.format("cd %s && find . -name . -o -type d -prune -o \\( -name %s -type f \\) -print 2>/dev/null", quote(rootdir), quote(patt)), "r")
		for ln in fh:lines() do
			local name = ln:match("([^/]+)$")
			if name and name ~= "." then
				coroutine.yield(name)
			end
		end
	end)
end

local function files_fold(t, iter)
	for name in iter do
		t[#t + 1] = name
	end
	table.sort(t)
	return t
end

local list_fnmatch = files_fold({}, files_fnmatch("/etc", "*s*"))
local list_shfind = files_fold({}, files_shfind("/etc", "*s*"))

for i=1,math.max(#list_fnmatch, #list_shfind) do
	local a, b = list_fnmatch[i], list_shfind[i]
	if a ~= b then
		panic("discrepency between fnmatch(3) and find(1) (%s ~= %s)", tostring(a), tostring(b))
	end
end

say"OK"

