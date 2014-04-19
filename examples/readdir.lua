#!/usr/bin/env lua

local unix = require"unix"

local tmap = {
	b = unix.S_ISBLK,
	c = unix.S_ISCHR,
	d = unix.S_ISDIR,
	l = unix.S_ISLNK,
	s = unix.S_ISSOCK,
	p = unix.S_ISFIFO,
	f = unix.S_ISREG,
}

local function tname(mode)
	for name, f in pairs(tmap) do
		if f(mode) then
			return name
		end
	end

	return "-"
end -- tname


local tmp = unix.opendir(io.open(... or ".", "r"))
local dir = unix.opendir(tmp) --> can open from existing handle
tmp:close()

for name, type in dir:files("name", "type") do
	print(tname(type), name)
end

dir:close()
