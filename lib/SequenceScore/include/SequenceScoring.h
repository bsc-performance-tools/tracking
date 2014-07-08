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

#ifndef __SEQUENCE_SCORING_H__
#define __SEQUENCE_SCORING_H__

#include <fstream>
#include <map>
#include <set>
#include <vector>
#include "types.h"
#include "ClusterIDs.h"
#include "Error.h"
#include "Links.h"

using std::ifstream;
using std::map;
using std::pair;
using std::set;
using std::vector;

#define MIN_OCCURRENCE_PCT 0

typedef map<INT32, INT32>                                   ClusterScore_t;
typedef vector<ClusterScore_t>                              Sequence_t;
typedef INT32                                               Occurrences_t;
typedef map< pair<ClusterID_t, ClusterID_t>, Occurrences_t> Matchings_t;
typedef map< ClusterID_t, Occurrences_t >                   SimultaneousClusters_t;

class SequenceScoring: public Error
{
  private:
    string   SequenceFileName;
    ifstream SequenceFileStream;
  
    vector< vector<INT32> > ClusterSequences;
    Sequence_t              GlobalSequence;
 
    vector<double>          PercentageDurations;
    INT32                   DifferentClusters;
    map<char,INT32>         AminoacidTranslation;

  public:
    SequenceScoring(string SequenceFileName);
  
    bool LoadClustersInformation(string ClustersInfoFileName);

    bool GetScores(vector<double>& Scores, vector<INT32>& Appearances);
    
    void GetPercentageDurations(vector<double>& PercentageDurations);
  
    bool GetGlobalScore(vector<double>& Scores,
                        double&         GlobalScore);

    int FindSimultaneousClusters (ClusterID_t ClusterID, SimultaneousClusters_t &Simultaneous);

    void TimeCorrelation(DoubleLinks *UnivocalLinks, SequenceScoring *SS2, Matchings_t &SequenceMatching);

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

    void ComputeGlobalSequence();

    void GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, Sequence_t &Subsequence);

    void FindAllSubsequences(SequenceScoring *SS, int FromCluster, int ToCluster, vector<Sequence_t> &SubsequencesList);

    void PrintSequence(Sequence_t & Sequence);

    void PrintClustersScore(ClusterScore_t & Clusters);

    void GetMatchingsBetweenSequences(Sequence_t  &Seq1, 
                                      Sequence_t  &Seq2, 
                                      Matchings_t &SequenceMatchings);

    void PrintMatchings(Matchings_t &SequenceMatchings);
};

#endif /* __SEQUENCE_SCORING_H__ */
