include $(top_srcdir)/PATHS

SUBDIRS = scripts gui

bin_PROGRAMS = tracking.bin

tracking_bin_SOURCES = \
  AppMain.cpp                                 \
  CorrelationMatrix.cpp CorrelationMatrix.h   \
  Links.cpp Links.h                           \
  ClassifyTool.cpp ClassifyTool.h             \
  ClusteringState.cpp ClusteringState.h       \
  CrossClassifier.cpp CrossClassifier.h       \
  CallersTracker.cpp CallersTracker.h         \
  SequenceTracker.cpp SequenceTracker.h       \
  Tracking.cpp Tracking.h                     \
  TraceReconstructor.cpp TraceReconstructor.h \
  Utils.cpp Utils.h                           \
  ClusterIDs.h
  
tracking_bin_CXXFLAGS  = $(BOOST_CPPFLAGS) -I$(ANN_DIR)/include -I$(SCORE_DIR)/include $(PARAVER_KERNEL_INCLUDES)
tracking_bin_LDFLAGS   = $(PARAVER_KERNEL_LDFLAGS) $(PARAVER_KERNEL_LIBS)
tracking_bin_LDADD     = $(ANN_DIR)/src/libANNClassify.la $(SCORE_DIR)/libSequenceScore.la

if HAVE_PARAVER
#tracking_bin_SOURCES  += CallersTracker.cpp
#tracking_bin_CXXFLAGS += $(PARAVER_KERNEL_INCLUDES)
#tracking_bin_LDFLAGS  += $(PARAVER_KERNEL_LDFLAGS) $(PARAVER_KERNEL_LIBS)
endif
