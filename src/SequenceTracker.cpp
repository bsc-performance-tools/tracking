#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <libgen.h>
#include "CorrelationMatrix.h"
#include "SequenceTracker.h"
#include "Utils.h"
#include "Links.h"

SequenceTracker::SequenceTracker(string AlignFile, string CINFOFile)
{
  if (FileExists(AlignFile) && FileExists(CINFOFile))
  {
    vector<INT32> Appareances;

    SS = new SequenceScoring(AlignFile);
    SS->LoadClustersInformation(CINFOFile);
    SS->GetScores(Scores, Appareances);

    if (!SS->GetGlobalScore(Scores, GlobalScore))
    {
      cout << "ERROR: Unable to compute global score";
      cout << " (" << SS->GetLastError() << ")" << endl << endl;
      delete SS;
      SS          = NULL;
      GlobalScore = 0.0;
    }
  }
  else
  {
    cout << "WARNING: Time sequence not available (Alignment file '" << basename((char *)(AlignFile.c_str())) << "' not found)" << endl << endl;
    SS          = NULL;
    GlobalScore = 0.0;
  }
}

SequenceTracker::~SequenceTracker()
{
  if (SS != NULL) delete SS;
}

bool SequenceTracker::isAvailable()
{
  return (SS != NULL);
}

double SequenceTracker::getGlobalScore()
{
  return GlobalScore;
}

double SequenceTracker::getClusterScore(CID cluster_id)
{
  if ((cluster_id < 1) || (cluster_id > Scores.size()))
  {
    cerr << "SequenceTracker::getClusterScore: ERROR: Invalid cluster ID '" << cluster_id << "'" << endl;
    return 0;
  }
  return Scores[cluster_id]; /* XXX Assumes index 0 is for NOISE */
}

int SequenceTracker::getNumberOfClusters()
{
  return Scores.size() - 1; /* XXX -1 assumes there's always NOISE in index 0 */
}

void SequenceTracker::dump()
{
  cout << "Global Score = " << GlobalScore << endl;
  cout << "Number of clusters = " << getNumberOfClusters() << endl;
  for (int i=1; i<=getNumberOfClusters(); i++)
  {
    cout << "Score for cluster " << i << " = " << getClusterScore(i) << endl;
  }
}

CorrelationMatrix * SequenceTracker::getClustersSimultaneity()
{
  if (SS == NULL) return NULL;

  int ClustersInSequence = getNumberOfClusters();
  CorrelationMatrix *CCM = new CorrelationMatrix(ClustersInSequence, ClustersInSequence);

  for (int i=1; i<=ClustersInSequence; i++)
  {
    map<INT32, int> Simultaneous; /* map[cluster] = probability of appearing together */
    int NumberOfAppearances = SS->FindSimultaneousClusters( i, Simultaneous );

    /* DEBUG 
    cout << "[DEBUG] Simultaneity for cluster " << i << " = "; */
    map<INT32, int>::iterator it;
    for (it = Simultaneous.begin(); it != Simultaneous.end(); it ++)
    {
      int SeenCount = it->second;
      /* DEBUG 
      cout << it->first << " (" << (SeenCount * 100)/NumberOfAppearances << "%) "; */
      CCM->SetStats(i, it->first, SeenCount, (SeenCount * 100)/NumberOfAppearances);
    }
    /* DEBUG
    cout << endl; */ 
  }
  return CCM;
}


CorrelationMatrix * SequenceTracker::CorrelateWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2)
{
  map<TClusterScoreMap, TClusterScoreMap> SequenceMatchings;
  map<TClusterScoreMap, TClusterScoreMap>::iterator it;

  SS->TimeCorrelation(UniqueCorrelations, ST2->SS, SequenceMatchings);

  CorrelationMatrix *CCM = new CorrelationMatrix(getNumberOfClusters(), ST2->getNumberOfClusters());
  for (it = SequenceMatchings.begin(); it != SequenceMatchings.end(); it++)
  {
    TClusterScoreMap left  = it->first ;
    TClusterScoreMap::iterator it1;
    TClusterScoreMap right = it->first ;
    TClusterScoreMap::iterator it2;

    for (it1 = left.begin(); it1 != left.end(); it1++)
    {
      for (it2 = right.begin(); it2 != right.end(); it2++)
      {
        CCM->Hit(it1->first, it2->first);
      }
    }   
  }

  map<CID, CID>::iterator it2;
  for (it2=UniqueCorrelations.begin(); it2!=UniqueCorrelations.end(); it2++)
  {
    CCM->Hit(it2->first, it2->second);
  }

  CCM->ComputePcts();

  return CCM;
}

DoubleLink * SequenceTracker::PairWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2)
{
  map<TClusterScoreMap, TClusterScoreMap> SequenceMatchings;
  map<TClusterScoreMap, TClusterScoreMap>::iterator it;

  SS->TimeCorrelation(UniqueCorrelations, ST2->SS, SequenceMatchings);

  DoubleLink *PairedSequences = new DoubleLink();
  for (it = SequenceMatchings.begin(); it != SequenceMatchings.end(); it++)
  {
    TClusterScoreMap left  = it->first ;
    TClusterScoreMap::iterator it1;
    TClusterScoreMap right = it->first ;
    TClusterScoreMap::iterator it2;

    TClustersSet LeftGroup, RightGroup;
    for (it1 = left.begin(); it1 != left.end(); it1++)
    {
      CID ClusterID = it1->first; 
      LeftGroup.insert( ClusterID );
    }
    for (it2 = right.begin(); it2 != right.end(); it2++)
    {
      CID ClusterID = it2->first;
      RightGroup.insert( ClusterID );
    }
    PairedSequences->add( LeftGroup, RightGroup );
  }
  return PairedSequences;
}

DoubleLink * SequenceTracker::PairWithAnother(map<CID, CID> UniqueCorrelations, SequenceTracker *ST2, CID LastClusterTrace1, CID LastClusterTrace2)
{
  DoubleLink *PairedSequences = new DoubleLink();
/*
  map<CID, CID>::iterator it3;
  for (it3 = UniqueCorrelations.begin(); it3 != UniqueCorrelations.end(); ++it3)
  {
    TClustersSet LeftGroup, RightGroup;
    LeftGroup.insert(it3->first);
    RightGroup.insert(it3->second);
    PairedSequences->add( LeftGroup, RightGroup );
  }
*/

  vector< pair<TSequence, TSequence> > SubsequenceCombos;
  vector< pair<TSequence, TSequence> >::iterator it;

  SS->TimeCorrelation(UniqueCorrelations, ST2->SS, SubsequenceCombos);

  for (it = SubsequenceCombos.begin(); it != SubsequenceCombos.end(); ++it)
  {
    TSequence Seq1, Seq2, FilteredSeq1, FilteredSeq2;
    int size1, size2;
    Seq1 = it->first;
    Seq2 = it->second;
    size1 = FilterSubsequence(Seq1, LastClusterTrace1, FilteredSeq1);
    size2 = FilterSubsequence(Seq2, LastClusterTrace2, FilteredSeq2);

    if ((size1 == size2) && (size1 > 0))
    {
      /* DEBUG 
      cout << "SEQUENCE PAIR:\n";
      SS->PrintSequence(FilteredSeq1);
      cout << " vs " << endl;
      SS->PrintSequence(FilteredSeq2);
      cout << endl; */

      for (int i=0; i<size1; i++)
      {
        TClusterScoreMap left  = FilteredSeq1[i];
        TClusterScoreMap right = FilteredSeq2[i];

        TClusterScoreMap::iterator it1;
        TClusterScoreMap::iterator it2;
        TClustersSet LeftGroup, RightGroup;
        for (it1 = left.begin(); it1 != left.end(); it1++)
        {
          CID ClusterID = it1->first;
          LeftGroup.insert( ClusterID );
        }
        for (it2 = right.begin(); it2 != right.end(); it2++)
        { 
          CID ClusterID = it2->first;
          RightGroup.insert( ClusterID );
        }
        /* DEBUG
        cout << "***** ADDING ";
        PrintClusterGroup(cout, LeftGroup, ",");
        cout << " <-> ";
        PrintClusterGroup(cout, RightGroup, ",");
        cout << endl; */

        /* XXX Make add not to insert repeated links! */
        PairedSequences->add( LeftGroup, RightGroup );
      }
    }
  }
  return PairedSequences;
}

int SequenceTracker::FilterSubsequence(TSequence Seq, CID LastCluster, TSequence &FilteredSeq)
{
  FilteredSeq.clear();

  /* DEBUG 
  cout << "Last cluster = " << LastCluster << endl;
  cout << "Sequence size = " << Seq.size() << endl;
  cout << "[ ";
  for (int i=0; i<Seq.size(); i++)
  {

    TClusterScoreMap::iterator it;
    cout << "( ";
    for (it = Seq[i].begin(); it != Seq[i].end(); it++ )
    {
      cout << it->first << " ";
    }
    cout << ") ";
  }
  cout << "] " << endl; */

  for (int i=0; i<Seq.size(); i++)
  {
    TClusterScoreMap m1, m2;
    m1 = Seq[i];
    TClusterScoreMap::iterator it = m1.begin();
    for (it=m1.begin(); it!=m1.end(); ++it)
    {
      CID ClusterID = it->first;
      INT32 Score   = it->second;
      if (ClusterID <= LastCluster)
      {
        m2[ClusterID] = Score;
      }
    }

    if (m2.size() > 0)
    {
      FilteredSeq.push_back( m1 );
    }
  }

  /* DEBUG
  cout << "Filtered sequence size = " << FilteredSeq.size() << endl; */
  return FilteredSeq.size();
}

