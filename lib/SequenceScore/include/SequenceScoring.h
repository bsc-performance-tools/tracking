/*******************************************************************************
 *  $RCSfile: SequenceScoring.h,v $
 *
 *  Last modifier: $Author: jgonzale $
 *  Last check in: $Date: 2009/02/27 10:42:21 $
 *  Revision:      $Revision: 1.2 $
 *
 *  Copyright (c) 2006  CEPBA Tools
 *  jgonzale@cepba.upc.edu
 *
 *  Description: class which performs cluster sequence scoring
 *
 ******************************************************************************/

#ifndef SEQUENCESCORING_H
#define SEQUENCESCORING_H

#include "Error.h"
#include "types.h"

#include <vector>
using std::vector;

#include <fstream>
using std::ifstream;

#include <map>
using std::map;

#include <set>
using std::set;

using std::pair;

typedef map<INT32, INT32> TClusterScoreMap;
typedef vector<TClusterScoreMap> TSequence;

class SequenceScoring: public Error
{
  private:
    
    string   SequenceFileName;
    ifstream SequenceFileStream;
  
    vector< vector<INT32> > ClusterSequences;
#if defined OLD_TIME_SEQ
    vector< set<INT32> >        TimeSequence;
#else
    TSequence  TimeSequencePercentage;
#endif

    vector<double>          PercentageDurations;
    INT32                   DifferentClusters;
    map<char,INT32>         AminoacidTranslation;

  
  public:
    SequenceScoring(string SequenceFileName);
  
    bool LoadClustersInformation(string ClustersInfoFileName);

    bool GetScores(vector<double>& Scores, vector<INT32>& Appareances);
    
    void GetPercentageDurations(vector<double>& PercentageDurations);
  
    bool GetGlobalScore(vector<double>& Scores,
                        double&         GlobalScore);

	void CalcTimeSequence();

	void FindSimultaneousClusters (INT32 ClusterID, set<INT32> & SimultaneousClusters);
    int FindSimultaneousClusters (INT32 ClusterID, map<INT32, int> & SimultaneousClusters);

    void TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatching);
    void TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, vector< pair<TSequence, TSequence> > &SubsequenceCombos);

    int  NextClusterPositionInSequence(SequenceScoring *SS, int ClusterID, int StartingIndex);
    //void GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, vector<INT32> &Subsequence);
    void GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, TSequence &Subsequence);
    //void FindAllSubsequences(SequenceScoring *SS, int FromCluster, int ToCluster, vector< vector<INT32> > &SubsequencesList);
    void FindAllSubsequences(SequenceScoring *SS, int FromCluster, int ToCluster, vector< TSequence > &SubsequencesList);
    void PrintSequence(TSequence & Sequence);
    void PrintClustersScore(TClusterScoreMap & Clusters);

    void GetMatchingsBetweenSequences(TSequence &Seq1, TSequence &Seq2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings);
    void PrintMatchingsBetweenSequences(map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings);


  private:
    void LoadAminoacidTable(void);
  
    bool LoadSequences(void);
    bool LoadFASTASequences(void);
  
    bool CheckSequences(INT32& SequencesLength);
  
    bool ColumnClustersInfo(ifstream& ClustersInfoStream);
  
    bool GetClustersDuration(ifstream&       ClustersInfoStream,
                             vector<UINT64>& ClustersDuration);
  
    bool ParseColumnLine(string Line, vector<UINT64>& ClustersDuration);
  
    bool ParseRowLine(string Line, vector<UINT64>& ClustersDuration);

    void SplitDiscriminableByPct(TClusterScoreMap m1, TClusterScoreMap m2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings);
    bool Discriminable(TClusterScoreMap &m);
};

#endif /* SEQUENCESCORING_H */
