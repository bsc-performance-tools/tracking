/*******************************************************************************
 *  $RCSfile: SequenceScoring.cpp,v $
 *
 *  Last modifier: $Author: jgonzale $
 *  Last check in: $Date: 2009/02/27 10:42:21 $
 *  Revision:      $Revision: 1.3 $
 *
 *  Copyright (c) 2006  CEPBA Tools
 *  jgonzale@cepba.upc.edu
 *
 *  Description: class which performs cluster sequence scoring
 *
 ******************************************************************************/

#include <string.h>

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include "SequenceScoring.h"

#include <errno.h>

/* #define BOOST_SPIRIT_DEBUG */

#include <boost/spirit/include/classic_rule.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_refactoring.hpp>
#include <boost/spirit/include/classic_lists.hpp>
using namespace boost::spirit;
using namespace boost::spirit::classic;

/***************************************************************************
 * 'SequenceScoring' constructor
 **************************************************************************/

SequenceScoring::SequenceScoring(string SequenceFileName)
{
  this->SequenceFileName = SequenceFileName;

  SequenceFileStream.open(SequenceFileName.c_str(), ifstream::in);
  
  if (!SequenceFileStream)
  {
    string ErrorMessage = "Unable to open sequence file '"+SequenceFileName+"'";
    
    SetError(true);
    SetErrorMessage(ErrorMessage, strerror(errno));
    return;
  }

  
  DifferentClusters = 0;
  
  LoadAminoacidTable();
  
  if (!LoadSequences())
    return;
  
  CalcTimeSequence();

  return;
}

/***************************************************************************
 * 'LoadClustersInformation' PUBLIC
 **************************************************************************/

bool
SequenceScoring::LoadClustersInformation(string ClustersInfoFileName)
{
  ifstream       ClustersInfoStream;
  vector<UINT64> ClustersDuration;
  // vector<double> PercentageDuration;
  UINT64         TotalDuration = 0;

  ClustersInfoStream.open(ClustersInfoFileName.c_str(), ios_base::in);

  if (!ClustersInfoStream)
  {
    string ErrorMessage = "Unable to open clusters information file '"+
                          ClustersInfoFileName+"'";
    
    SetError(true);
    SetErrorMessage(ErrorMessage, strerror(errno));
    return false;
  }

  if (!GetClustersDuration(ClustersInfoStream, ClustersDuration))
  {
    return false;
  }

  for (INT32 i = 0; i < ClustersDuration.size(); i++)
  {
    TotalDuration += ClustersDuration[i];
  }

  for (INT32 i = 0; i < ClustersDuration.size(); i++)
  {
    PercentageDurations.push_back(1.0*ClustersDuration[i]/TotalDuration);
  }

  if (PercentageDurations.size() != DifferentClusters)
  {
    return false;
  }

  return true;
}

/***************************************************************************
 * 'GetScores' PUBLIC
 **************************************************************************/

bool
SequenceScoring::GetScores(vector<double>& Scores, vector<INT32>& Appareances)
{
  INT32 SequencesLength;
  
  Appareances = vector<INT32> (DifferentClusters, 0);
  Scores      = vector<double> (DifferentClusters, 0.0);
  
  if (!CheckSequences(SequencesLength))
    return false;
  
//cout << "GetScores: SequencesLength=" << SequencesLength << endl;

  for (INT32 i = 0; i < SequencesLength; i++)
  {
    vector<INT32> Hits (DifferentClusters, 0);
    
    /* iteration over # of sequences */
    for (INT32 j = 0; j < ClusterSequences.size(); j++)
    {
      
      Hits[ClusterSequences[j][i]]++;
    }
    
    /* iteration over # of clusters */
    for (INT32 j = 0; j < DifferentClusters; j++)
    {
      if (Hits[j] != 0)
      {
        Appareances[j]++;
        Scores[j] += (1.0*Hits[j]/ClusterSequences.size());
      }
    }
  }
  
//cout << "GetScores: DifferentClusters=" << DifferentClusters << endl;
  for (INT32 i = 0; i < DifferentClusters; i++)
  {
    Scores[i] = Scores[i]/Appareances[i];
  }
  
  return true;
}

/***************************************************************************
 * 'GetPercentageDurations' PUBLIC
 **************************************************************************/

void
SequenceScoring::GetPercentageDurations(vector<double>& PercentageDurations)
{
  PercentageDurations = this->PercentageDurations;
}

/***************************************************************************
 * 'GetGlobalScore' PUBLIC
 **************************************************************************/

bool
SequenceScoring::GetGlobalScore(vector<double>& Scores,
                                double&         GlobalScore)
{
  GlobalScore = 0.0;

  if (Scores.size() == PercentageDurations.size())
  {
    for (INT32 i = 0; i < Scores.size(); i++)
    {
      GlobalScore += (Scores[i]*PercentageDurations[i]);
    }
  }
  else if (Scores.size() <= PercentageDurations.size())
  {
    for (INT32 i = 0; i < Scores.size(); i++)
    {
      GlobalScore += (Scores[i]*PercentageDurations[i-1]);
    }
  }
  else
  {
    ostringstream ErrorMessage;
    
    ErrorMessage << "Clusters in sequence = " << Scores.size();
    ErrorMessage << " Clusters in info file = " << PercentageDurations.size();
    
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }
  
  return true;
}


void 
SequenceScoring::CalcTimeSequence()
{
    unsigned int SequenceLength;

    if ( ClusterSequences.size() <= 0 ) return;

    /* All sequences have the same length */
    SequenceLength = ClusterSequences[0].size();

    /* Iterate the sequence */
    for (unsigned int i = 0; i < SequenceLength; i++)
    {
        vector< vector<INT32> > :: iterator it;

#if defined OLD_TIME_SEQ
        TimeSequence.push_back( set<INT32>() );
#else
        TimeSequencePercentage.push_back( map<INT32,INT32>() );
#endif

        /* Iterate the tasks */
        for (it = ClusterSequences.begin(); it != ClusterSequences.end(); ++ it)
        {
            INT32 CurrentClusterValue;

            CurrentClusterValue = (*it)[i];

            if (CurrentClusterValue != 0)
            {
#if defined OLD_TIME_SEQ
                TimeSequence[i].insert(CurrentClusterValue);
#else
                /* Store as value the number of times this cluster appears in the i-position of the sequence */
                if (TimeSequencePercentage[i].find(CurrentClusterValue) == TimeSequencePercentage[i].end())
                {
                   TimeSequencePercentage[i][CurrentClusterValue] = 1;
                }
                else
				{
                   TimeSequencePercentage[i][CurrentClusterValue] = TimeSequencePercentage[i][CurrentClusterValue] + 1;
				}
#endif
            }
        }
    }
#if defined OLD_TIME_SEQ
	/* DEBUG */
    cout << "Time Sequence (length: " << TimeSequence.size() << ")" << endl;
    for (int i = 0; i < TimeSequence.size(); i++)
    {
        int set_size = TimeSequence[i].size();

        if (set_size > 1) cout << "[";

        set<INT32>::iterator it;
        int count = 0;
        for (it = TimeSequence[i].begin(); it != TimeSequence[i].end(); ++ it)
        {
            cout << *it;
            if (count < TimeSequence[i].size() - 1) cout << ",";
            count ++;
        }
        if (set_size > 1) cout << "]";
        cout << " ";
    }
    cout << endl;
#else
    /* DEBUG */
    cout << "Time Sequence (length: " << TimeSequencePercentage.size() << ")" << endl;
    for (int i = 0; i < TimeSequencePercentage.size(); i++)
    {
       map<INT32, INT32>::iterator it;

       /* Store for each cluster (key) the percentage of occurrence in the sequence (value) */
       for (it=TimeSequencePercentage[i].begin(); it!=TimeSequencePercentage[i].end(); ++it)
       {
          it->second = (it->second * 100) / ClusterSequences.size();
       }

       int set_size = TimeSequencePercentage[i].size();
       if (set_size > 1) cout << "[";
       int count = 0;
       for (it = TimeSequencePercentage[i].begin(); it != TimeSequencePercentage[i].end(); ++ it)
       {
           cout << it->first << "(" << it->second << "%)";
           if (count < TimeSequencePercentage[i].size() - 1) cout << ",";
           count ++;
       }
       if (set_size > 1) cout << "]";
       cout << " ";

    }
    cout << endl;
#endif
}


#if defined OLD_TIME_SEQ
void 
SequenceScoring::FindSimultaneousClusters (INT32 ClusterID, set<INT32> & SimultaneousClusters)
{
	for (int i = 0; i < TimeSequence.size(); i++)
	{
		if (TimeSequence[i].find(ClusterID) != TimeSequence[i].end())
		{
			set<INT32>::iterator it;

			for (it = TimeSequence[i].begin(); it != TimeSequence[i].end(); ++ it)
			{
				if (*it != ClusterID)
				{
					SimultaneousClusters.insert(*it);
				}
			}
		}
	}

#if 0
	/* DEBUG */
	cout << "Simultaneous clusters to " << ClusterID << ": ";
	set<INT32>::iterator it;
	for (it = SimultaneousClusters.begin(); it != SimultaneousClusters.end(); ++ it)
	{
		cout << *it << " ";
	}
	cout << endl;
#endif
}
#else

#define MIN_OCCURRENCE_PCT 0

void
SequenceScoring::FindSimultaneousClusters (INT32 ClusterID, set<INT32> & SimultaneousClusters)
{
    
    for (int i = 0; i < TimeSequencePercentage.size(); i++)
    {
        if (TimeSequencePercentage[i].find(ClusterID) != TimeSequencePercentage[i].end())
        {
            map<INT32, INT32>::iterator it;

            for (it = TimeSequencePercentage[i].begin(); it != TimeSequencePercentage[i].end(); ++ it)
            {
                if ((it->first != ClusterID) && (it->second > MIN_OCCURRENCE_PCT))
                {
                    SimultaneousClusters.insert(it->first);
                }
            }
        }
    }

#if 0
    /* DEBUG */
    cout << "Simultaneous clusters to " << ClusterID << ": ";
    set<INT32>::iterator it;
    for (it = SimultaneousClusters.begin(); it != SimultaneousClusters.end(); ++ it)
    {
        cout << *it << " ";
    }
    cout << endl;
#endif
}

int
SequenceScoring::FindSimultaneousClusters (INT32 ClusterID, map<INT32, int> & SimultaneousClusters)
{
    int NumberOfAppearances = 0;
    for (int i = 0; i < TimeSequencePercentage.size(); i++)
    {
        if (TimeSequencePercentage[i].find(ClusterID) != TimeSequencePercentage[i].end())
        {
            NumberOfAppearances ++;
            map<INT32, INT32>::iterator it;

            for (it = TimeSequencePercentage[i].begin(); it != TimeSequencePercentage[i].end(); ++ it)
            {
                if ((it->first != ClusterID) && (it->second > MIN_OCCURRENCE_PCT))
                {
                    if (SimultaneousClusters.find(it->first) == SimultaneousClusters.end()) 
                    {
                      SimultaneousClusters[ it->first ] = 0;
                    }
                    SimultaneousClusters[ it->first ] ++;
                }
            }
        }
    }
    return NumberOfAppearances;
}

#endif

/*
INT32 ClusterInSequence(int pos)
{
  map<INT32, INT32> ClusterSet = TimeSequencePercentage[pos];
  map<INT32, INT32>::iterator it;

  for (it=ClusterSet.begin(); it!=ClusterSet.end(); it++)
  {
    
  }
  return TimeSequencePercentage[pos];
}
*/

int SequenceScoring::NextClusterPositionInSequence(SequenceScoring *SS, int ClusterID, int StartingIndex)
{
  for (int i = StartingIndex; i < SS->TimeSequencePercentage.size(); i++)
  {
    if (SS->TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = SS->TimeSequencePercentage[i].begin()->first;
      if (ClusterSoloInSequence == ClusterID)
      { 
        return i;
      }
    }
  }
  return -1;
}

void SequenceScoring::GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, TSequence &Subsequence)
{
  for (int k=FromIndex+1; k<ToIndex; k++)
  {
    Subsequence.push_back( SS->TimeSequencePercentage[k] );
  }
}

void SequenceScoring::FindAllSubsequences(SequenceScoring *SS, int FromCluster, int ToCluster, vector< TSequence > &SubsequencesList)
{
  int idx1 = 0, idx2 = 0;

  idx1 = NextClusterPositionInSequence(SS, FromCluster, idx2);
  idx2 = NextClusterPositionInSequence(SS, ToCluster,   idx1+1);

  while ((idx1 != -1) && (idx2 != -1))
  {
    TSequence Subsequence;
    GetSubsequence (SS, idx1, idx2, Subsequence);
    if (Subsequence.size() > 0) 
    {
      SubsequencesList.push_back( Subsequence );
    }
    idx1 = NextClusterPositionInSequence(SS, FromCluster, idx2);
    idx2 = NextClusterPositionInSequence(SS, ToCluster,   idx1+1);
  }
}

#if 0
void SequenceScoring::GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, vector<INT32> &Subsequence)
{
  for (int k=FromIndex+1; k<ToIndex; k++)
  {
    if (SS->TimeSequencePercentage[k].size() == 1)
    {
      Subsequence.push_back( SS->TimeSequencePercentage[k].begin()->first );
    }
    else
    {
      Subsequence.push_back( -1 );
    }
  }
}

void SequenceScoring::FindAllSubsequences(SequenceScoring *SS, int FromCluster, int ToCluster, vector< vector<INT32> > &SubsequencesList)
{
  int idx1 = 0, idx2 = 0;

  idx1 = NextClusterPositionInSequence(SS, FromCluster, idx2);
  idx2 = NextClusterPositionInSequence(SS, ToCluster,   idx1+1);

  while ((idx1 != -1) && (idx2 != -1))
  {
    vector< INT32 > Subsequence;
    GetSubsequence (SS, idx1, idx2, Subsequence);
    if (Subsequence.size() > 0) 
    {
      SubsequencesList.push_back( Subsequence );
    }
    idx1 = NextClusterPositionInSequence(SS, FromCluster, idx2);
    idx2 = NextClusterPositionInSequence(SS, ToCluster,   idx1+1);
  }
}
#endif

void SequenceScoring::PrintSequence(TSequence &Sequence)
{
  for (int i=0; i<Sequence.size(); i++)
  {
    cout << i << ":";
    TClusterScoreMap ClustersInSequence = Sequence[i];
    PrintClustersScore( ClustersInSequence );
  }
  cout << endl;
}

void SequenceScoring::PrintClustersScore(TClusterScoreMap &Clusters)
{
  TClusterScoreMap::iterator it;

  cout << "[ ";
  for (it=Clusters.begin(); it!=Clusters.end(); it++)
  {
    cout << it->first << "(" << it->second << "%) ";
  }
  cout << "] ";
}

/* Returns all combinations of subsequences without any type of filtering.
 * This implementation is better as it only looks for subsequences between consecutive indexes in both sequences!
 */
void SequenceScoring::TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, vector< pair<TSequence, TSequence> > &SubsequenceCombos)
{
  /* UniqueCorrelations is a map of clusters uniquely correlated
     Cluster -> TurnsInto
           1 -> 1
           2 -> 3
           3 -> 2 */
  /* Get the inverse mapping */
  map<INT32, INT32> UniqueInverted;
  map<INT32, INT32>::iterator it;
  for (it = UniqueCorrelations.begin(); it != UniqueCorrelations.end(); ++it)
  {
    UniqueInverted[it->second] = it->first;
  }

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes, Indexes2;
  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = TimeSequencePercentage[i].begin()->first;
      if (UniqueCorrelations.find( ClusterSoloInSequence ) != UniqueCorrelations.end())
      {
        Indexes.push_back( i );
      }
    }
  }

  /* Locate in the second global time sequence where the (right) corresponding clusters appear with a score of 100% */
  for (int i = 0; i < SS2->TimeSequencePercentage.size(); i++)
  {
    if (SS2->TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = SS2->TimeSequencePercentage[i].begin()->first;
      if (UniqueInverted.find( ClusterSoloInSequence ) != UniqueInverted.end())
      {
        Indexes2.push_back( i );
      }
    }
  }

  /* DEBUG 
  cout << "Indexes -> ";
  for (int i = 0; i<Indexes.size(); i++)
  {
    cout << Indexes[i] << " ";
  }
  cout << endl;

  cout << "Indexes2 -> ";
  for (int i = 0; i<Indexes2.size(); i++)
  {
    cout << Indexes2[i] << " ";
  }
  cout << endl; */

  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1;

    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes[i];
    int To=Indexes[j];

    TSequence Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    int ClusterFrom       = TimeSequencePercentage[From].begin()->first;
    int ClusterTo         = TimeSequencePercentage[To].begin()->first;
    int CorrespondingFrom = UniqueCorrelations[ClusterFrom];
    int CorrespondingTo   = UniqueCorrelations[ClusterTo];

    /* DEBUG
    cout << "[DEBUG] Subsequence from " << ClusterFrom << " to " << ClusterTo << " in Sequence 1:" << endl << " * ";
    PrintSequence( Subsequence ); */

    /* Look for subsequences between 'CorrespondingFrom' and 'CorrespondingTo' clusters */
    vector< TSequence > SubsequencesList;

    for (int ii = 0; ii<Indexes2.size()-1; ii++)
    {
      int jj = ii+1;
      TSequence Subsequence2;

      int From2=Indexes2[ii];
      int To2=Indexes2[jj];
      if ((SS2->TimeSequencePercentage[From2].begin()->first == CorrespondingFrom) &&
          (SS2->TimeSequencePercentage[To2].begin()->first == CorrespondingTo))
      {
        SS2->GetSubsequence(SS2, From2, To2, Subsequence2);
        if (Subsequence2.size() <= 0) continue;
        SubsequencesList.push_back( Subsequence2 );
      }
    }
    /* DEBUG 
    cout << "[DEBUG] Found " << SubsequencesList.size() << " subsequences from corresponding clusters " << CorrespondingFrom << " and " << CorrespondingTo << " in Sequence 2:" << endl; */

    for (int k=0; k<SubsequencesList.size(); k++)
    {
      /* DEBUG 
      cout << " * Subsequence #" << k+1 << ": ";
      PrintSequence( SubsequencesList[k] ); */

      pair<TSequence, TSequence> p = make_pair(Subsequence, SubsequencesList[k]);
      SubsequenceCombos.push_back(p);
    }
  }
}

#if 0
/* This implementation returns all combinations of subsequences without any type of filtering */
void SequenceScoring::TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, vector< pair<TSequence, TSequence> > &SubsequenceCombos)
{
  /* UniqueCorrelations is a map of clusters uniquely correlated
     Cluster -> TurnsInto
           1 -> 1
           2 -> 3
           3 -> 2 */

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes;
  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = TimeSequencePercentage[i].begin()->first;
      if (UniqueCorrelations.find( ClusterSoloInSequence ) != UniqueCorrelations.end())
      {
        Indexes.push_back( i );
      }
    }
  }
  /* DEBUG */
  cout << "Indexes -> ";
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    cout << Indexes[i] << " ";
  }
  cout << endl;


  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1;

    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes[i];
    int To=Indexes[j];

    TSequence Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    int ClusterFrom       = TimeSequencePercentage[From].begin()->first;
    int ClusterTo         = TimeSequencePercentage[To].begin()->first;
    int CorrespondingFrom = UniqueCorrelations[ClusterFrom];
    int CorrespondingTo   = UniqueCorrelations[ClusterTo];

    cout << "[DEBUG] Subsequence from " << ClusterFrom << " to " << ClusterTo << " in Sequence 1:" << endl << " * ";
    PrintSequence( Subsequence );

    /* Look for subsequences between 'CorrespondingFrom' and 'CorrespondingTo' clusters */
    vector< TSequence > SubsequencesList;
    FindAllSubsequences(SS2, CorrespondingFrom, CorrespondingTo, SubsequencesList);
    cout << "[DEBUG] Found " << SubsequencesList.size() << " subsequences from corresponding clusters " << CorrespondingFrom << " and " << CorrespondingTo << " in Sequence 2:" << endl;

    for (int k=0; k<SubsequencesList.size(); k++)
    {
      cout << " * Subsequence #" << k+1 << ": ";
      PrintSequence( SubsequencesList[k] );

      pair<TSequence, TSequence> p = make_pair(Subsequence, SubsequencesList[k]);
      SubsequenceCombos.push_back(p);
    }
  }
}
#endif

/* This implementation is better, only looks for subsequences between consecutive indexes in both sequences! */
void SequenceScoring::TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  /* UniqueCorrelations is a map of clusters uniquely correlated
     Cluster -> TurnsInto
           1 -> 1
           2 -> 3
           3 -> 2 */

  /* Get the inverse mapping */
  map<INT32, INT32> UniqueInverted;
  map<INT32, INT32>::iterator it;
  for (it = UniqueCorrelations.begin(); it != UniqueCorrelations.end(); ++it)
  {
    UniqueInverted[it->second] = it->first;
  }

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes, Indexes2;
  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = TimeSequencePercentage[i].begin()->first;
      if (UniqueCorrelations.find( ClusterSoloInSequence ) != UniqueCorrelations.end())
      {
        Indexes.push_back( i );
      }
    }
  }

  /* Locate in the second global time sequence where the (right) corresponding clusters appear with a score of 100% */
  for (int i = 0; i < SS2->TimeSequencePercentage.size(); i++)
  {
    if (SS2->TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = SS2->TimeSequencePercentage[i].begin()->first;
      if (UniqueInverted.find( ClusterSoloInSequence ) != UniqueInverted.end())
      {
        Indexes2.push_back( i );
      }
    }
  }

  /* DEBUG 
  cout << "Indexes -> ";
  for (int i = 0; i<Indexes.size(); i++)
  {
    cout << Indexes[i] << " ";
  }
  cout << endl;

  cout << "Indexes2 -> ";
  for (int i = 0; i<Indexes2.size(); i++)
  {
    cout << Indexes2[i] << " ";
  }
  cout << endl; */

  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1; 
   
    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes[i];
    int To=Indexes[j];

    TSequence Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    int ClusterFrom       = TimeSequencePercentage[From].begin()->first;
    int ClusterTo         = TimeSequencePercentage[To].begin()->first;
    int CorrespondingFrom = UniqueCorrelations[ClusterFrom];
    int CorrespondingTo   = UniqueCorrelations[ClusterTo];

    cout << "[DEBUG] Subsequence from " << ClusterFrom << " to " << ClusterTo << " in Sequence 1:" << endl << " * ";
    PrintSequence( Subsequence );

    /* Look for subsequences between 'CorrespondingFrom' and 'CorrespondingTo' clusters */
    vector< TSequence > SubsequencesList;

    for (int ii = 0; ii<Indexes2.size()-1; ii++)
    {
      int jj = ii+1;
      TSequence Subsequence2;

      int From2=Indexes2[ii];
      int To2=Indexes2[jj];
      if ((SS2->TimeSequencePercentage[From2].begin()->first == CorrespondingFrom) && 
          (SS2->TimeSequencePercentage[To2].begin()->first == CorrespondingTo))
      {
        SS2->GetSubsequence(SS2, From2, To2, Subsequence2);
        if (Subsequence2.size() <= 0) continue;
        SubsequencesList.push_back( Subsequence2 );
      }
    }
    cout << "[DEBUG] Found " << SubsequencesList.size() << " subsequences from corresponding clusters " << CorrespondingFrom << " and " << CorrespondingTo << " in Sequence 2:" << endl;

    for (int k=0; k<SubsequencesList.size(); k++)
    {
      cout << " * Subsequence #" << k+1 << ": ";
      PrintSequence( SubsequencesList[k] );

      GetMatchingsBetweenSequences( Subsequence, SubsequencesList[k], SequenceMatchings );
    }
  }
  cout << "[DEBUG] Matchings between subsequences:" << endl;
  PrintMatchingsBetweenSequences(SequenceMatchings);
}

#if 0
/* This implementation filters subsequences of the same size */
void SequenceScoring::TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  /* UniqueCorrelations is a map of clusters uniquely correlated
     Cluster -> TurnsInto
           1 -> 1
           2 -> 3
           3 -> 2 */

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes;
  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = TimeSequencePercentage[i].begin()->first;
      if (UniqueCorrelations.find( ClusterSoloInSequence ) != UniqueCorrelations.end())
      {
        Indexes.push_back( i );
      }
    }
  }

  cout << "Len -> " << TimeSequencePercentage.size() << endl;
  /* DEBUG */
  cout << "Indexes -> ";
  for (int i = 0; i<Indexes.size(); i++)
  {
    cout << Indexes[i] << " ";
  }
  cout << endl;



  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1; 
   
    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes[i];
    int To=Indexes[j];

    TSequence Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    int ClusterFrom       = TimeSequencePercentage[From].begin()->first;
    int ClusterTo         = TimeSequencePercentage[To].begin()->first;
    int CorrespondingFrom = UniqueCorrelations[ClusterFrom];
    int CorrespondingTo   = UniqueCorrelations[ClusterTo];

    cout << "[DEBUG] Subsequence from " << ClusterFrom << " to " << ClusterTo << " in Sequence 1:" << endl << " * ";
    PrintSequence( Subsequence );

    /* Look for subsequences between 'CorrespondingFrom' and 'CorrespondingTo' clusters */
    vector< TSequence > SubsequencesList;
    FindAllSubsequences(SS2, CorrespondingFrom, CorrespondingTo, SubsequencesList);
    cout << "[DEBUG] Found " << SubsequencesList.size() << " subsequences from corresponding clusters " << CorrespondingFrom << " and " << CorrespondingTo << " in Sequence 2:" << endl;

    for (int k=0; k<SubsequencesList.size(); k++)
    {
      cout << " * Subsequence #" << k+1 << ": ";
      PrintSequence( SubsequencesList[k] );

      GetMatchingsBetweenSequences( Subsequence, SubsequencesList[k], SequenceMatchings );
    }
  }
  cout << "[DEBUG] Matchings between subsequences:" << endl;
  PrintMatchingsBetweenSequences(SequenceMatchings);
}
#endif

#if 0
/* This implementation splits groups into 1 <-> 1 correlations according to the cluster score */
void SequenceScoring::GetMatchingsBetweenSequences(TSequence &Seq1, TSequence &Seq2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  if (Seq1.size() == Seq2.size())
  {
    for (int i=0; i<Seq1.size(); i++)
    {
      if ((Seq1[i].size() == Seq2[i].size()) && (Seq1[i].size() > 0) && (Seq2[i].size() > 0))
      {
        if (Discriminable(Seq1[i]) && Discriminable(Seq2[i]))
        {
          SplitDiscriminableByPct(Seq1[i], Seq2[i], SequenceMatchings);
        }
      }
    }
  }
}
#else
/* This version links groups of clusters without trying to split them, as long as the subsequences have the same length */
void SequenceScoring::GetMatchingsBetweenSequences(TSequence &Seq1, TSequence &Seq2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  if (Seq1.size() == Seq2.size())
  {
    for (int i=0; i<Seq1.size(); i++)
    {
      if ((Seq1[i].size() == Seq2[i].size()) && (Seq1[i].size() > 0) && (Seq2[i].size() > 0))
      {
        SequenceMatchings[Seq1[i]] = Seq2[i];
      }
    }
  }
}
#endif

#define MIN_DISCRIMINATIVE_PCT 3
bool SequenceScoring::Discriminable(TClusterScoreMap &m)
{
  TClusterScoreMap::iterator it, it2;
  for (it=m.begin(); it!=m.end(); ++it)
  {
    int Pct = it->second;
    it2 = it;
    it2 ++;
    while (it2 != m.end())
    {
      int Pct2 = it2->second;
      int Delta = Pct - Pct2;
      if ((Delta >= -MIN_DISCRIMINATIVE_PCT) && (Delta <= MIN_DISCRIMINATIVE_PCT))
      {
        /* Percentages are too similar (or equal) and can not discriminate who is who */
        return false;
      } 
      it2 ++;
    }
  }
  return true;
}

void SequenceScoring::SplitDiscriminableByPct(TClusterScoreMap m1, TClusterScoreMap m2, map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  while (m1.size() > 0)
  {
    int ClusterID  = m1.begin()->first;
    int ClusterPct = m1.begin()->second;
    m1.erase (m1.begin()); 
    
    int MinDelta = 100;
    TClusterScoreMap::iterator it, min_it;

    /* Match cluster with the one with most similar percentage */
    for (it=m2.begin(); it!=m2.end(); ++it)
    {
      int Delta = it->second - ClusterPct;
      if (Delta < 0) Delta = Delta * (-1);
      if (Delta < MinDelta) 
      {
        MinDelta = Delta;
        min_it = it;
      }
    }
    int CorrespondingID  = min_it->first;
    int CorrespondingPct = min_it->second;
    m2.erase(min_it);

    TClusterScoreMap Left;
    Left[ClusterID] = ClusterPct;
    TClusterScoreMap Right;
    Right[CorrespondingID] = CorrespondingPct;

    SequenceMatchings[Left] = Right;
  }
}

void SequenceScoring::PrintMatchingsBetweenSequences(map<TClusterScoreMap, TClusterScoreMap> &SequenceMatchings)
{
  map<TClusterScoreMap, TClusterScoreMap>::iterator it;
  for (it=SequenceMatchings.begin(); it!=SequenceMatchings.end(); it++)
  {
    TClusterScoreMap m1 = it->first;
    PrintClustersScore( m1 );
    cout << "--> ";
    TClusterScoreMap m2 = it->second;
    PrintClustersScore( m2 );
    cout << endl;
  }
}

#if 0
void VERY_OLD_TimeCorrelation(map<INT32, INT32> &UniqueCorrelations, SequenceScoring *SS2, map< INT32, set<INT32> > &SequenceMatchings)
{
  /* UniqueCorrelations is a map of clusters uniquely correlated
     Cluster -> TurnsInto
           1 -> 1
           2 -> 3
           3 -> 2 */
  map<INT32, INT32>::iterator it;

  for (it=UniqueCorrelations.begin(); it!=UniqueCorrelations.end(); it++)
  {
    int ThisCluster = it->first;
    int TurnsInto   = it->second;
  }

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes;
  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int ClusterSoloInSequence = TimeSequencePercentage[i].begin()->first;
      if (UniqueCorrelations.find( ClusterSoloInSequence ) != UniqueCorrelations.end())
      {
        Indexes.push_back( i );
      }
    }
  }

  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1; 
   
    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes[i];
    int To=Indexes[j];

    vector< INT32 > Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    int ClusterFrom = TimeSequencePercentage[From].begin()->first;
    int ClusterTo   = TimeSequencePercentage[To].begin()->first;
    int CounterFrom = UniqueCorrelations[ClusterFrom];
    int CounterTo   = UniqueCorrelations[ClusterTo];

    cout << "Subsequence from " << ClusterFrom << " to " << ClusterTo << ": ";
    for (int k=0; k<Subsequence.size(); k++)
    {
      cout << Subsequence[k] << " ";
    }
    cout << endl;

    /* Look for subsequences between 'CounterFrom' and 'CounterTo' clusters */
    cout << "Look for subsequences between " << CounterFrom << " and " << CounterTo << " in Sequence 2" << endl;
    vector< vector<INT32> > SubsequencesList;
    FindAllSubsequences(SS2, CounterFrom, CounterTo, SubsequencesList);
    cout << "Found " << SubsequencesList.size() << " subsequences" << endl;
    for (int k=0; k<SubsequencesList.size(); k++)
    {
      cout << "Subsequence #" << k+1 << ": ";
      for (int l=0; l<SubsequencesList[k].size(); l++)
      {
        cout << SubsequencesList[k][l] << " ";
      }
      cout << endl;

      if (Subsequence.size() == SubsequencesList[k].size())
      {
        for (int m=0; m<Subsequence.size(); m++)
        {
          if ((Subsequence[m] != -1) && (SubsequencesList[k][m] != -1))
          {
            SequenceMatchings[ Subsequence[m] ].insert( SubsequencesList[k][m] ); 
          }
        }
      }
    }
  }

  cout << "Cluster correlation according to time sequence:" << endl;
  map< INT32, set<INT32> >::iterator it2;
  for(it2 = SequenceMatchings.begin(); it2 != SequenceMatchings.end(); ++ it2)
  {
    cout << it2->first << " <-> ";
    set<INT32>::iterator it3;
    for (it3 = it2->second.begin(); it3 != it2->second.end(); ++ it3)
    {
      cout << *it3 << " ";
    }
    cout << endl;
  }


#if 0
  set<INT32> CheckpointClusters;
  CheckpointClusters.insert(1);
  CheckpointClusters.insert(2);
  CheckpointClusters.insert(3);
  vector<INT32> Indexes;

  for (int i = 0; i < TimeSequencePercentage.size(); i++)
  {
    if (TimeSequencePercentage[i].size() == 1)
    {
      int UniqueClusterInSequence = TimeSequencePercentage[i].begin()->first;
cout << "UniqueClusterInSequence = " << UniqueClusterInSequence << endl;
      if (CheckpointClusters.find( UniqueClusterInSequence ) != CheckpointClusters.end())
      {
        Indexes.push_back( i );
      }
    }
  }

  
  for (int i = 0; i<Indexes.size()-1; i++)
  {
    int j = i+1; 
   
    int From=Indexes[i];
    int To=Indexes[j];

    vector< INT32 > Subsequence;
    for (int k=From+1; k<To; k++)
    {
      if (TimeSequencePercentage[k].size() == 1)
      {
        Subsequence.push_back( TimeSequencePercentage[k].begin()->first );
      } 
      else
      {
        Subsequence.push_back( -1 );
      }
    }
    cout << "Subsequence from " << From << " to " << To << ": ";
    for (int k=0; k<Subsequence.size(); k++)
    {
      cout << Subsequence[k] << " ";
    }
    cout << endl;
  }
#endif
}
#endif



/***************************************************************************
 * 'LoadAminoacidTable' PRIVATE
 **************************************************************************/
void
SequenceScoring::LoadAminoacidTable(void)
{
  /* Value 0 is reserved for alignment special char such as '-' o 'X' */
  /* A  B  C  D  E  F  G  H  I  K  L  M  N  P  Q  R  S  T  V  W  X  Y  Z  * */

  AminoacidTranslation['Z'] = 0;
  AminoacidTranslation['A'] = 1;
  AminoacidTranslation['B'] = 2;
  AminoacidTranslation['C'] = 3;
  AminoacidTranslation['D'] = 4;
  AminoacidTranslation['E'] = 5;
  AminoacidTranslation['F'] = 6;
  AminoacidTranslation['G'] = 7;
  AminoacidTranslation['H'] = 8;
  AminoacidTranslation['I'] = 9;
  AminoacidTranslation['K'] = 10;
  AminoacidTranslation['L'] = 11;
  AminoacidTranslation['M'] = 12;
  AminoacidTranslation['N'] = 13;
  AminoacidTranslation['P'] = 14;
  AminoacidTranslation['Q'] = 15;
  AminoacidTranslation['R'] = 16;
  AminoacidTranslation['S'] = 17;
  AminoacidTranslation['T'] = 18;
  AminoacidTranslation['V'] = 19;
  AminoacidTranslation['W'] = 20;
  AminoacidTranslation['X'] = 21;
  AminoacidTranslation['Y'] = 22;

  /* WTF 
  AminoacidTranslation.insert(make_pair('Z', 0));
  AminoacidTranslation.insert(make_pair('A', 1));
  AminoacidTranslation.insert(make_pair('B', 2));
  AminoacidTranslation.insert(make_pair('C', 3));
  AminoacidTranslation.insert(make_pair('D', 4));
  AminoacidTranslation.insert(make_pair('E', 5));
  AminoacidTranslation.insert(make_pair('F', 6));
  AminoacidTranslation.insert(make_pair('G', 7));
  AminoacidTranslation.insert(make_pair('H', 8));
  AminoacidTranslation.insert(make_pair('I', 9));
  AminoacidTranslation.insert(make_pair('K', 10));
  AminoacidTranslation.insert(make_pair('L', 11));
  AminoacidTranslation.insert(make_pair('M', 12));
  AminoacidTranslation.insert(make_pair('N', 13));
  AminoacidTranslation.insert(make_pair('P', 14));
  AminoacidTranslation.insert(make_pair('Q', 15));
  AminoacidTranslation.insert(make_pair('R', 16));
  AminoacidTranslation.insert(make_pair('S', 17));
  AminoacidTranslation.insert(make_pair('T', 18));
  AminoacidTranslation.insert(make_pair('V', 19));
  AminoacidTranslation.insert(make_pair('W', 20));
  AminoacidTranslation.insert(make_pair('X', 21));
  AminoacidTranslation.insert(make_pair('Y', 22));
  */
  
  /* TESTING 
  AminoacidTranslation.insert(make_pair('*', 1));
  AminoacidTranslation.insert(make_pair('A', 2));
  AminoacidTranslation.insert(make_pair('B', 3));
  AminoacidTranslation.insert(make_pair('C', 4));
  AminoacidTranslation.insert(make_pair('D', 5));
  AminoacidTranslation.insert(make_pair('E', 6));
  AminoacidTranslation.insert(make_pair('F', 7));
  AminoacidTranslation.insert(make_pair('G', 8));
  AminoacidTranslation.insert(make_pair('H', 9));
  AminoacidTranslation.insert(make_pair('I', 10));
  AminoacidTranslation.insert(make_pair('K', 11));
  AminoacidTranslation.insert(make_pair('L', 12));
  AminoacidTranslation.insert(make_pair('M', 13));
  AminoacidTranslation.insert(make_pair('N', 14));
  AminoacidTranslation.insert(make_pair('P', 15));
  AminoacidTranslation.insert(make_pair('Q', 16));
  AminoacidTranslation.insert(make_pair('R', 17));
  AminoacidTranslation.insert(make_pair('S', 18));
  AminoacidTranslation.insert(make_pair('T', 19));
  AminoacidTranslation.insert(make_pair('V', 20));
  AminoacidTranslation.insert(make_pair('W', 21));
  AminoacidTranslation.insert(make_pair('X', 22));
  AminoacidTranslation.insert(make_pair('Y', 23));
  AminoacidTranslation.insert(make_pair('Z', 24));
  */
  /*
  AminoacidTranslation.insert(make_pair('Z', 1));
  AminoacidTranslation.insert(make_pair('A', 2));
  AminoacidTranslation.insert(make_pair('B', 3));
  AminoacidTranslation.insert(make_pair('C', 4));
  AminoacidTranslation.insert(make_pair('D', 5));
  AminoacidTranslation.insert(make_pair('E', 6));
  AminoacidTranslation.insert(make_pair('F', 7));
  AminoacidTranslation.insert(make_pair('G', 8));
  AminoacidTranslation.insert(make_pair('H', 9));
  AminoacidTranslation.insert(make_pair('I', 10));
  AminoacidTranslation.insert(make_pair('J', 11));
  AminoacidTranslation.insert(make_pair('K', 12));
  AminoacidTranslation.insert(make_pair('L', 13));
  AminoacidTranslation.insert(make_pair('M', 14));
  AminoacidTranslation.insert(make_pair('N', 15));
  AminoacidTranslation.insert(make_pair('O', 16));
  AminoacidTranslation.insert(make_pair('P', 17));
  AminoacidTranslation.insert(make_pair('Q', 18));
  AminoacidTranslation.insert(make_pair('R', 19));
  AminoacidTranslation.insert(make_pair('S', 20));
  AminoacidTranslation.insert(make_pair('T', 21));
  AminoacidTranslation.insert(make_pair('U', 22));
  AminoacidTranslation.insert(make_pair('V', 23));
  AminoacidTranslation.insert(make_pair('W', 24));
  AminoacidTranslation.insert(make_pair('X', 25));
  AminoacidTranslation.insert(make_pair('Y', 26));
  /*
  AminoacidTranslation.insert(make_pair('a', 26));
  AminoacidTranslation.insert(make_pair('b', 27));
  AminoacidTranslation.insert(make_pair('c', 28));
  AminoacidTranslation.insert(make_pair('d', 29));
  AminoacidTranslation.insert(make_pair('e', 30));
  AminoacidTranslation.insert(make_pair('f', 31));
  AminoacidTranslation.insert(make_pair('g', 32));
  AminoacidTranslation.insert(make_pair('h', 33));
  AminoacidTranslation.insert(make_pair('i', 34));
  AminoacidTranslation.insert(make_pair('j', 35));
  AminoacidTranslation.insert(make_pair('k', 36));
  AminoacidTranslation.insert(make_pair('l', 37));
  AminoacidTranslation.insert(make_pair('m', 38));
  AminoacidTranslation.insert(make_pair('n', 39));
  AminoacidTranslation.insert(make_pair('o', 40));
  AminoacidTranslation.insert(make_pair('p', 41));
  AminoacidTranslation.insert(make_pair('q', 42));
  AminoacidTranslation.insert(make_pair('r', 43));
  AminoacidTranslation.insert(make_pair('s', 44));
  AminoacidTranslation.insert(make_pair('t', 45));
  AminoacidTranslation.insert(make_pair('u', 46));
  AminoacidTranslation.insert(make_pair('v', 47));
  AminoacidTranslation.insert(make_pair('w', 48));
  AminoacidTranslation.insert(make_pair('y', 49));
  AminoacidTranslation.insert(make_pair('z', 50));
  */
}

/***************************************************************************
 * 'LoadFASTASequences' PRIVATE
 **************************************************************************/
bool
SequenceScoring::LoadFASTASequences(void)
{
  map<char, INT32>::iterator AminoacidTranslationIterator;
  INT32  CurrentSequence = -1;
  string CurrentLine;

  while (!SequenceFileStream.eof())
  {
    getline(SequenceFileStream, CurrentLine);
    
    if (CurrentLine.size() == 0)
      continue;
    
    if (CurrentLine.at(0) == '>')
    { /* New sequence! */
      CurrentSequence++;
      ClusterSequences.push_back(vector<INT32> ());
    }
    else
    {
      /* Fill current sequence */
      for (INT32 i = 0; i < CurrentLine.size(); i++)
      {
        /* INT32 CurrentClusterValue = (INT32) (CurrentLine.at(i) - 'A')+1; */
        INT32 CurrentClusterValue;

        AminoacidTranslationIterator = 
          AminoacidTranslation.find(CurrentLine.at(i));
        
        if (AminoacidTranslationIterator == AminoacidTranslation.end())
        {
          CurrentClusterValue = 0;
        }
        else
        {
          CurrentClusterValue = AminoacidTranslationIterator->second;
        }
        
        /*
        if (CurrentLine.at(i) == '-')
        {
          CurrentClusterValue = 0;
        }
        */

        if (CurrentClusterValue > DifferentClusters)
          DifferentClusters = CurrentClusterValue;
        
        ClusterSequences[CurrentSequence].push_back(CurrentClusterValue);
      }
    }
  }
  
  DifferentClusters++;

  if (SequenceFileStream.fail() && !SequenceFileStream.eof())
  {
    SetError(true);
    SetErrorMessage("Error loading sequences", strerror(errno));
    return false;
  }
  
  return true;
}

bool
SequenceScoring::LoadSequences(void)
{
  map<char, INT32>::iterator AminoacidTranslationIterator;
  INT32  CurrentSequence = -1;
  string CurrentLine;

  while (!SequenceFileStream.eof())
  {
    getline(SequenceFileStream, CurrentLine);

    if (CurrentLine.size() == 0)
      continue;

    if (CurrentLine.at(0) == '>')
    { /* New sequence! */
      CurrentSequence++;
      ClusterSequences.push_back(vector<INT32> ());
    }
    else
    {
      /* Fill current sequence */
      string token;
      stringstream ss_line;
      ss_line << CurrentLine;

      while (getline(ss_line, token, ','))
      {
        stringstream ss_token;
        ss_token << token;
        int CurrentClusterValue;
        ss_token >> CurrentClusterValue;
        ClusterSequences[CurrentSequence].push_back(CurrentClusterValue);

        if (CurrentClusterValue > DifferentClusters)
          DifferentClusters = CurrentClusterValue;
      }
    }
  }

  DifferentClusters++;

  if (SequenceFileStream.fail() && !SequenceFileStream.eof())
  {
    SetError(true);
    SetErrorMessage("Error loading sequences", strerror(errno));
    return false;
  }

  return true;
}



/***************************************************************************
 * 'CheckSequences' PRIVATE
 **************************************************************************/

bool
SequenceScoring::CheckSequences(INT32& SequencesLength)
{
  /* Check if there are more than 2 sequences available */
  if (ClusterSequences.size() == 0)
  {
    SetError(true);
    SetErrorMessage("No sequences to analyze");
    return false;
  }
  
  if (ClusterSequences.size() == 1)
  {
    SetError(true);
    SetErrorMessage("Number of sequences must be greater than 1");
    return false;
  }
  
  /* Check if all sequences have the same length */
  SequencesLength = ClusterSequences[0].size();
  for (INT32 i = 1; i < ClusterSequences.size(); i++)
  {
    if (ClusterSequences[i].size() != SequencesLength)
    {
      SetError(true);
      SetErrorMessage("All the sequences must have the same length");
      return false;
    }
  }
  
  return true;
}

/***************************************************************************
 * 'ColumnClustersInfo' PRIVATE
 **************************************************************************/

bool
SequenceScoring::ColumnClustersInfo(ifstream& ClustersInfoStream)
{
  /* Check whether  'clusters_info' file is distributed in columns or rows */
  string FirstLine;
  
  getline(ClustersInfoStream, FirstLine);
  
  /* Check if 'Density' word is in this first line. If it is then the 
     clusters_info are distributed as rows */
  
  if (FirstLine.find("Density") != string::npos)
  {
    return false;
  }
  else
  {
    return true;
  }
}


/***************************************************************************
 * 'GetClustersDuration' PRIVATE
 **************************************************************************/

bool
SequenceScoring::GetClustersDuration(ifstream&       ClustersInfoStream,
                                     vector<UINT64>& ClustersDuration)
{
  string CurrentLine;
  bool   ColumnDisposition;
  
  ColumnDisposition = ColumnClustersInfo(ClustersInfoStream);
  
  /* Seek the stream to initial position */
  ClustersInfoStream.seekg(0, ios::beg);
  
  if (ColumnDisposition)
  {
    
    getline(ClustersInfoStream, CurrentLine); /* Header */
    getline(ClustersInfoStream, CurrentLine); /* Density */
    
    getline(ClustersInfoStream, CurrentLine); /* Total duration */
    
    if (!ParseColumnLine(CurrentLine, ClustersDuration))
    {
      SetError(true);
      SetErrorMessage("Error parsing duration line");
      return false;
    }
  }
  else
  {
    getline(ClustersInfoStream, CurrentLine); /* Header */
    while (!getline(ClustersInfoStream, CurrentLine).eof())
    {
     if (!ParseRowLine(CurrentLine, ClustersDuration))
      {
        SetError(true);
        SetErrorMessage("Error parsing line to get duration");
        return false;
      }
    }
  }

  /* DEBUG 
  cout << "Clusters Duration = ";
  for (INT32 i = 0; i < ClustersDuration.size(); i++)
  {
    cout << ClustersDuration[i];
    
    if (i < ClustersDuration.size() - 1)
      cout << ",";
  }
  cout << endl; */
  
  
  return true;
}

/***************************************************************************
 * 'ParseColumnLine' PRIVATE
 **************************************************************************/

bool
SequenceScoring::ParseColumnLine(string          Line, 
                                 vector<UINT64>& ClustersDuration)
{
  rule<> TotalDurationRule;
  uint_parser<UINT64, 10, 1, -1> timestamp_p;

  TotalDurationRule
    = (str_p("Total Duration")) >>
      *( ',' >> (timestamp_p[push_back_a(ClustersDuration)]))
      >> end_p
    ;
  
  BOOST_SPIRIT_DEBUG_RULE(TotalDurationRule);
  
  
  return parse(Line.c_str(), TotalDurationRule).full;
}


/***************************************************************************
 * 'ParseRowLine' PRIVATE
 **************************************************************************/

bool
SequenceScoring::ParseRowLine(string Line, vector<UINT64>& ClustersDuration)
{
  rule<> ClusterStatsRule;
  uint_parser<UINT64, 10, 1, -1> timestamp_p;
  
  ClusterStatsRule
    = +(alpha_p|digit_p|"_") >> ',' >> (uint_p) >> ',' >> 
       (timestamp_p[push_back_a(ClustersDuration)])
    ;
  
  BOOST_SPIRIT_DEBUG_RULE(ClusterStatsRule);
  
  return parse(Line.c_str(), ClusterStatsRule).hit;
}

