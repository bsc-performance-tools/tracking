#include "Tracker.h"

Tracker::Tracker()
{
  WhoIsWho = NULL;
}

Tracker::~Tracker()
{
  if (WhoIsWho != NULL)
  {
    delete WhoIsWho;
  }
}

void Tracker::Track()
{
  RunTracker();

  WhoIsWho->Print();
  WhoIsWho->Stats();
}

ObjectLinks * Tracker::getLinks( ClusterID_t cluster_id, double min_likeliness )
{
  return WhoIsWho->getLinks( cluster_id, min_likeliness );
}

DoubleLinks * Tracker::getLinks( double min_likeliness )
{
  return WhoIsWho->getCorrelations( min_likeliness );
}
