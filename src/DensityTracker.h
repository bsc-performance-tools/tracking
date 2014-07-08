#ifndef __DENSITY_TRACKER_H__
#define __DENSITY_TRACKER_H__

#include <string>
#include <vector>
#include "ClustersInfo.h"
#include "Tracker.h"

using std::string;
using std::vector;

class DensityTracker : public Tracker
{
  public:
    DensityTracker(ClustersInfo *clusters_info_1, ClustersInfo *clusters_info_2);
    ~DensityTracker();

    void RunTracker();

  private:
    int NumClusters1;
    int NumClusters2;
    ClustersInfo *ClustersInfoData1;
    ClustersInfo *ClustersInfoData2;
};

#endif /* __DENSITY_TRACKER_H__ */

