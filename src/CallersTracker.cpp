#include <iostream>
#include "CallersTracker.h"
#include "ClusterIDs.h"

using std::cout;
using std::cerr;
using std::endl;

CallersTracker::CallersTracker(CallersHistogram *histogram1, CallersHistogram *histogram2)
{
  H1 = histogram1;
  H2 = histogram2;
}

CallersTracker::~CallersTracker()
{
  H1 = NULL;
  H2 = NULL;
}

void CallersTracker::RunTracker()
{
  WhoIsWho = new CorrelationMatrix(H1->size(), H2->size());
  
  for (ClusterID_t CurrentCluster = 1; CurrentCluster <= H1->size(); CurrentCluster++) 
  {
    vector<Caller_t> CurrentClusterCallers;

    if (!H1->hasCallers(CurrentCluster)) continue;

    H1->getCallersForCluster(CurrentCluster, CurrentClusterCallers);

    /* DEBUG 
    cout << "[DEBUG] CallersTracker::Track: " << CurrentCluster << " --> "; */

    for (int i=0; i<CurrentClusterCallers.size(); i++)
    {
      Caller_t    CurrentCaller = CurrentClusterCallers[i];
      vector<ClusterID_t> CorrespondingClusters;

      H2->getClustersWithCaller(CurrentCaller.CallerName, CorrespondingClusters);

      if (CorrespondingClusters.size() > 0)
      {
        for (int j=0; j<CorrespondingClusters.size(); j++)
        {
          Caller_t CorrespondingCaller;
          double   MinPercentage = 0;

          H2->getStatsForClusterAndCaller(CorrespondingClusters[j], CurrentCaller.CallerName, CorrespondingCaller);
          MinPercentage = (CurrentCaller.Pct < CorrespondingCaller.Pct ? CurrentCaller.Pct : CorrespondingCaller.Pct);

          /* DEBUG 
          cout << CorrespondingClusters[j] << " (" << MinPercentage << ") ";
          cout.flush(); */

          WhoIsWho->RaisePct(CurrentCluster, CorrespondingClusters[j], MinPercentage);
        }
      }
    }
    /* DEBUG 
    cout << endl; */
  }
}

