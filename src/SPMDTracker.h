#ifndef __SPMDINESS_TRACKER_H__
#define __SPMDINESS_TRACKER_H__

#include "ClustersAlignment.h"
#include "Tracker.h"

class SPMDTracker : public Tracker
{
  public:
    SPMDTracker(ClustersAlignment *AlignmentsFromTrace1);
    ~SPMDTracker();

    void RunTracker();

  private:
    ClustersAlignment *Alignments;
};

#endif /* __SPMDINESS_TRACKER_H__ */
