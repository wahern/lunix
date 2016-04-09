#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

local args = { "./getopt", "-ax", "-a", "x", "-b", "-bb", "--", "-a" }
local a = 0;
local b = 0;

for optc, optarg in unix.getopt(args, "a:b") do
	info("got option %s with argument %s", check(optc, "optc was nil"), optarg or "nil")

	if optc == "a" then
		check(optarg == "x", "expected optarg of 'x' with -a, got '%s'", tostring(optarg))
		a = a + 1
	elseif optc == "b" then
		check(optarg == nil, "expected nil optarg with -b")
		b = b + 1
	else
		panic("unexpected option: %s", optc)
	end
end

check(a == 2, "expected 2 -a, got %d", a)
check(b == 3, "expected 3 -b, got %d", b)
check(unix.optind == 8, "expected optind of 8, got %d", unix.optind)


local args = { "./getopt", "-a" }
local missing = 0

for optc, optarg in unix.getopt(args, ":a:") do
	info("got option %s with argument %s", check(optc, "optc was nil"), optarg or "nil")
	
	if optc == ":" then
		missing = missing + 1
		check(optarg == "a", "expected optarg to be 'a', got '%s'", optarg)
	else
		panic("unexpected option: %s", optc)
	end
end

check(missing == 1, "failed missig option test")


local args = { "./getopt", "-a" }
local illegal = 0

for optc, optarg in unix.getopt(args, ":b") do
	info("got option %s with argument %s", check(optc, "optc was nil"), optarg or "nil")

	if optc == "?" then
		illegal = illegal + 1
		check(optarg == "a", "expected optarg to be 'a', got '%s'", optarg)
	else
		panic("unexpected option: %s", optc)
	end
end

check(illegal == 1, "failed illegal option test")

say"OK"

