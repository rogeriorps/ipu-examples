#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ipu-examples], [0.1], [rpimentel.silva@gmail.com])
AC_CONFIG_SRCDIR([mx6/alphablending/example1/alpha_ex1.c])
S
#AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([ipu-examples], [0.1])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday memset munmap])

##########################################################################
# Checks for programs needed for tests
##########################################################################

AM_CONDITIONAL(TARGET_MX5, test "$target" = mx5)
AM_CONDITIONAL(TARGET_MX6, test "$target" = mx6)

AC_CONFIG_FILES([Makefile
		mx5/basic/example1/Makefile
      		mx5/rotation/example1/Makefile
		mx6/alphablending/example1/Makefile
               	mx6/alphablending/example2/Makefile
              	mx6/cropping/example1/Makefile
                mx6/csc/example1/Makefile
                mx6/deinterlacing/example1/Makefile
                mx6/resizing/example1/Makefile
                mx6/rotation/example1/Makefile])

if test "x$target" = "x"; then
	AC_MSG_NOTICE([==> Set target option: --target=mx5 or mx6])
	AC_MSG_ERROR([target variable is mandatory.])

elif test "$target" = "mx5"; then
	AC_MSG_NOTICE([==> Used target i.MX5!])

elif test "$target" = "mx6"; then
	AC_MSG_NOTICE([==> Used target i.MX6!])

else
	AC_MSG_NOTICE([==> Invalid target. Set target option: --target=mx5 or mx6])
	AC_MSG_ERROR([target variable is mandatory.])
fi

AC_OUTPUT
