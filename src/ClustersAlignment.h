#ifndef __CLUSTERS_ALIGNMENT_H__
#define __CLUSTERS_ALIGNMENT_H__

#include <string>
#include <vector>

#include "ClusterIDs.h"
#include "SequenceScoring.h"

using std::string;
using std::vector;

class ClustersAlignment 
{
  public:
    ClustersAlignment(string AlignmentFile, string ClustersInfoFile);
    ~ClustersAlignment();

    bool   Exists( void );;
    int    NumberOfClusters( void );
    double ClusterScore( ClusterID_t cluster_id );
    double GlobalScore( void );
    void   Print( void );

    SequenceScoring *Scoring();

  private:
    SequenceScoring *SS;
    double           ScoreGlobal;
    vector<double>   ScorePerCluster;
};

#endif /* __CLUSTERS_ALIGNMENT_H__ */
