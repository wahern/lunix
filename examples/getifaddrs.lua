#!/usr/bin/env lua

local unix = require"unix"

for ifa in unix.getifaddrs() do
	print("--")
	for k,v in pairs(ifa) do
		print(k, v)
	end
end
