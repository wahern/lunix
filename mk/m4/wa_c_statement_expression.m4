AC_DEFUN([WA_C_STATEMENT_EXPRESSION], [
	AS_VAR_PUSHDEF([ac_Symbol], [AS_TR_SH([ac_cv_c_statement_expression])])
	AC_CACHE_CHECK([for statement expression support], [ac_Symbol], [
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([],[return ({ 0; });])
		], [AS_VAR_SET([ac_Symbol], [yes])], [AS_VAR_SET([ac_Symbol], [no])])
	])
	AS_VAR_IF([ac_Symbol], [yes], [AC_DEFINE([HAVE_C_STATEMENT_EXPRESSION], [1], [Define to 1 if compiler supports GCC statement expressions])])
	AS_VAR_POPDEF([ac_Symbol])
])
