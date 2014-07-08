#include <iostream>
#include <libgen.h>
#include <stdlib.h>
#include "CorrelationMatrix.h"
#include "SequenceTracker.h"
#include "Utils.h"

using std::cout;
using std::cerr;
using std::endl;

SequenceTracker::SequenceTracker(ClustersAlignment *alignments_frame_1, ClustersAlignment *alignments_frame_2, DoubleLinks *univocal_links)
{
  AlignmentsFrame1 = alignments_frame_1;
  AlignmentsFrame2 = alignments_frame_2;
  UnivocalLinks    = univocal_links;
}

SequenceTracker::~SequenceTracker()
{
  AlignmentsFrame1 = NULL;
  AlignmentsFrame2 = NULL;
  UnivocalLinks    = NULL;
}

void SequenceTracker::RunTracker()
{
  //map<TClusterScoreMap, TClusterScoreMap> SequenceMatchings;
  map< pair<INT32, INT32>, INT32 > SequenceMatchings;
  map< pair<INT32, INT32>, INT32 >::iterator it;

  AlignmentsFrame1->Scoring()->TimeCorrelation(UnivocalLinks, AlignmentsFrame2->Scoring(), SequenceMatchings);

  WhoIsWho = new CorrelationMatrix(
    AlignmentsFrame1->NumberOfClusters(), 
    AlignmentsFrame2->NumberOfClusters()
  );

  for (it = SequenceMatchings.begin(); it != SequenceMatchings.end(); it++)
  {
    ClusterID_t left_cluster  = it->first.first;
    ClusterID_t right_cluster = it->first.first;
    int occurrences   = it->second;

    for (int i=0; i<occurrences; i++)
    {
      WhoIsWho->Hit(left_cluster, right_cluster);
    }
  }
  WhoIsWho->ComputePcts();
}
