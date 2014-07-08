#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/spirit/include/classic_rule.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_refactoring.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/tuple/tuple.hpp>
#include "DensityTracker.h"
#include "Knapsack.h"

using namespace std;
using namespace boost::spirit;
using namespace boost::spirit::classic;

DensityTracker::DensityTracker( ClustersInfo *clusters_info_1, ClustersInfo *clusters_info_2 )
{
  ClustersInfoData1 = clusters_info_1;
  ClustersInfoData2 = clusters_info_2;
  NumClusters1      = ClustersInfoData1->GetNumClusters(); 
  NumClusters2      = ClustersInfoData2->GetNumClusters(); 

  WhoIsWho = new CorrelationMatrix(NumClusters1, NumClusters2);
}

DensityTracker::~DensityTracker( )
{
  ClustersInfoData1 = NULL;
  ClustersInfoData2 = NULL;
  NumClusters1 = 0;
  NumClusters2 = 0;
}

void DensityTracker::RunTracker()
{
  map<ClusterID_t, int> CandidateClusters;

  for (int i=1; i<=NumClusters2; i++)
  {
    CandidateClusters[i] = ClustersInfoData2->GetDensity(i);
  }
  for (int i=1; i<=NumClusters1; i++)
  {
    /* DEBUG -- Cluster to correlate 
    cerr << "[DEBUG] DensityTracker:: Cluster " << i << " --> "; */

    set<ClusterID_t> PickedClusters;

    Knapsack( ClustersInfoData1->GetDensity(i), CandidateClusters, PickedClusters );

    set<ClusterID_t>::iterator it;
    for (it = PickedClusters.begin(); it != PickedClusters.end(); ++ it)
    {
      /* DEBUG -- corresponding clusters that fit 
      cerr << " " << *it; */
      CandidateClusters.erase( CandidateClusters.find( *it ) );
      WhoIsWho->Hit(i, *it);
    }
    /* DEBUG -- 
    cerr << endl; */
    if (CandidateClusters.size () <= 0) break;
  }
  WhoIsWho->ComputePcts();
}

