
# Process this file with autoconf to produce a configure script.

#########################
#    Initializations    #
#########################

m4_define([tracking_major], [2])
m4_define([tracking_minor], [6])
m4_define([tracking_micro], [10])

m4_define([tracking_version], [tracking_major.tracking_minor.tracking_micro])

# Initialize autoconf & define package name, version and bug-report address 
AC_INIT(Tracking, [tracking_version], tools@bsc.es)

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

AX_BOOST_BASE(1.36)
AX_BOOST_SERIALIZATION
BOOST_RPATH="-R ${BOOST_LDFLAGS#-L}"
AC_SUBST(BOOST_RPATH)

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

AC_CONFIG_FILES([Makefile lib/Makefile lib/SequenceScore/Makefile doc/Makefile src/Makefile src/scripts/Makefile src/gui/Makefile])

AC_CONFIG_LINKS([lib/SequenceScore/include/SequenceScoring.h:lib/SequenceScore/include/SequenceScoring.h] \
                [lib/SequenceScore/include/Error.h:lib/SequenceScore/include/Error.h] \
                [lib/ANNClassify/include/ANNClassify/ANNClassify.h:lib/ANNClassify/include/ANNClassify/ANNClassify.h])


AC_CONFIG_SUBDIRS([lib/ANNClassify])

AC_OUTPUT
