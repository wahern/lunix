AC_DEFUN([WA_CHECK_VAR_BODY], [AC_LINK_IFELSE([
	AC_LANG_PROGRAM([],[[
		extern char $1[];
		return $1[0];
	]])
], [AS_VAR_SET([ac_Symbol], [yes])], [AS_VAR_SET([ac_Symbol], [no])])])

AC_DEFUN([WA_CHECK_VAR], [
	AS_VAR_PUSHDEF([ac_Symbol], [ac_cv_var_$1])
	AC_CACHE_CHECK([for $1], [ac_Symbol], [WA_CHECK_VAR_BODY([$1])])
	AS_VAR_IF([ac_Symbol], [yes], [AC_DEFINE(AS_TR_CPP([HAVE_$1]), [1], [Define to 1 if variable $1 is defined (but not necessarily declared).])])
	AS_VAR_IF([ac_Symbol], [yes], [$2], [$3])
	AS_VAR_POPDEF([ac_Symbol])
])

AC_DEFUN([WA_CHECK_VARS], [m4_map_args_w([$1], [WA_CHECK_VAR(], [, [$2], [$3])])])
