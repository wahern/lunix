#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"

regress.setmpsafe(true)

local fmt = string.format

local function checkf(what, v, ...)
	if v then
		info("%s: OK", what)
		return v, ...
	else
		regress.panic("%s: %s", what, ...)
	end
end

local function stroflags(flags)
	local names = {}

	for k,v in pairs(unix) do
		if type(k) == "string" and type(v) == "number" then
			if k:match"^O_" and 0 ~= unix.bitand(flags, v) then
				names[#names + 1] = k

				flags = unix.bitand(flags, unix.compl(v))
			end
		end
	end

	-- NB: on AIX flags can be greater than 2^32-1
	for i=0,31 do
		local flag = math.pow(2, i)
		if 0 ~= unix.bitand(flag, flags) then
			names[#names + 1] = fmt("0x%.8x", flag)
		end
	end

	return table.concat(names,"|")
end

local function strsigname(signo)
	for k,v in pairs(unix) do
		if type(k) == "string" and k:match"^SIG[^_]" and v == signo then
			return k
		end
	end
	return fmt("SIG(%d)", signo)
end


local NSLAVES = 3
local NHUPS = NSLAVES
local NALRMS = NSLAVES - NHUPS --> 0
local CLOSE_MTTY = { [1] = true, [2] = false, [3] = true }
local CLOSE_STTY = { [1] = true, [2] = true, [3] = true }
local REOPEN_STTY = { [1] = true, [2] = false, [3] = false }

local pipe_rd, pipe_wr = check(unix.fpipe())
pipe_wr:setvbuf"line"

local mtty_pid = unix.fork()
if mtty_pid > 0 then
	info("forked leader %d", mtty_pid)
	check(unix.close(pipe_wr))

	local _, how, status = check(unix.waitpid(mtty_pid))
	check(how == "exited" and status == 0, "session leader died (%s with %d)", how, status or -1)

	local rcvd = { SIGHUP = 0, SIGALRM = 0 }
	for sig in pipe_rd:lines() do
		if rcvd[sig] then
			rcvd[sig] = rcvd[sig] + 1
		else
			info("%s: unexpected signal", sig)
		end
	end

	check(NHUPS == rcvd.SIGHUP, "expected %d SIGHUPs, got %d", NHUPS, rcvd.SIGHUP)
	check(NALRMS == rcvd.SIGALRM, "expected %d SIGALRMSs, got %d", NALRMS, rcvd.SIGALRM)

	say"OK"
	os.exit(0)
end

check(unix.close(pipe_rd))
regress.logpid = true

--
-- Become leader of new session and process group, disassociated from any
-- controlling terminal.
--
checkf("setsid", unix.setsid())
info("%d is session leader", unix.getpid())

--
-- Create a new pty master/slave pair.
--
-- POSIX leaves as implemented-defined how to assign a controlling terminal
-- to a session. See 11.1.3 The Controlling Terminal in IEEE Std
-- 1003.1-2013. AFAIU, some systems require TIOCSCTTY, while others
-- automatically make the first pty opened the controlling terminal.
--
-- For our test, if TIOCSCTTY is defined then we'll open the pty with
-- O_NOCTTY and use TIOCSCTTY. Otherwise we won't specify O_NOCTTY and
-- assume that opening the slave pty will make it the controlling terminal.
-- Note that these are not the only possible implementation permutations.
--
local mtty_flags = unix.bitor(unix.O_RDWR, unix.O_NOCTTY)
local mtty_how = fmt("posix_openpt(%s)", stroflags(mtty_flags))
local mtty = checkf(mtty_how, unix.posix_openpt(unix.bitor(unix.O_RDWR, unix.O_NOCTTY)))

checkf("grantpt", unix.grantpt(mtty))
checkf("unlockpt", unix.unlockpt(mtty))

local stty_path = checkf("ptsname", unix.ptsname(mtty))
local stty_flags = unix.bitor(unix.O_RDWR, unix.TIOCSCTTY and unix.O_NOCTTY or 0)
local stty_how = fmt("open(%s, %s)", stty_path, stroflags(stty_flags))
local stty = checkf(stty_how, unix.open(stty_path, stty_flags))

--
-- Set slave pty as controlling terminal of our session. The foreground
-- process group of the terminal will be initialized to our current process
-- group.
--
info("tcgetpgrp: %s", tostring(unix.tcgetpgrp(stty)))

if unix.TIOCSCTTY then
	checkf("TIOCSCTTY", unix.ioctl(stty, unix.TIOCSCTTY))
	info("tcgetpgrp: %s", tostring(unix.tcgetpgrp(stty)))
end

-- tcsetpgrp is probably a noop at this point
checkf("tcsetpgrp", unix.tcsetpgrp(stty, unix.getpgrp()))
info("tcgetpgrp: %s", tostring(unix.tcgetpgrp(stty)))

--
-- Fork children to test SIGHUP.
--
-- POSIX doesn't require SIGHUP to be sent to processes in the foreground
-- process group. It only requires SIGHUP to be sent to the controlling
-- terminal. See 11.1.10 Modem Disconnect in IEEE Std 1003.1-2013. But
-- sending SIGHUP to the foreground process group is historical behavior and
-- what we're expecting to happen.
--
for i=1,NSLAVES do
	unix.sigprocmask(unix.SIG_BLOCK, "*");

	local stty_pid = check(unix.fork())
	if stty_pid == 0 then
		-- it shouldn't matter whether we close the master tty
		if CLOSE_MTTY[i] then
			checkf("close", unix.close(mtty))
			info"closed master tty"
		end

		-- we expect SIGHUP to be sent to all processes if at least
		-- one process has the slave tty open
		if CLOSE_STTY[i] then
			checkf(fmt("close(%s)", stty_path), unix.close(stty))

			if REOPEN_STTY[i] then
				stty = checkf(stty_how, unix.open(stty_path, stty_flags))
			else
				info("skipping slave tty reopen")
			end
		end

		local timeout = 3
		unix.alarm(timeout);

		local unblock = unix.sigemptyset()
		unix.sigaddset(unblock, unix.SIGHUP)
		unix.sigaddset(unblock, unix.SIGALRM)

		local signo = check(unix.sigtimedwait(unblock, timeout + 1))
		info("rcvd %s", strsigname(signo))
		pipe_wr:write(strsigname(signo), "\n")

		info"exiting"
		os.exit(0)
	else
		info("leader forked %d", stty_pid)
	end
end

-- give children time to finish any tty manipulations
unix.sleep(1)

info"exiting"
os.exit(0)
