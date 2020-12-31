
AC_PREREQ([2.69])
AC_INIT([asm2ws], [v0.9], [kspalaiologos@gmail.com])
AC_CONFIG_SRCDIR([wsi.c])
AC_CONFIG_HEADERS([config.h])
AC_COPYRIGHT([Copyright (C) Kamila Szewczyk, 2021. Licensed under the terms of GPLv3 license.])
AC_LANG([C])

AC_DEFUN([AX_CHECK_COMPILE_FLAG],
[AC_PREREQ(2.64)dnl for _AC_LANG_PREFIX and AS_VAR_IF
AS_VAR_PUSHDEF([CACHEVAR],[ax_cv_check_[]_AC_LANG_ABBREV[]flags_$4_$1])dnl
AC_CACHE_CHECK([whether _AC_LANG compiler accepts $1], CACHEVAR, [
  ax_check_save_flags=$[]_AC_LANG_PREFIX[]FLAGS
  _AC_LANG_PREFIX[]FLAGS="$4 $1"
  AC_COMPILE_IFELSE([m4_default([$5],[AC_LANG_PROGRAM()])],
    [AS_VAR_SET(CACHEVAR,[yes])],
    [AS_VAR_SET(CACHEVAR,[no])])
  _AC_LANG_PREFIX[]FLAGS=$ax_check_save_flags])
AS_VAR_IF(CACHEVAR,yes,
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl AX_CHECK_COMPILE_FLAGS

AC_DEFUN([CHK_FLAG2], [
    AX_CHECK_COMPILE_FLAG([$1],
            [AC_SUBST([$2], ["$$2 $1"])], [
                AC_MSG_WARN([$1 is beneficial for debug builds, but it's not present.])
            ], [-Werror])
])

AC_DEFUN([CHK_FLAG], [
    CHK_FLAG2($1, [LOC_F])
])

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX
AC_PROG_MAKE_SET

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h limits.h stddef.h stdint.h stdlib.h string.h assert.h stdio.h libtcc.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_ARG_ENABLE([jit],
    AS_HELP_STRING([--enable-jit], [enable the TinyCC-based JIT compiler]), [
        AC_DEFINE([JIT], [], [enable the TinyCC-based JIT compiler])
])

AC_ARG_WITH([target],
            AS_HELP_STRING([--with-target=@<:@auto/release/debug/coverage@:>@],
                           [Build target (default: auto)]),
            [asm2ws_distro_target=$withval],
            [asm2ws_distro_target=auto])

case "$asm2ws_distro_target" in
    "release")
        AC_SUBST([DISTRO], ["-Ofast -march=native -funroll-loops -fomit-frame-pointer"])
        ;;
    "debug")
        AC_SUBST([LOC_F], [])
        CHK_FLAG([-fsanitize=pointer-compare])
        CHK_FLAG([-fsanitize=pointer-subtract])
        CHK_FLAG([-fsanitize=address])
        CHK_FLAG([-fsanitize=leak])
        CHK_FLAG([-fsanitize=undefined])
        CHK_FLAG([-fcf-protection=full])
        CHK_FLAG([-fstack-protector-strong])
        CHK_FLAG([-fstack-check])
        
        AC_SUBST([DISTRO], m4_normalize(["
            -Wall -Wextra
            -g3 -O2
            $LOC_F
        "]))
        ;;
    "coverage")
        AX_CHECK_COMPILE_FLAG([-coverage],
            ,
            [AC_MSG_FAILURE([-coverage is required for coverage builds.])],
            [-Werror])
        
        AC_SUBST([DISTRO], ["-g3 -O0 -coverage"])
        ;;
    *)
        AC_SUBST([DISTRO], ["-O2"])
        ;;
esac

AC_ARG_WITH([jobs],
            AS_HELP_STRING([--with-jobs=N],
                           [Set the amount of make jobs. Must be greater than zero and lesser or equal to 8.]),
            [asm2ws_jobs=$withval],
            [asm2ws_jobs=1])

case "$asm2ws_jobs" in
    1)
        AC_SUBST([MSFLAGS], [""])
        ;;
    2|3|4|5|6|7|8)
        AC_SUBST([MSFLAGS], ["-j $asm2ws_jobs "])
        ;;
    *)
        AC_MSG_FAILURE([Pass a number in range 1-8 to --with-jobs.])
        ;;
esac

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([printf scanf putchar getchar realloc malloc free fopen fread fprintf perror exit])

AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([tcc], [tcc_new])

dnl Autoconf should have put it before, no clue how to force it to do so.

test "x$prefix" = xNONE && prefix=$ac_default_prefix
AC_SUBST([PREFIX], ["$prefix/"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
