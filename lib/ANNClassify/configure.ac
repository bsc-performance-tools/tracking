SVN_REVISION="$Rev: 20 $"

AC_INIT([libANNClassify], [0.03], [tools@bsc.es])
AC_CONFIG_AUX_DIR([.])
AC_CANONICAL_SYSTEM

AC_CONFIG_HEADER(config.h)

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_PROG_CC(gcc cc xlc)
AC_PROG_CXX(g++ CC xlC)
AC_PROG_CPP

AM_PROG_CC_C_O

AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE
AC_PROG_MKDIR_P

dnl =======================================================================
dnl Final Makefile generation
dnl =======================================================================

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  libANN/Makefile
  src/Makefile
])

AC_OUTPUT
