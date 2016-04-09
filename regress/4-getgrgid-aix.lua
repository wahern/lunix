#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

require"regress".export".*"
local unix = require"unix"

local gid, grp = check(unix.getgrgid(check(unix.getgid()), "gid", "name"))
info("real GID is %s", grp)

local uid, usr = check(unix.getpwuid(check(unix.getuid()), "uid", "name"))
info("real UID is %s", usr)

check(check(unix.getgrnam(grp, "gid")) == gid, "failed to map %s back to GID %d", grp, gid)
info("mapped %s back to GID %d", grp, gid)

check(check(unix.getpwnam(usr, "uid")) == uid, "failed to map %s back to UID %d", usr, uid)
info("mapped %s back to UID %d", usr, uid)

local pwd, why, error
repeat
	local usr = tmpnonce(6)
	pwd, why, error = unix.getpwnam(usr)
until not pwd

check(not error, "unable to force lookup of non-existent user (got '%s')", why)
info("lookup of non-existent user returns '%s'", why)

say"OK"