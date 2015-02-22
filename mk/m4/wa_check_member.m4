AC_DEFUN([WA_CHECK_SIZEOF_MEMBER], [
	AS_VAR_PUSHDEF([ac_Size], [ac_cv_sizeof_$1])
	AC_MSG_CHECKING([size of $1])
	AC_CACHE_VAL([ac_Size], [
		AC_COMPUTE_INT(
			[ac_Size],
			[sizeof (ac_aggr.m4_bpatsubst([$1], [^\([^.]*\)\.\(.*\)], [\2]))],
			[
				AC_INCLUDES_DEFAULT([$2])
				static m4_bpatsubst([$1], [^\([^.]*\)\.\(.*\)], [\1]) ac_aggr;
			],
			[AS_VAR_SET([ac_Size], [-1])]
		)
	])
	AS_IF(
		[test $ac_Size -gt 0], [
			AC_MSG_RESULT([$ac_Size])
			AC_DEFINE_UNQUOTED(AS_TR_CPP([SIZEOF_$1]), [$ac_Size], [Size of `$1'.])
		],
		[AC_MSG_RESULT([cannot compute])]
	)
	AS_VAR_POPDEF([ac_Size])
])

AC_DEFUN([WA_CHECK_SIZEOF_MEMBERS], [m4_map_args_sep([WA_CHECK_SIZEOF_MEMBER(], [, [$2])], [], $1)])

AC_DEFUN([WA_CHECK_OFFSETOF_MEMBER], [
	AS_VAR_PUSHDEF([ac_Offset], [ac_cv_offsetof_$1])
	AC_MSG_CHECKING([offset of $1])
	AC_CACHE_VAL([ac_Offset], [
		AC_COMPUTE_INT(
			[ac_Offset],
			[offsetof[](m4_bpatsubst([$1], [^\([^.]*\)\.\(.*\)], [\1]), m4_bpatsubst([$1], [^\([^.]*\)\.\(.*\)], [\2]))],
			[
				AC_INCLUDES_DEFAULT([$2])
				#ifdef STDC_HEADERS
				#include <stddef.h>
				#endif
			],
			[AS_VAR_SET(ac_Offset, -1)]
		)
	])
	AS_IF(
		[test $ac_Offset -gt 0], [
			AC_MSG_RESULT([$ac_Offset])
			AC_DEFINE_UNQUOTED(AS_TR_CPP([OFFSETOF_$1]), [$ac_Offset], [Offset of `$1'.])
		],
		[AC_MSG_RESULT([cannot compute])]
	)
	AS_VAR_POPDEF([ac_Offset])
])

AC_DEFUN([WA_CHECK_OFFSETOF_MEMBERS], [m4_map_args_sep([WA_CHECK_OFFSETOF_MEMBER(], [, [$2])], [], $1)])
