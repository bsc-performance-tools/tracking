#ifndef __SEQUENCE_TRACKER_H__
#define __SEQUENCE_TRACKER_H__

#include <map>
#include "ClustersAlignment.h"
#include "ClusterIDs.h"
#include "Links.h"
#include "Tracker.h"

using std::map;

class SequenceTracker : public Tracker
{
  public:
    SequenceTracker(ClustersAlignment *alignments_frame_1, ClustersAlignment *alignments_frame_2, DoubleLinks *univocal_links);
    ~SequenceTracker();

    void RunTracker();

  private:
    ClustersAlignment *AlignmentsFrame1;
    ClustersAlignment *AlignmentsFrame2;
    DoubleLinks       *UnivocalLinks;
};

#endif /* __SEQUENCE_TRACKER_H__ */
