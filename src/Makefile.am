include $(top_srcdir)/PATHS

SUBDIRS = scripts gui

bin_PROGRAMS = tracking.bin

tracking_bin_SOURCES = \
  AppMain.cpp                                 \
  ClustersInfo.cpp ClustersInfo.h             \
  CorrelationMatrix.cpp CorrelationMatrix.h   \
  CallersHistogram.cpp CallersHistogram.h     \
  ClassifyTool.cpp ClassifyTool.h             \
  ClusteringState.cpp ClusteringState.h       \
  ClustersAlignment.cpp ClustersAlignment.h   \
  Knapsack.cpp Knapsack.h                     \
  Tracker.cpp Tracker.h                       \
  DensityTracker.cpp DensityTracker.h         \
  DistanceTracker.cpp DistanceTracker.h       \
  CallersTracker.cpp CallersTracker.h         \
  SequenceTracker.cpp SequenceTracker.h       \
  SPMDTracker.cpp SPMDTracker.h     \
  Tracking.cpp Tracking.h                     \
  Links.cpp Links.h                           \
  TraceReconstructor.cpp TraceReconstructor.h \
  Utils.cpp Utils.h                           \
  ClusterIDs.h 
  
tracking_bin_CXXFLAGS  = $(BOOST_CPPFLAGS) -I$(ANN_DIR)/include -I$(SCORE_DIR)/include $(PARAVER_KERNEL_INCLUDES)
tracking_bin_LDFLAGS   = $(PARAVER_KERNEL_LDFLAGS) $(PARAVER_KERNEL_LIBS) $(PARAVER_KERNEL_RPATH) -R $(prefix)/lib
tracking_bin_LDADD     = $(ANN_DIR)/src/libANNClassify.la $(SCORE_DIR)/libSequenceScore.la

