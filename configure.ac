#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(vvssh vvinit vvinput, 0.0.1, [wlsdhzy@163.com])
AM_INIT_AUTOMAKE(vvssh vvinit vvinput,0.0.1)
AC_CONFIG_SRCDIR([src/vvinput.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])
# FIXME: Replace `main' with a function in `-lssh2':
AC_CHECK_LIB([ssh2], [main])
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netinet/in.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([select socket])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 src/Makefile])
AC_OUTPUT
