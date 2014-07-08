#ifndef __TRACKER_H__
#define __TRACKER_H__

#include "Links.h"
#include "CorrelationMatrix.h"

class Tracker 
{
  public:
    Tracker();
    ~Tracker();

    void Track();
    virtual void RunTracker() = 0;

    ObjectLinks * getLinks(ClusterID_t cluster_id, double min_likeliness = 0.0);

    DoubleLinks * getLinks(double min_likeliness = 0.0);

    CorrelationMatrix *WhoIsWho;
};

#endif /* __TRACKER_H__ */

