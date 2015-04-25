AC_DEFUN([WA_C___EXTENSION__], [
	AS_VAR_PUSHDEF([ac_Symbol], [AS_TR_SH([ac_cv_c___extension__])])
	AC_CACHE_CHECK([for __extension__ annotation support], [ac_Symbol], [
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([],[return __extension__ 0;])
		], [AS_VAR_SET([ac_Symbol], [yes])], [AS_VAR_SET([ac_Symbol], [no])])
	])
	AS_VAR_IF([ac_Symbol], [yes], [AC_DEFINE([HAVE_C___EXTENSION__], [1], [Define to 1 if compiler supports __extension__ annotation])])
	AS_VAR_POPDEF([ac_Symbol])
])
