local unix = require"unix"

local regress = { }

regress.mpsafe = false
regress.mtsafe = false
regress.logpid = false

local _lockfh
function regress.setmpsafe(mpsafe)
	regress.mpsafe = mpsafe

	if regress.mpsafe and not _lockfh then
		_lockfh = assert(io.tmpfile())
	end
end

local _progname
local function progname()
	_progname = _progname or string.gsub(os.getenv"PROGNAME" or "regress", ".lua$", "")
	return _progname
end

local function lock_stderr()
	if regress.mpsafe then
		_lockfh = _lockfh or assert(io.tmpfile())
		assert(unix.lockf(_lockfh, unix.F_LOCK))
	end

	if regress.mtsafe then
		unix.flockfile(_lockfh)
	end
end -- lock_stderr

local function unlock_stderr()
	if regress.mtsafe then
		unix.funlockfile(_lockfh)
	end

	if _lockfh then
		assert(unix.lockf(_lockfh, unix.F_ULOCK))
	end
end -- unlock_stderr

function regress.say(fmt, ...)
	local pid = regress.logpid and string.format("[%d] ", unix.getpid()) or ""
	lock_stderr()
	io.stderr:write(progname(), ": ", pid, string.format(fmt, ...), "\n")
	unlock_stderr()
end -- say

function regress.panic(...)
	regress.say(...)
	os.exit(false, true)
end -- panic

local verbose = false

for _, v in ipairs(arg or {}) do
	if v == "-v" then
		verbose = true
	end
end

function regress.info(...)
	if verbose then
		regress.say(...)
	end
end -- info

function regress.check(v, ...)
	if v then
		return v, ...
	else
		regress.panic(...)
	end
end -- check

function regress.import(t, ...)
	for _, pat in ipairs{ ... } do
		for k, v in pairs(regress) do
			if string.match(k, pat) then
				t[k] = v
			end
		end
	end

	return regress
end -- import

function regress.export(...)
	return regress.import(_G, ...)
end -- export

local base32 = {
	"A", "B", "C", "D", "E", "F", "G", "H",
	"I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X",
	"Y", "Z", "2", "3", "4", "5", "6", "7",
}

function regress.tmpnonce(n)
	return string.gsub(unix.arc4random_buf(n or 16), ".", function (s)
		return base32[math.fmod(string.byte(s), #base32) + 1]
	end)
end -- regress.tmpnonce

function regress.tmpdir()
	local tmpdir = os.getenv"TMPDIR" or "/tmp"

	return string.format("%s/%s-%s", tmpdir, progname(), regress.tmpnonce())
end -- regress.tmpdir

function regress.mkdtemp()
	local tmpdir = regress.tmpdir()

	regress.atexit(function ()
		unix.chmod(tmpdir, "u+rwx") --> ignore any errors

		local dir, why, error = unix.opendir(tmpdir)

		if not dir then
			if error ~= unix.ENOENT then
				say("%s: %s", tmpdir, why)
			end

			return
		end

		for name in dir:files"name" do
			if name ~= "." and name ~= ".." then
				local path = tmpdir .. "/" .. name
				local ok, why = unix.unlink(path)

				if not ok then
					say("%s: %s", path, why)
				end
			end
		end

		local ok, why = unix.rmdir(tmpdir)

		if not ok then
			say("%s: %s", tmpdir, why)
		end
	end)

	local ok, why, error = unix.mkdir(tmpdir, "0700")

	if not ok then
		return nil, why, error
	else
		return tmpdir
	end
end -- mkdtemp

local onexit = setmetatable({ }, { __gc = function (t)
	for i=1,#t do
		local ok, why = pcall(t[i])

		if not ok then
			regress.say("%s", why)
		end
	end
end })

function regress.atexit(f)
	onexit[#onexit + 1] = f
end -- regress.atexit

return regress
