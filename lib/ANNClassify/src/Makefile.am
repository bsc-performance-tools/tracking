lib_LTLIBRARIES=libANNClassify.la

libANNClassify_la_SOURCES=\
	libANNClassify.cpp\
	libANNClassify.h\
	ANNClassify.cpp

libANNClassify_la_CXXFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/libANN

libANNClassify_la_LIBADD=\
	$(top_srcdir)/libANN/libANN.la

kk:
	echo $(top_srcdir)/libANN/libANN.la
