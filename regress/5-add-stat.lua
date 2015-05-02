#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local path = "."
local dir = check(unix.opendir(path))

for name, ino in dir:files("name", "ino") do
	info("%s", name)

	local st = check(unix.stat(path .. "/" .. name))

	check(ino == st.ino, "%s: inode mismatch (%d, %d)", name, ino, st.ino)

	local w = 0
	for k in pairs(st) do w = math.max(#k, w) end

	for k, v in pairs(st) do
		info("\t%s: %s%s", k, string.rep(" ", w - #k), v)
	end
end

local path_st = { check(unix.stat(path, "dev", "ino", "mode", "nlink", "uid", "gid", "size")) }
local dir_st = { check(unix.stat(dir, "dev", "ino", "mode", "nlink", "uid", "gid", "size")) }

check(#path_st == #dir_st and #path_st == 7, "stat list form calling broken")

for i=1,#path_st do
	check(path_st[i] == dir_st[i], "mismatched values (%s, %s)", tostring(path_st[i]), tostring(dir_st[i]))
	check(path_st[i] ~= nil, "required field is nil")
end

say"OK"
