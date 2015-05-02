AC_DEFUN([WA_C__GENERIC], [
	AS_VAR_PUSHDEF([ac_Symbol], [AS_TR_SH([ac_cv_c__generic])])
	AC_CACHE_CHECK([for C11 _Generic expression support], [ac_Symbol], [
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([],[
				long T = -1;
				int a@<:@_Generic(T, default: -1, long: 1)@:>@ = { 0 };
				return a@<:@0@:>@;
			])
		], [AS_VAR_SET([ac_Symbol], [yes])], [AS_VAR_SET([ac_Symbol], [no])])
	])
	AS_VAR_IF([ac_Symbol], [yes], [AC_DEFINE([HAVE_C__GENERIC], [1], [Define to 1 if compiler supports C11 _Generic expressions])])
	AS_VAR_POPDEF([ac_Symbol])
])
