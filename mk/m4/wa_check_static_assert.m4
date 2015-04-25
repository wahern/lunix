AC_DEFUN([WA_CHECK_STATIC_ASSERT_BODY], [
	AS_VAR_PUSHDEF([ac_Symbol], [AS_TR_SH([ac_cv_$1])])
	AC_CACHE_CHECK([for $1], [ac_Symbol], [
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM([$4],[[
				$1(1, "");
				return 0;
			]])
		], [AS_VAR_SET([ac_Symbol], [yes])], [AS_VAR_SET([ac_Symbol], [no])])
	])
	AS_VAR_IF([ac_Symbol], [yes], [AC_DEFINE(AS_TR_CPP([HAVE_$1]), [1], [Define to 1 if have $1.])])
	AS_VAR_IF([ac_Symbol], [yes], [$2], [$3])
	AS_VAR_POPDEF([ac_Symbol])
])

AC_DEFUN([WA_CHECK_STATIC_ASSERT], [
	WA_CHECK_STATIC_ASSERT_BODY([static_assert],[],[],[#include <assert.h>])
	WA_CHECK_STATIC_ASSERT_BODY([_Static_assert],[],[],[])
])
