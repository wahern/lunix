#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

require"regress".export".*"
local unix = require"unix"
local tmpdir = check(mkdtemp())

local nofile = 256

for i=1,nofile do
	local path = string.format("%s/%02x-%s", tmpdir, i - 1, tmpnonce())
	check(io.open(path, "w+")):close()
end

info("created %d files in %s", nofile, tmpdir)

local n = 0

for name in check(unix.opendir(tmpdir)):files"name" do
	if name ~= "." and name ~= ".." then
		n = n + 1
	end
end

check(n == nofile, "expected to see %d files, but only saw %d", nofile, n)
info("found all %d files in %s", nofile, tmpdir)


local function tryfail(how)
	unix.chmod(tmpdir, "u+rwx") -- undo any previous chmod

	local dir = check(unix.opendir(tmpdir))

	how(dir)

	repeat
		local name, why, error = unix.readdir(dir, "name")

		if name then
			how(dir, name)
		elseif error then
			return error
		end
	until not name

	dir:close()
end -- tryfail

local function dofail(t)
	for _, how in ipairs(t) do
		info("%s", how.text)

		local error = tryfail(how.func)

		if error then
			return error
		end
	end
end -- dofail

local goterror = dofail{
	{
		text = "testing readdir_r error by changing file permissions during read",
		func = function (dir, name)
			if name then
				check(unix.chmod(dir, "0000"), "unable to chmod")
			end
		end
	},
	{
		text = "testing readdir_r error by changing file permissions before read",
		func = function (dir, name)
			if not name then
				check(unix.chmod(dir, "0000"), "unable to chmod")
			end
		end
	},
	{
		text = "testing readdir_r error by duping over descriptor during read",
		func = function (dir, name)
			if name then
				local fh = check(io.tmpfile())
				check(unix.dup2(fh, dir), "unable to dup2")
				fh:close()
			end
		end
	},
	{
		text = "testing readdir_r error by duping over descriptor before read",
		func = function (dir, name)
			if not name then
				local fh = check(io.tmpfile())
				check(unix.dup2(fh, dir), "unable to dup2")
				fh:close()
			end
		end
	},
}

check(goterror, "unable to induce readdir_r error")
info("induced readdir_r error: %s", unix.strerror(goterror))

say"OK"

