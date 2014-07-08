#include "SPMDTracker.h"

SPMDTracker::SPMDTracker(ClustersAlignment *AlignmentsFromTrace1)
{
  Alignments = AlignmentsFromTrace1;
}

SPMDTracker::~SPMDTracker()
{
  Alignments = NULL;
}

void SPMDTracker::RunTracker()
{
  int ClustersInSequence = Alignments->NumberOfClusters();

  WhoIsWho = new CorrelationMatrix(ClustersInSequence, ClustersInSequence);

  for (int i=1; i<=ClustersInSequence; i++)
  {
    SimultaneousClusters_t Simultaneous; /* map[cluster] = probability of appearing together */

    int NumberOfAppearances = Alignments->Scoring()->FindSimultaneousClusters( i, Simultaneous );

    /* DEBUG 
    cout << "[DEBUG] Simultaneity for cluster " << i << " = "; */
    SimultaneousClusters_t::iterator it;
    for (it = Simultaneous.begin(); it != Simultaneous.end(); it ++)
    {
      int SeenCount = it->second;
      /* DEBUG 
      cout << it->first << " (" << (SeenCount * 100)/NumberOfAppearances << "%) "; */
      WhoIsWho->SetStats(i, it->first, SeenCount, (SeenCount * 100)/NumberOfAppearances);
    }
    /* DEBUG
    cout << endl; */ 
  }
}

