#!/bin/sh
_=[[
	. "${0%/*}/regress.sh"
	exec runlua -r5.2 "$0" "$@"
]]

local unix = require"unix"
local regress = require"regress".export".*"
local T = {}

local function spairs(t)
	local keys = {}
	for k in pairs(t) do
		keys[#keys + 1] = k
	end

	table.sort(keys)

	local i = 0
	return function ()
		if i < #keys then
			i = i + 1
			local k = keys[i]
			return k, t[k]
		end
	end
end

local function parsetest(s)
	local t = {}
	local match
	
	t.id, t.pattern, t.subject, match = assert(s:match("^" .. string.rep("%s+([^%s]+)", 4)))
	if t.subject == "NULL" then
		t.subject = ""
	end

	t.nomatch = match == "NOMATCH"

	local n = 0
	t.match = {}
	for so, eo in match:gmatch"%(([%d?]+),([%d?]+)%)" do
		t.match[n] = { so = tonumber(so) or -1, eo = tonumber(eo) or -1 }
		n = n + 1
	end

	return t
end

local function tests(s)
	return coroutine.wrap(function ()
		for l in s:gmatch"[^\n]+" do
			coroutine.yield(assert(parsetest(l)))
		end
	end)
end

local function runtest(class, t)
	local re, errstr = unix.regcomp(t.pattern, unix.REG_EXTENDED + unix.REG_ICASE)
	check(re, "%s (%s)", errstr, t.pattern)

	if t.nomatch then
		local match, errstr, error = unix.regexec(re, t.subject, {})
		check(not match, "expected no match, got match (%s)", t.pattern)
		check(error == unix.REG_NOMATCH, "expected REG_NOMATCH, got %s", tostring(error))
	else
		check(re.nsub == #t.match, "expected %d subexpressions, got %d (%s)", re.nsub, #t.match, t.pattern)

		local match, errstr = unix.regexec(re, t.subject, {})
		check(match, "%s (%s)", errstr, t.pattern)

		check(#match == #t.match, "expected %d matches, got %d (%s)", #t.match + 1, #match + 1, t.pattern)
		for i=0,#match do
			check(match[i].so == t.match[i].so, "expected %d, got %d for match %d begin (%s)", t.match[i].so, match[i].so, i, t.pattern)
		end
	end

	info("%s %d: OK (%s)", class, t.id, t.pattern)
end

-- from http://hackage.haskell.org/package/regex-posix-unittest-1.1/src/data-dir/basic3.txt
T.basic3 = [==[
	1		\)		()	(1,2)
	2		\}		}	(0,1)
	3		]		]	(0,1)
	4		$^		NULL	(0,0)
	5		a($)		aa	(1,2)(2,2)
	6		a*(^a)		aa	(0,1)(0,1)
	7		(..)*(...)*		a	(0,0)(?,?)(?,?)
	8		(..)*(...)*		abcd	(0,4)(2,4)(?,?)
	9		(ab|a)(bc|c)		abc	(0,3)(0,2)(2,3)
	10		(ab)c|abc		abc	(0,3)(0,2)
	11		a{0}b		ab			(1,2)
	12		(a*)(b?)(b+)b{3}	aaabbbbbbb	(0,10)(0,3)(3,4)(4,7)
	13		(a*)(b{0,1})(b{1,})b{3}	aaabbbbbbb	(0,10)(0,3)(3,4)(4,7)
	15		((a|a)|a)			a	(0,1)(0,1)(0,1)
	16		(a*)(a|aa)			aaaa	(0,4)(0,3)(3,4)
	17		a*(a.|aa)			aaaa	(0,4)(2,4)
	18		a(b)|c(d)|a(e)f			aef	(0,3)(?,?)(?,?)(1,2)
	19		(a|b)?.*			b	(0,1)(0,1)
	20		(a|b)c|a(b|c)			ac	(0,2)(0,1)(?,?)
	21		(a|b)c|a(b|c)			ab	(0,2)(?,?)(1,2)
	22		(a|b)*c|(a|ab)*c		abc	(0,3)(1,2)(?,?)
	23		(a|b)*c|(a|ab)*c		xc	(1,2)(?,?)(?,?)
	24		(.a|.b).*|.*(.a|.b)		xa	(0,2)(0,2)(?,?)
	25		a?(ab|ba)ab			abab	(0,4)(0,2)
	26		a?(ac{0}b|ba)ab			abab	(0,4)(0,2)
	27		ab|abab				abbabab	(0,2)
	28		aba|bab|bba			baaabbbaba	(5,8)
	29		aba|bab				baaabbbaba	(6,9)
	30		(aa|aaa)*|(a|aaaaa)		aa	(0,2)(0,2)(?,?)
	31		(a.|.a.)*|(a|.a...)		aa	(0,2)(0,2)(?,?)
	32		ab|a				xabc	(1,3)
	33		ab|a				xxabc	(2,4)
	34		(Ab|cD)*			aBcD	(0,4)(2,4)
	35		:::1:::0:|:::1:1:0:	:::0:::1:::1:::0:	(8,17)
	36		:::1:::0:|:::1:1:1:	:::0:::1:::1:::0:	(8,17)
	37		[[:lower:]]+		`az{		(1,3)
	38		[[:upper:]]+		@AZ[		(1,3)
	39		(a)(b)(c)	abc	(0,3)(0,1)(1,2)(2,3)
	43  	((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))	x	(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)
	44  	((((((((((((((((((((((((((((((x))))))))))))))))))))))))))))))*	xx	(0,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)(1,2)
	45		a?(ab|ba)*	ababababababababababababababababababababababababababababababababababababababababa	(0,81)(79,81)
	46		abaa|abbaa|abbbaa|abbbbaa	ababbabbbabbbabbbbabbbbaa	(18,25)
	47		abaa|abbaa|abbbaa|abbbbaa	ababbabbbabbbabbbbabaa	(18,22)
	48		aaac|aabc|abac|abbc|baac|babc|bbac|bbbc	baaabbbabac	(7,11)
	49		aaaa|bbbb|cccc|ddddd|eeeeee|fffffff|gggg|hhhh|iiiii|jjjjj|kkkkk|llll		XaaaXbbbXcccXdddXeeeXfffXgggXhhhXiiiXjjjXkkkXlllXcbaXaaaa	(53,57)
	50		a*a*a*a*a*b		aaaaaaaaab	(0,10)
	51		ab+bc			abbc		(0,4)
	52		ab+bc			abbbbc		(0,6)
	53		ab?bc			abbc		(0,4)
	54		ab?bc			abc		(0,3)
	55		ab?c			abc		(0,3)
	56		ab|cd			abc		(0,2)
	57		ab|cd			abcd		(0,2)
	58		a\(b			a(b		(0,3)
	59		a\(*b			ab		(0,2)
	60		a\(*b			a((b		(0,4)
	61		((a))			abc		(0,1)(0,1)(0,1)
	62		(a)b(c)			abc		(0,3)(0,1)(2,3)
	63		a+b+c			aabbabc		(4,7)
	64		a*			aaa		(0,3)
	65		(a*)*			-		(0,0)(0,0)
	66		(a*)+			-		(0,0)(0,0)
	67		(a*|b)*			-		(0,0)(0,0)
	68		(a+|b)*			ab		(0,2)(1,2)
	69		(a+|b)+			ab		(0,2)(1,2)
	70		(a+|b)?			ab		(0,1)(0,1)
	71		(^)*			-		(0,0)(0,0)
	72		([abc])*d		abbbcd		(0,6)(4,5)
	73		([abc])*bcd		abcd		(0,4)(0,1)
	74		a|b|c|d|e		e		(0,1)
	75		(a|b|c|d|e)f		ef		(0,2)(0,1)
	76		((a*|b))*		-		(0,0)(0,0)(0,0)
	77		(ab|cd)e		abcde		(2,5)(2,4)
	78		(a|b)c*d		abcd		(1,4)(1,2)
	79		(ab|ab*)bc		abc		(0,3)(0,1)
	80		a([bc]*)c*		abc		(0,3)(1,3)
	81		a([bc]*)(c*d)		abcd		(0,4)(1,3)(3,4)
	82		a([bc]+)(c*d)		abcd		(0,4)(1,3)(3,4)
	83		a([bc]*)(c+d)		abcd		(0,4)(1,2)(2,4)
	84		a[bcd]*dcdcde		adcdcde		(0,7)
	85		(ab|a)b*c		abc		(0,3)(0,2)
	86		((a)(b)c)(d)		abcd		(0,4)(0,3)(0,1)(1,2)(3,4)
	87		^a(bc+|b[eh])g|.h$	abh		(1,3)(?,?)
	88		(bc+d$|ef*g.|h?i(j|k))	effgz		(0,5)(0,5)(?,?)
	89		(bc+d$|ef*g.|h?i(j|k))	ij		(0,2)(0,2)(1,2)
	90		(bc+d$|ef*g.|h?i(j|k))	reffgz		(1,6)(1,6)(?,?)
	91		(((((((((a)))))))))	a		(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)(0,1)
	92		(.*)c(.*)		abcde		(0,5)(0,2)(3,5)
	93		a(bc)d			abcd		(0,4)(1,3)
	94		a[-]?c		ac		(0,3)
	95		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Qaddafi	(0,15)(?,?)(10,12)
	96		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Mo'ammar_Gadhafi	(0,16)(?,?)(11,13)
	97		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Kaddafi	(0,15)(?,?)(10,12)
	98		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Qadhafi	(0,15)(?,?)(10,12)
	99		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Gadafi	(0,14)(?,?)(10,11)
	100		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Mu'ammar_Qadafi	(0,15)(?,?)(11,12)
	101		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Moamar_Gaddafi	(0,14)(?,?)(9,11)
	102		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Mu'ammar_Qadhdhafi	(0,18)(?,?)(13,15)
	103		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Khaddafi	(0,16)(?,?)(11,13)
	104		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Ghaddafy	(0,16)(?,?)(11,13)
	105		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Ghadafi	(0,15)(?,?)(11,12)
	106		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Ghaddafi	(0,16)(?,?)(11,13)
	107		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muamar_Kaddafi	(0,14)(?,?)(9,11)
	108		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Quathafi	(0,16)(?,?)(11,13)
	109		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Muammar_Gheddafi	(0,16)(?,?)(11,13)
	110		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Moammar_Khadafy	(0,15)(?,?)(11,12)
	111		M[ou]'?am+[ae]r_.*([AEae]l[-_])?[GKQ]h?[aeu]+([dtz][dhz]?)+af[iy]	Moammar_Qudhafi	(0,15)(?,?)(10,12)
	112		a+(b|c)*d+		aabcdd			(0,6)(3,4)
	113		^.+$			vivi			(0,4)
	114		^(.+)$			vivi			(0,4)(0,4)
	115		^([^!.]+).att.com!(.+)$	gryphon.att.com!eby	(0,19)(0,7)(16,19)
	116		^([^!]+!)?([^!]+)$	bas			(0,3)(?,?)(0,3)
	117		^([^!]+!)?([^!]+)$	bar!bas			(0,7)(0,4)(4,7)
	118		^([^!]+!)?([^!]+)$	foo!bas			(0,7)(0,4)(4,7)
	119		^.+!([^!]+!)([^!]+)$	foo!bar!bas		(0,11)(4,8)(8,11)
	120		((foo)|(bar))!bas	bar!bas			(0,7)(0,3)(?,?)(0,3)
	121		((foo)|(bar))!bas	foo!bar!bas		(4,11)(4,7)(?,?)(4,7)
	122		((foo)|(bar))!bas	foo!bas			(0,7)(0,3)(0,3)(?,?)
	123		((foo)|bar)!bas		bar!bas			(0,7)(0,3)(?,?)
	124		((foo)|bar)!bas		foo!bar!bas		(4,11)(4,7)(?,?)
	125		((foo)|bar)!bas		foo!bas			(0,7)(0,3)(0,3)
	126		(foo|(bar))!bas		bar!bas			(0,7)(0,3)(0,3)
	127		(foo|(bar))!bas		foo!bar!bas		(4,11)(4,7)(4,7)
	128		(foo|(bar))!bas		foo!bas			(0,7)(0,3)(?,?)
	129		(foo|bar)!bas		bar!bas			(0,7)(0,3)
	130		(foo|bar)!bas		foo!bar!bas		(4,11)(4,7)
	131		(foo|bar)!bas		foo!bas			(0,7)(0,3)
	132		^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$	foo!bar!bas	(0,11)(0,11)(?,?)(?,?)(4,8)(8,11)
	133		^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$	bas		(0,3)(?,?)(0,3)(?,?)(?,?)
	134		^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$	bar!bas		(0,7)(0,4)(4,7)(?,?)(?,?)
	135		^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$	foo!bar!bas	(0,11)(?,?)(?,?)(4,8)(8,11)
	136		^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$	foo!bas		(0,7)(0,4)(4,7)(?,?)(?,?)
	137		^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$	bas		(0,3)(0,3)(?,?)(0,3)(?,?)(?,?)
	138		^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$	bar!bas		(0,7)(0,7)(0,4)(4,7)(?,?)(?,?)
	139		^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$	foo!bar!bas	(0,11)(0,11)(?,?)(?,?)(4,8)(8,11)
	140		^(([^!]+!)?([^!]+)|.+!([^!]+!)([^!]+))$	foo!bas		(0,7)(0,7)(0,4)(4,7)(?,?)(?,?)
	141		.*(/XXX).*			/XXX			(0,4)(0,4)
	142		.*(\\XXX).*			\XXX			(0,4)(0,4)
	143		\\XXX				\XXX			(0,4)
	144		.*(/000).*			/000			(0,4)(0,4)
	145		.*(\\000).*			\000			(0,4)(0,4)
	146		\\000				\000			(0,4)
]==]

-- from http://hackage.haskell.org/package/regex-posix-unittest-1.1/src/data-dir/repetition2.txt
T.repetition2 = [==[
	1	((..)|(.))	NULL	NOMATCH
	2	((..)|(.))((..)|(.))	NULL	NOMATCH
	3	((..)|(.))((..)|(.))((..)|(.))	NULL	NOMATCH
	4	((..)|(.)){1}	NULL	NOMATCH
	5	((..)|(.)){2}	NULL	NOMATCH
	6	((..)|(.)){3}	NULL	NOMATCH
	7	((..)|(.))*	NULL	(0,0)(?,?)(?,?)(?,?)
	8	((..)|(.))	a	(0,1)(0,1)(?,?)(0,1)
	9	((..)|(.))((..)|(.))	a	NOMATCH
	10	((..)|(.))((..)|(.))((..)|(.))	a	NOMATCH
	11	((..)|(.)){1}	a	(0,1)(0,1)(?,?)(0,1)
	12	((..)|(.)){2}	a	NOMATCH
	13	((..)|(.)){3}	a	NOMATCH
	14	((..)|(.))*	a	(0,1)(0,1)(?,?)(0,1)
	15	((..)|(.))	aa	(0,2)(0,2)(0,2)(?,?)
	16	((..)|(.))((..)|(.))	aa	(0,2)(0,1)(?,?)(0,1)(1,2)(?,?)(1,2)
	17	((..)|(.))((..)|(.))((..)|(.))	aa	NOMATCH
	18	((..)|(.)){1}	aa	(0,2)(0,2)(0,2)(?,?)
	19	((..)|(.)){2}	aa	(0,2)(1,2)(?,?)(1,2)
	20	((..)|(.)){3}	aa	NOMATCH
	21	((..)|(.))*	aa	(0,2)(0,2)(0,2)(?,?)
	22	((..)|(.))	aaa	(0,2)(0,2)(0,2)(?,?)
	23	((..)|(.))((..)|(.))	aaa	(0,3)(0,2)(0,2)(?,?)(2,3)(?,?)(2,3)
	24	((..)|(.))((..)|(.))((..)|(.))	aaa	(0,3)(0,1)(?,?)(0,1)(1,2)(?,?)(1,2)(2,3)(?,?)(2,3)
	25	((..)|(.)){1}	aaa	(0,2)(0,2)(0,2)(?,?)
	26	((..)|(.)){2}	aaa	(0,3)(2,3)(?,?)(2,3)
	27	((..)|(.)){3}	aaa	(0,3)(2,3)(?,?)(2,3)
	28	((..)|(.))*	aaa	(0,3)(2,3)(?,?)(2,3)
	29	((..)|(.))	aaaa	(0,2)(0,2)(0,2)(?,?)
	30	((..)|(.))((..)|(.))	aaaa	(0,4)(0,2)(0,2)(?,?)(2,4)(2,4)(?,?)
	31	((..)|(.))((..)|(.))((..)|(.))	aaaa	(0,4)(0,2)(0,2)(?,?)(2,3)(?,?)(2,3)(3,4)(?,?)(3,4)
	32	((..)|(.)){1}	aaaa	(0,2)(0,2)(0,2)(?,?)
	33	((..)|(.)){2}	aaaa	(0,4)(2,4)(2,4)(?,?)
	34	((..)|(.)){3}	aaaa	(0,4)(3,4)(?,?)(3,4)
	35	((..)|(.))*	aaaa	(0,4)(2,4)(2,4)(?,?)
	36	((..)|(.))	aaaaa	(0,2)(0,2)(0,2)(?,?)
	37	((..)|(.))((..)|(.))	aaaaa	(0,4)(0,2)(0,2)(?,?)(2,4)(2,4)(?,?)
	38	((..)|(.))((..)|(.))((..)|(.))	aaaaa	(0,5)(0,2)(0,2)(?,?)(2,4)(2,4)(?,?)(4,5)(?,?)(4,5)
	39	((..)|(.)){1}	aaaaa	(0,2)(0,2)(0,2)(?,?)
	40	((..)|(.)){2}	aaaaa	(0,4)(2,4)(2,4)(?,?)
	41	((..)|(.)){3}	aaaaa	(0,5)(4,5)(?,?)(4,5)
	42	((..)|(.))*	aaaaa	(0,5)(4,5)(?,?)(4,5)
	43	((..)|(.))	aaaaaa	(0,2)(0,2)(0,2)(?,?)
	44	((..)|(.))((..)|(.))	aaaaaa	(0,4)(0,2)(0,2)(?,?)(2,4)(2,4)(?,?)
	45	((..)|(.))((..)|(.))((..)|(.))	aaaaaa	(0,6)(0,2)(0,2)(?,?)(2,4)(2,4)(?,?)(4,6)(4,6)(?,?)
	46	((..)|(.)){1}	aaaaaa	(0,2)(0,2)(0,2)(?,?)
	47	((..)|(.)){2}	aaaaaa	(0,4)(2,4)(2,4)(?,?)
	48	((..)|(.)){3}	aaaaaa	(0,6)(4,6)(4,6)(?,?)
	49	((..)|(.))*	aaaaaa	(0,6)(4,6)(4,6)(?,?)
	100	X(.?){0,}Y	X1234567Y	(0,9)(7,8)
	101	X(.?){1,}Y	X1234567Y	(0,9)(7,8)
	102	X(.?){2,}Y	X1234567Y	(0,9)(7,8)
	103	X(.?){3,}Y	X1234567Y	(0,9)(7,8)
	104	X(.?){4,}Y	X1234567Y	(0,9)(7,8)
	105	X(.?){5,}Y	X1234567Y	(0,9)(7,8)
	106	X(.?){6,}Y	X1234567Y	(0,9)(7,8)
	107	X(.?){7,}Y	X1234567Y	(0,9)(7,8)
	108	X(.?){8,}Y	X1234567Y	(0,9)(8,8)
	110	X(.?){0,8}Y	X1234567Y	(0,9)(7,8)
	111	X(.?){1,8}Y	X1234567Y	(0,9)(7,8)
	112	X(.?){2,8}Y	X1234567Y	(0,9)(7,8)
	113	X(.?){3,8}Y	X1234567Y	(0,9)(7,8)
	114	X(.?){4,8}Y	X1234567Y	(0,9)(7,8)
	115	X(.?){5,8}Y	X1234567Y	(0,9)(7,8)
	116	X(.?){6,8}Y	X1234567Y	(0,9)(7,8)
	117	X(.?){7,8}Y	X1234567Y	(0,9)(7,8)
	118	X(.?){8,8}Y	X1234567Y	(0,9)(8,8)
	260	(a|ab|c|bcd){0,}(d*)	ababcd	(0,6)(3,6)(6,6)
	261	(a|ab|c|bcd){1,}(d*)	ababcd	(0,6)(3,6)(6,6)
	262	(a|ab|c|bcd){2,}(d*)	ababcd	(0,6)(3,6)(6,6)
	263	(a|ab|c|bcd){3,}(d*)	ababcd	(0,6)(3,6)(6,6)
	264	(a|ab|c|bcd){4,}(d*)	ababcd	NOMATCH
	265	(a|ab|c|bcd){0,10}(d*)	ababcd	(0,6)(3,6)(6,6)
	266	(a|ab|c|bcd){1,10}(d*)	ababcd	(0,6)(3,6)(6,6)
	267	(a|ab|c|bcd){2,10}(d*)	ababcd	(0,6)(3,6)(6,6)
	268	(a|ab|c|bcd){3,10}(d*)	ababcd	(0,6)(3,6)(6,6)
	269	(a|ab|c|bcd){4,10}(d*)	ababcd	NOMATCH
	270	(a|ab|c|bcd)*(d*)	ababcd	(0,6)(3,6)(6,6)
	271	(a|ab|c|bcd)+(d*)	ababcd	(0,6)(3,6)(6,6)
]==]

for class, s in spairs(T) do
	for t in tests(s) do
		runtest(class, t)
	end
end

say"OK"
