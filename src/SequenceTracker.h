#ifndef __SEQUENCE_TRACKER_H__
#define __SEQUENCE_TRACKER_H__

#include <string>
using std::string;
#include "SequenceScoring.h"
#include "ClusterIDs.h"
#include "ClusterCorrelationMatrix.h"
#include "Correlation.h"

class SequenceTracker
{
public:
  SequenceScoring *SS;

  SequenceTracker(string AlignFile, string CINFOFile);
  ~SequenceTracker();

  bool   isAvailable();
  double getGlobalScore();
  double getClusterScore(CID cluster_id);
  int    getNumberOfClusters();
  ClusterCorrelationMatrix * getClustersSimultaneity();
  ClusterCorrelationMatrix * CorrelateWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2);
  TwoWayCorrelation * PairWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2);
  TwoWayCorrelation * PairWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2, CID LastClusterTrace1, CID LastClusterTrace2);

  void   dump();

private:
  double         GlobalScore;
  vector<double> Scores;

  int FilterSubsequence(TSequence Seq, CID LastCluster, TSequence &FilteredSeq);

};

#endif /* __SEQUENCE_TRACKER_H__ */
