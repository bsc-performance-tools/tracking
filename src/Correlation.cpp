#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
#include <string>
using std::string;
using std::make_pair;
#include "Correlation.h"
#include "ClusterIDs.h"
#include "TraceReconstructor.h"

void PrintClusterGroup( ostream & channel, TClusterGroup &CG, string delim )
{
    TClusterGroup::iterator it;

    int count = 0;
    for (it=CG.begin(); it!=CG.end(); it++)
    {
        channel << *it;
        if (count < CG.size() - 1) channel << delim;
        count ++;
    }
    if (count == 0)
    {
      channel << "x";
    }
}

OneWayCorrelation::OneWayCorrelation(CID cluster_id)
{
  ClusterID = cluster_id;
}

int OneWayCorrelation::size()
{
  return CorrelatingClusters.size();
}

void OneWayCorrelation::add(CID cluster_id)
{
  CorrelatingClusters.insert(cluster_id);
}

void OneWayCorrelation::print()
{
  cout << ClusterID << " -> ";
  PrintClusterGroup( cout, CorrelatingClusters, ",");
  cout << endl;
}

void OneWayCorrelation::join(OneWayCorrelation *AnotherCorrelation)
{
  TClusterGroup::iterator it;
  for (it=AnotherCorrelation->begin(); it!=AnotherCorrelation->end(); it++)
  {
    CorrelatingClusters.insert( *it );
  }
}

void OneWayCorrelation::intersect(OneWayCorrelation *AnotherCorrelation)
{
  TClusterGroup IntersectGroup;
  std::insert_iterator< TClusterGroup > ii (IntersectGroup, IntersectGroup.begin());

  std::set_intersection(CorrelatingClusters.begin(), CorrelatingClusters.end(),
    AnotherCorrelation->CorrelatingClusters.begin(), AnotherCorrelation->CorrelatingClusters.end(),
    ii);

  CorrelatingClusters = IntersectGroup;
}

TClusterGroup::iterator OneWayCorrelation::begin()
{
  return CorrelatingClusters.begin();
}

TClusterGroup::iterator OneWayCorrelation::end()
{
  return CorrelatingClusters.end();
}

CID OneWayCorrelation::ID()
{
  return ClusterID;
}

TClusterGroup OneWayCorrelation::CorrelatesTo()
{
  return CorrelatingClusters;
}

/**
 * Assumes that the input vectors are ordered by the cluster number, so that first position in the 
 * array corresponds to the 1-way correlations of cluster 1 and so on
 */
TwoWayCorrelation::TwoWayCorrelation(vector<OneWayCorrelation *> &Forward, vector<OneWayCorrelation *> &Backward)
{
  for (unsigned int i=0; i<Forward.size(); i++)
  {
    TClusterGroup LeftGroup, RightGroup;
    CID LeftClusterID = Forward[i]->ID();

    LeftGroup.insert( LeftClusterID );
    RightGroup = Forward[i]->CorrelatesTo();

    TClusterGroup::iterator it1, it2;
    for (it1 = RightGroup.begin(); it1 != RightGroup.end(); it1 ++)
    {
      CID RightClusterID = *it1;
      if (RightClusterID <= Backward.size())
      {
        for (it2 = Backward[RightClusterID-1]->begin(); it2 != Backward[RightClusterID-1]->end(); it2 ++)
        {
          LeftGroup.insert( *it2 );
        }
      }
    }

    for (unsigned int j=0; j<Backward.size(); j++)
    {
      CID RightClusterID = Backward[j]->ID();
      TClusterGroup BackwardCorrelatingClusters = Backward[j]->CorrelatesTo();
      if (BackwardCorrelatingClusters.find( LeftClusterID ) != BackwardCorrelatingClusters.end())
      {
        RightGroup.insert( RightClusterID );
      }
    }

    add(LeftGroup, RightGroup);
  }

  Combine();
}

TwoWayCorrelation::TwoWayCorrelation()
{
  Links.clear();
}

void TwoWayCorrelation::add(TClusterGroup LeftGroup, TClusterGroup RightGroup)
{
  TLinkedGroups p = make_pair(LeftGroup, RightGroup);
  Links.push_back(p);
}

vector<TLinkedGroups>::iterator TwoWayCorrelation::begin()
{
  return Links.begin();
}

vector<TLinkedGroups>::iterator TwoWayCorrelation::end()
{
  return Links.end();
}

void TwoWayCorrelation::print()
{
  vector< TLinkedGroups >::iterator it;
  for (it = Links.begin(); it != Links.end(); it ++)
  {
    TClusterGroup LeftGroup, RightGroup;
    LeftGroup  = it->first;
    RightGroup = it->second;

    PrintClusterGroup( cout, LeftGroup, "," );
    cout << " <-> ";
    PrintClusterGroup( cout, RightGroup, "," );

    cout << endl;
  }
}

void TwoWayCorrelation::Combine()
{
  vector< TLinkedGroups >::iterator it1, it2;

  it1 = Links.begin();
  while (it1 != Links.end())
  {
    TClusterGroup LeftGroup1, RightGroup1;
    TClusterGroup LeftGroup2, RightGroup2;

    it2 = it1 + 1;

    while (it2 != Links.end())
    {
      TClusterGroup isecLeft, isecRight;

      LeftGroup1  = it1->first;
      RightGroup1 = it1->second;
      LeftGroup2  = it2->first;
      RightGroup2 = it2->second;
       
      /* DEBUG 
      PrintClusterGroup ( cout, LeftGroup1, "," );
      cout << " vs ";
      PrintClusterGroup ( cout, LeftGroup2, "," );
      cout << " AND ";
      PrintClusterGroup ( cout, RightGroup1, "," );
      cout << " vs ";
      PrintClusterGroup ( cout, RightGroup2, "," ); 
      cout << endl; */

      set_intersection( LeftGroup1.begin(), LeftGroup1.end(), LeftGroup2.begin(), LeftGroup2.end(), inserter(isecLeft, isecLeft.begin()) );
      set_intersection( RightGroup1.begin(), RightGroup1.end(), RightGroup2.begin(), RightGroup2.end(), inserter(isecRight, isecRight.begin()) );

      if ((isecLeft.size() > 0) || (isecRight.size() > 0))
      {
         set_union( LeftGroup1.begin(), LeftGroup1.end(), LeftGroup2.begin(), LeftGroup2.end(), inserter(it1->first, it1->first.begin()) );
         set_union( RightGroup1.begin(), RightGroup1.end(), RightGroup2.begin(), RightGroup2.end(), inserter(it1->second, it1->second.begin()) );
         Links.erase(it2);
         it2 = it1 + 1;
      }
      else
      {
        it2 ++;
      }
    }
    it1 ++;
  }
}


/**
 * Return the right association in a correlation.
 * Example: (1,2) <-> (3,4) ... returns (3,4) for (1,2)
 */
TClusterGroup TwoWayCorrelation::FindLink(TClusterGroup &LeftGroup)
{
  vector< TLinkedGroups >::iterator it;
  vector< vector< TLinkedGroups >::iterator > Candidates;
  unsigned int CandidatesMaxSetSize = 0;

  for (it = begin(); it != end(); it ++)
  {
    TClusterGroup Left, Right;
    Left  = it->first;
    Right = it->second;

    if (Left == LeftGroup)
    {
      return Right;
    }
    else
    {
      TClusterGroup isec;
      set_intersection( Left.begin(), Left.end(), LeftGroup.begin(), LeftGroup.end(), inserter(isec, isec.begin()) );
      if (isec.size() > 0)
      {
        if (isec.size() > CandidatesMaxSetSize)
        {
          CandidatesMaxSetSize = isec.size();
          Candidates.clear();
          Candidates.push_back( it );
        }
        else if (isec.size() == CandidatesMaxSetSize)
        {
          Candidates.push_back( it );
        }
      }
    }
  }

  TClusterGroup CandidatesAdded;

  if (Candidates.size() > 1)
  {
    cerr << "Multiple links (" << Candidates.size() << ") for cluster group (";
    PrintClusterGroup( cerr, LeftGroup, "," );
    cerr << ") : ";
    for (int i=0; i<Candidates.size(); i++)
    {
      cerr << "(";
      PrintClusterGroup( cerr, Candidates[i]->second, "," );
      cerr << ") ";
    }
    cerr << endl;

    for (int i=0; i<Candidates.size(); i++)
    {
//    if (Candidates[i]->first.size() == CandidatesMaxSetSize)
      {
        TClusterGroup::iterator it2;
        for (it2=Candidates[i]->second.begin(); it2!=Candidates[i]->second.end(); it2++)
        {
          CandidatesAdded.insert( *it2 );
        }
      }
    }
  }
  else if (Candidates.size() == 1)
  {
    CandidatesAdded = Candidates[0]->second;
  }

  return CandidatesAdded;
}

void TwoWayCorrelation::GetUnique(map<CID, CID> &UniqueCorrelations)
{
  vector< TLinkedGroups >::iterator it;

  for (it = Links.begin(); it != Links.end(); it ++)
  {
    TClusterGroup LeftGroup, RightGroup;
    LeftGroup  = it->first;
    RightGroup = it->second;
    if ((LeftGroup.size() == 1) && (RightGroup.size() == 1))
    {
      CID Left  = *(LeftGroup.begin());
      CID Right = *(RightGroup.begin());

      UniqueCorrelations[Left] = Right;
    }
  }
}

NWayCorrelation::NWayCorrelation(vector<TwoWayCorrelation *> &AllPairs)
{
  int count = 0;
  vector<TLinkedGroups>::iterator it;

  TwoWayCorrelation *First = AllPairs[0];
  for (it = First->begin(); it != First->end(); it ++)
  {
    count ++;
    /* DEBUG */ 
    cout << "SEQUENCE " << count << " ";

    TClusterGroup Left, Right, Next;
    TClusterSequence CurrentSequence;

    Left  = it->first;
    Right = it->second;
    //if ((Left.size() == 0) || (Right.size() == 0)) continue;

    CurrentSequence.push_back(Left);
    CurrentSequence.push_back(Right);

    Left = Right;

    for (int i=1; i<AllPairs.size(); i++)
    {
      Next = AllPairs[i]->FindLink( Left );
      if (Next.size() == 0)
      {
        cerr << endl << "*** TRACKER IS LOST! ***" << endl;
        cerr << "Could not find the next correlation for cluster group (";
        PrintClusterGroup( cerr, Left, "," );
        cerr << ") at step " << i << endl << endl;
//        exit(EXIT_FAILURE);
      }
      CurrentSequence.push_back(Next);
      Left = Next;
    }
    FinalSequence.push_back(CurrentSequence);

    /* DEBUG */
    write(cout, CurrentSequence, ",");
  }
  UnifySequences();
}

void NWayCorrelation::UnifySequences(int s1, int s2)
{
  /* Insert s2 elements in s1 */
  for (int i=0; i<FinalSequence[s2].size(); i++)
  {
    TClusterGroup CG = FinalSequence[s2][i];
    TClusterGroup::iterator it;
    for (it=CG.begin(); it!=CG.end(); it++)
    {
      FinalSequence[s1][i].insert(*it);
    }
  }
  /* Delete s2 */
  FinalSequence.erase(FinalSequence.begin() + s2);
}

void NWayCorrelation::UnifySequences()
{
  int SequenceLength = FinalSequence[0].size();
  int i=0, j=0, k=0;

  while (i < FinalSequence.size())
  {
    j = i+1;
    while (j < FinalSequence.size())
    {
      for (k=0; k<SequenceLength; k++)
      {
#if 1
        TClusterGroup isec;
        set_intersection( FinalSequence[i][k].begin(), FinalSequence[i][k].end(), FinalSequence[j][k].begin(), FinalSequence[j][k].end(), inserter(isec, isec.begin()) );
        if (isec.size() > 0)
#else
        if (FinalSequence[i][k] == FinalSequence[j][k])
#endif
        {
          /* DEBUG 
          cout << "Unify sequence " << i+1 << " and " << j+1 << " (column " << k+1 << ")   ";
          PrintClusterGroup( cout, FinalSequence[i][k], "," );
          cout << endl; */

          UnifySequences(i, j);

          j --;
          break;
        }
      }
      j++;
    }
    i++;
  }

  /* DEBUG */
  cout << endl << "+ Unifying sequences that share clusters..." << endl << endl;
  write(cout);
  cout << endl;
}

void NWayCorrelation::write(ostream &Channel)
{
  for (int i=0; i<FinalSequence.size(); i++)
  {
    TClusterSequence CurrentSequence = FinalSequence[i];
    for (int j=0; j<CurrentSequence.size(); j++)
    {
      PrintClusterGroup( Channel, CurrentSequence[j], "," );
      if (j < CurrentSequence.size()-1) Channel << " <-> ";
      else Channel << endl;
    }
  }
}

void NWayCorrelation::write(ostream &Channel, TClusterSequence &Sequence, string Delimiter)
{
  for (int i=0; i<Sequence.size(); i++)
  {
    PrintClusterGroup( Channel, Sequence[i], Delimiter );
    if (i < Sequence.size()-1) Channel << " <-> ";
    else Channel << endl;
  }
}

int NWayCorrelation::GetTranslationTable(int trace, map< TTypeValuePair, TTypeValuePair > &TranslationTable)
{
  TranslationTable.clear();

  for (int i=0; i<FinalSequence.size(); i++)
  {
    TClusterSequence CurrentSequence = FinalSequence[i];

    TClusterGroup OldClusters = CurrentSequence[trace];
    TClusterGroup::iterator it;
    for (it = OldClusters.begin(); it != OldClusters.end(); it ++)
    {
      TTypeValuePair p1 = make_pair(CLUSTER_EV, *it + FIRST_CLUSTER);
      TTypeValuePair p2 = make_pair(CLUSTER_EV, i+1+FIRST_CLUSTER);

      if (p1.second != p2.second)
      {
        TranslationTable[p1] = p2;
      }
    }
  }
  return TranslationTable.size();
}

