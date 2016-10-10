AC_INIT([ipu-examples], [1.0])

##########################################################################
# Checks for programs needed for tests
##########################################################################
AM_CONDITIONAL(TARGET_MX5, test "$target" = mx5)
AM_CONDITIONAL(TARGET_MX6, test "$target" = mx6)

AC_CONFIG_FILES([Makefile
		mx5/framebuffer/example1/Makefile
      		mx5/rotation/example1/Makefile
		mx6/alphablending/example1/Makefile
               	mx6/alphablending/example2/Makefile
              	mx6/cropping/example1/Makefile
                mx6/csc/example1/Makefile
                mx6/deinterlacing/example1/Makefile
                mx6/resizing/example1/Makefile
                mx6/rotation/example1/Makefile
		mx6/framebuffer/example1/Makefile
		mx6/framebuffer/example2/Makefile])

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

