local regress = { }

function regress.say(fmt, ...)
	io.stderr:write(os.getenv"PROGNAME" or "regress", ": ", string.format(fmt, ...), "\n")
end -- say

function regress.panic(...)
	regress.say(...)
	os.exit(false)
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

function regress.export(...)
	for _, pat in ipairs{ ... } do
		for k, v in pairs(regress) do
			if string.match(k, pat) then
				_G[k] = v
			end
		end
	end

	return regress
end -- export

return regress
