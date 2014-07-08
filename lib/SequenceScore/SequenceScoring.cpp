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


#include <iostream>
#include <string>
#include <sstream>
#include <errno.h>
#include <string.h>
/* #define BOOST_SPIRIT_DEBUG */
#include <boost/spirit/include/classic_rule.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_refactoring.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include "SequenceScoring.h"

using namespace std;
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
  
  ComputeGlobalSequence();

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
SequenceScoring::GetScores(vector<double>& Scores, vector<INT32>& Appearances)
{
  INT32 SequencesLength;
  
  Appearances = vector<INT32> (DifferentClusters, 0);
  Scores      = vector<double> (DifferentClusters, 0.0);
  
  if (!CheckSequences(SequencesLength))
    return false;
  
  /* DEBUG 
  cout << "GetScores: SequencesLength=" << SequencesLength << endl; */

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
        Appearances[j]++;
        Scores[j] += (1.0*Hits[j]/ClusterSequences.size());
      }
    }
  }
  
  /* DEBUG 
  cout << "GetScores: DifferentClusters=" << DifferentClusters << endl; */

  for (INT32 i = 0; i < DifferentClusters; i++)
  {
    Scores[i] = Scores[i]/Appearances[i];
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

/***************************************************************************
 * 'ComputeGlobalSequence' PUBLIC
 ***************************************************************************/

void 
SequenceScoring::ComputeGlobalSequence()
{
  unsigned int SequenceLength;

  if ( ClusterSequences.size() <= 0 ) return;

  /* All sequences have the same length */
  SequenceLength = ClusterSequences[0].size();

  /* Iterate the sequence */
  for (unsigned int i = 0; i < SequenceLength; i++)
  {
    vector< vector<INT32> >::iterator it;

    GlobalSequence.push_back( ClusterScore_t() );

    /* Iterate the tasks */
    for (it = ClusterSequences.begin(); it != ClusterSequences.end(); ++ it)
    {
      INT32 CurrentClusterValue;

      CurrentClusterValue = (*it)[i];

      if (CurrentClusterValue != 0)
      {
        /* Store as value the number of times this cluster appears in the i-position of the sequence */
        if (GlobalSequence[i].find(CurrentClusterValue) == GlobalSequence[i].end())
        {
          GlobalSequence[i][CurrentClusterValue] = 1;
        }
        else
        {
          GlobalSequence[i][CurrentClusterValue] = GlobalSequence[i][CurrentClusterValue] + 1;
        }
      }
    }
  }
  /* DEBUG 
  cout << "Time Sequence (length: " << GlobalSequence.size() << ")" << endl; */
  for (int i = 0; i < GlobalSequence.size(); i++)
  {
    ClusterScore_t::iterator it;

    // Store for each cluster (key) the percentage of occurrence in the sequence (value) 
    for (it=GlobalSequence[i].begin(); it!=GlobalSequence[i].end(); ++it)
    {
      it->second = (it->second * 100) / ClusterSequences.size();
    }

    /* DEBUG
    int set_size = GlobalSequence[i].size();
    if (set_size > 1) cout << "[";
    int count = 0;
    for (it = GlobalSequence[i].begin(); it != GlobalSequence[i].end(); ++ it)
    {
      cout << it->first << "(" << it->second << "%)";
      if (count < GlobalSequence[i].size() - 1) cout << ",";
      count ++;
    }
    if (set_size > 1) cout << "]";
    cout << " " << endl;
    */
  }
}

/***************************************************************************
 * 'FindSimultaneousClusters' PUBLIC
 ***************************************************************************/

int
SequenceScoring::FindSimultaneousClusters (ClusterID_t ClusterID, SimultaneousClusters_t &Simultaneous)
{
  int NumberOfAppearances = 0;
  for (int i = 0; i < GlobalSequence.size(); i++)
  {
    if (GlobalSequence[i].find(ClusterID) != GlobalSequence[i].end())
    {
      NumberOfAppearances ++;
      map<INT32, INT32>::iterator it;

      for (it = GlobalSequence[i].begin(); it != GlobalSequence[i].end(); ++ it)
      {
        if ((it->first != ClusterID) && (it->second > MIN_OCCURRENCE_PCT))
        {
          if (Simultaneous.find(it->first) == Simultaneous.end()) 
          {
            Simultaneous[ it->first ] = 0;
          }
          Simultaneous[ it->first ] ++;
        }
      }
    }
  }
  return NumberOfAppearances;
}


/***************************************************************************
 * 'GetSubsequence' PUBLIC
 ***************************************************************************/

void SequenceScoring::GetSubsequence(SequenceScoring *SS, int FromIndex, int ToIndex, Sequence_t &Subsequence)
{
  for (int i=FromIndex+1; i<ToIndex; i++)
  {
    Subsequence.push_back( SS->GlobalSequence[i] );
  }
}

/***************************************************************************
 * 'PrintSequence' PUBLIC
 ***************************************************************************/

void SequenceScoring::PrintSequence(Sequence_t &Sequence)
{
  for (int i=0; i<Sequence.size(); i++)
  {
    cout << i << ":";
    ClusterScore_t ClustersInSequence = Sequence[i];
    PrintClustersScore( ClustersInSequence );
  }
  cout << endl;
}

/***************************************************************************
 * 'PrintClustersScore' PUBLIC
 ***************************************************************************/

void SequenceScoring::PrintClustersScore(ClusterScore_t &Clusters)
{
  ClusterScore_t::iterator it;

  cout << "[ ";
  for (it=Clusters.begin(); it!=Clusters.end(); it++)
  {
    cout << it->first << "(" << it->second << "%) ";
  }
  cout << "] ";
}

/***************************************************************************
 * 'TimeCorrelation' PUBLIC
 ***************************************************************************/

void SequenceScoring::TimeCorrelation(DoubleLinks *UnivocalLinks, SequenceScoring *SS2, Matchings_t &SequenceMatchings)
{
  /* UnivocalLinks is a map of clusters linked 1 to 1 
        From -> To
           1 -> 1
           2 -> 3
           3 -> 2 */

  /* Get the inverse mapping */
  DoubleLinks *UnivocalReverse = UnivocalLinks->Reverse();

  /* Locate in the first global time sequence where the (left) clusters appear with a score of 100% */
  vector<INT32> Indexes1, Indexes2;
  for (int i = 0; i < GlobalSequence.size(); i++)
  {
    if (GlobalSequence[i].size() == 1)
    {
      ClusterID_t  ClusterSoloInSequence = GlobalSequence[i].begin()->first;
      ObjectSet_t OnlyCluster;
      OnlyCluster.insert( ClusterSoloInSequence );

      if (UnivocalLinks->find( OnlyCluster ) != UnivocalLinks->end())
      {
        Indexes1.push_back( i );
      }
    }
  }

  /* Locate in the second global time sequence where the (right) corresponding clusters appear with a score of 100% */
  for (int i = 0; i < SS2->GlobalSequence.size(); i++)
  {
    if (SS2->GlobalSequence[i].size() == 1)
    {
      ClusterID_t ClusterSoloInSequence = SS2->GlobalSequence[i].begin()->first;
      ObjectSet_t OnlyCluster;
      OnlyCluster.insert( ClusterSoloInSequence );
 
      if (UnivocalReverse->find( OnlyCluster ) != UnivocalReverse->end())
      {
        Indexes2.push_back( i );
      }
    }
  }

  /* DEBUG 
  cout << "Indexes1 -> ";
  for (int i = 0; i<Indexes1.size(); i++)
  {
    cout << Indexes1[i] << " ";
  }
  cout << endl;

  cout << "Indexes2 -> ";
  for (int i = 0; i<Indexes2.size(); i++)
  {
    cout << Indexes2[i] << " ";
  }
  cout << endl; */

  /* Find pairs of these clusters in the sequence */
  for (int i = 0; i<Indexes1.size()-1; i++)
  {
    int j = i+1; 
   
    /* Find the subsequence between this pair in the first time sequence */
    int From=Indexes1[i];
    int To=Indexes1[j];

    Sequence_t Subsequence;
    GetSubsequence(this, From, To, Subsequence);
    if (Subsequence.size() <= 0) continue;

    ClusterID_t ClusterFrom       = GlobalSequence[From].begin()->first;
    ClusterID_t ClusterTo         = GlobalSequence[To].begin()->first;

    ObjectSet_t SetFrom, SetTo;
    SetFrom.insert( ClusterFrom );
    SetTo.insert( ClusterTo );

    ObjectSet_t CorrespondingFrom = UnivocalLinks->get_links( SetFrom );
    ObjectSet_t CorrespondingTo   = UnivocalLinks->get_links( SetTo );

    /* 
    cout << "[DEBUG] Subsequence from " << ClusterFrom << " to " << ClusterTo << " in Sequence 1:" << endl;
    PrintSequence( Subsequence );
    */

    /* Look for subsequences between 'CorrespondingFrom' and 'CorrespondingTo' clusters */
    vector< Sequence_t > SubsequencesList;

    for (int ii = 0; ii<Indexes2.size()-1; ii++)
    {
      int jj = ii+1;
      Sequence_t Subsequence2;

      int From2=Indexes2[ii];
      int To2=Indexes2[jj];
      if ((SS2->GlobalSequence[From2].begin()->first == *(CorrespondingFrom.begin())) && 
          (SS2->GlobalSequence[To2].begin()->first == *(CorrespondingTo.begin())))
      {
        SS2->GetSubsequence(SS2, From2, To2, Subsequence2);
        if (Subsequence2.size() <= 0) continue;
        SubsequencesList.push_back( Subsequence2 );
      }
    }
    /* 
    cout << "[DEBUG] Found " << SubsequencesList.size() << " subsequences from corresponding clusters " << CorrespondingFrom << " and " << CorrespondingTo << " in Sequence 2:" << endl;
    */
    for (int k=0; k<SubsequencesList.size(); k++)
    {
      /*
      cout << " * Subsequence #" << k+1 << ": " << endl;
      PrintSequence( SubsequencesList[k] );
      */

      GetMatchingsBetweenSequences( Subsequence, SubsequencesList[k], SequenceMatchings );
    } 
  }
  /*
  cout << "[DEBUG] Matchings between subsequences:" << endl;
  PrintMatchings(SequenceMatchings); */
}

/***************************************************************************
 * 'GetMatchingsBetweenSequences' PRIVATE
 ***************************************************************************/

void SequenceScoring::GetMatchingsBetweenSequences(Sequence_t &Seq1, Sequence_t &Seq2, Matchings_t &SequenceMatchings)
{
  if (Seq1.size() == Seq2.size())
  {
    for (int i=0; i<Seq1.size(); i++)
    {
      if ((Seq1[i].size() > 0) && (Seq2[i].size() > 0))
      {
        ClusterScore_t::iterator it, it2;

        for (it = Seq1[i].begin(); it != Seq1[i].end(); ++ it)
        {
          for (it2 = Seq2[i].begin(); it2 != Seq2[i].end(); ++ it2)
          {
            std::pair<INT32, INT32> p = std::make_pair( it->first, it2->first );
            if (SequenceMatchings.find( p ) != SequenceMatchings.end())
            {
              SequenceMatchings[p] ++;
            }
            else
            {
              SequenceMatchings[p] = 1;
            }
          }
        }
      }
    }
  }
}

/***************************************************************************
 * 'PrintMatchings' PRIVATE
 **************************************************************************/

void SequenceScoring::PrintMatchings(Matchings_t &SequenceMatchings)
{
  Matchings_t::iterator it;

  for (it=SequenceMatchings.begin(); it!=SequenceMatchings.end(); it++)
  {
    cout << it->first.first << " -> " << it->first.second << " (" << it->second << ")" << endl;
  }
}

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

/***************************************************************************
 * 'LoadSequences' PRIVATE
 **************************************************************************/

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

