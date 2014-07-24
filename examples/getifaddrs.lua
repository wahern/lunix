#!/usr/bin/env lua
--
-- very simple ifconfig(1) clone
--
local unix = require"unix"

-- memoize reverse mapping of constants and group by identifier pattern
local map = {}
local function getsubmap(pat)
	local submap = map[pat]

	if not submap then
		submap = {}

		for k,v in pairs(unix) do
			if k:match(pat) then
				submap[v] = k
			end
		end

		map[pat] = submap
	end

	return submap
end -- getsubmap


-- convert v to string identfier name based on identifier pattern pat
local function strconst(pat, v)
	return tostring(getsubmap(pat)[v] or v)
end -- strconst


-- iterator over strconst applied to each bit in bit field
local function strcbits(pat, bits, w)
	local i = 0

	return function ()
		local n

		while not n and i < (w or 32) do
			if bits % 2 == 1 then
				n = 2^i
				bits = bits - 1
			end

			i = i + 1
			bits = bits / 2
		end

		if n then
			return strconst(pat, n)
		end
	end
end -- strcbits


local function strfamily(af)
	return strconst("^AF_", af)
end -- strfamily


local function striffs(iffs)
	local t = {}

	for k in strcbits("^IFF_", iffs, 20) do
		t[#t + 1] = k
	end

	return #t > 0 and table.concat(t, ",") or tostring(iffs)
end -- striffs


local fmt = string.format

local function say(...)
	return io.stdout:write(fmt(...), "\n")
end

for ifa in unix.getifaddrs() do
	if ifa.family == unix.AF_INET or ifa.family == unix.AF_INET6 then
		local netmask = ifa.netmask and fmt(" netmask %s", ifa.netmask) or ""
		local prefixlen = ifa.prefixlen and fmt(" prefixlen %d", ifa.prefixlen) or ""
		local network = ifa.family == unix.AF_INET and netmask or prefixlen
		local broadaddr = ifa.broadaddr and fmt(" broadcast %s", ifa.broadaddr) or ""

		say("%s: flags=%d<%s>", ifa.name, ifa.flags, striffs(ifa.flags):gsub("IFF_", ""))
		say("\t%s %s%s%s", strfamily(ifa.family):gsub("^AF_", ""):lower(), ifa.addr, network, broadaddr)
	end
end
