#ifndef __DISTANCE_TRACKER_H__
#define __DISTANCE_TRACKER_H__

#include <string>
#include <vector>
#include "ClassifyTool.h"
#include "ClusteringState.h"
#include "Tracker.h"

using std::string;
using std::vector;

class DistanceTracker : public Tracker
{
  public:
    DistanceTracker(int trace1, string CSV1, int trace2, string CSV2, double Epsilon = 1.0);
    ~DistanceTracker();

    void RunTracker();

  private:
    ClassifyTool    *CT;
    ClusteringState *CS1, *CS2;
    string           CrossClassifierOutput;
};

#endif /* __DISTANCE_TRACKER_H__ */
