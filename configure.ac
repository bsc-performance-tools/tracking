
# Process this file with autoconf to produce a configure script.

#########################
#    Initializations    #
#########################

# Initialize autoconf & define package name, version and bug-report address 
AC_INIT(Tracking Tool, 2.6, tools@bsc.es)

# GNU Autotools intermediate files are stored in the following directory
AC_CONFIG_AUX_DIR(config)

# Loads some shell variables like host_cpu and host_os, to get the host information 
AC_CANONICAL_SYSTEM

# Initialize automake
AM_INIT_AUTOMAKE
AC_PROG_MKDIR_P

# Specify the output configuration header file
AC_CONFIG_HEADER(config.h)

# Search for libtool support
AC_LIBTOOL_DLOPEN
AC_ENABLE_SHARED
AC_PROG_LIBTOOL

#############################
#    Checks for programs    #
#############################

AC_PROG_CC(gcc cc xlc)
AC_PROG_CXX(g++ CC xlC)
AC_PROG_CPP
AC_PROG_INSTALL
CFLAGS="$CFLAGS -Wno-deprecated"
CXXFLAGS="$CXXFLAGS -Wno-deprecated"

# Automake 1.10 reports a problem if this is not defined
AM_PROG_CC_C_O

##############################
#    Checks for libraries    #
##############################
BOOST_REQUIRE(1.36)
BOOST_SERIALIZATION
AX_PROG_CLUSTERING
AX_PROG_PARAVER

#############################
#    Checks for headers     #
#############################

###########################
#    Checks for types     #
###########################

AC_C_CONST
AC_TYPE_SIZE_T

AC_CHECK_TYPES([long, long long, char, int, short int])
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(short int)
AC_CHECK_TYPES([int64_t, uint64_t, int8_t, uint8_t, int32_t, uint32_t, int16_t, uint16_t])

AC_CONFIG_FILES([Makefile lib/Makefile lib/SequenceScore/Makefile src/Makefile src/scripts/Makefile src/gui/Makefile])

AC_CONFIG_SUBDIRS([lib/ANNClassify])

AC_OUTPUT
