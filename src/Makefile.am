
SUBDIRS = scripts

bin_PROGRAMS = tracking.bin test

tracking_bin_SOURCES = AppMain.cpp Tracking.cpp Tracking.h ClusteringState.cpp ClusteringState.h ClassifyTool.cpp ClassifyTool.h ClusterCorrelationMatrix.cpp ClusterCorrelationMatrix.h CrossClassifier.cpp CrossClassifier.h CallersTracker.cpp CallersTracker.h SequenceTracker.cpp SequenceTracker.h Utils.cpp Utils.h Correlation.cpp Correlation.h TraceReconstructor.cpp TraceReconstructor.h
tracking_bin_CXXFLAGS = -I$(CLUSTERING_INCLUDES) -I$(ANN_INCLUDES) $(PARAVER_KERNEL_INCLUDES)
tracking_bin_LDFLAGS = -L$(CLUSTERING_LIBSDIR) $(CLUSTERING_LIBS) $(SEQUENCE_LIBS) -L$(ANN_LIBSDIR) $(ANN_LIBS) $(PARAVER_KERNEL_LDFLAGS) $(PARAVER_KERNEL_LIBS)


test_SOURCES = test.cpp CallersTracker.cpp CallersTracker.h ClusterCorrelationMatrix.cpp ClusterCorrelationMatrix.h Utils.cpp Utils.h SequenceTracker.cpp SequenceTracker.h Correlation.cpp Correlation.h
test_CXXFLAGS = $(PARAVER_KERNEL_INCLUDES)
test_LDFLAGS = $(PARAVER_KERNEL_LDFLAGS) $(PARAVER_KERNEL_LIBS) -L$(CLUSTERING_LIBSDIR) $(SEQUENCE_LIBS)

