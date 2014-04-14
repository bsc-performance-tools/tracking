#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
#include <string>
using std::string;
using std::make_pair;
#include <algorithm>
#include "Links.h"
#include "ClusterIDs.h"
#include "TraceReconstructor.h"


void PrintClusterGroup( ostream & channel, TClustersSet &CG, string delim )
{
    TClustersSet::iterator it;

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

int subset(TClustersSet &g1, TClustersSet &g2)
{
  TClustersSet *small, *big;
  TClustersSet::iterator it;

  if (g1.size() <= g2.size()) small = &g1, big = &g2;
  else small = &g2, big = &g1;

  for (it = small->begin(); it != small->end(); ++it)
  {
    if (big->find(*it) == big->end()) return 0;
  }

  return ((g1.size() < g2.size()) ? 1 : 2);
}

/**
 * Constructor for a 1-way link, e.g. 1 -> [1, 2]
 */
Link::Link(Cluster cluster_id)
{
  ClusterID = cluster_id;
}

/**
 * Returns how many clusters are connected to the current one
 */
int Link::size(void)
{
  return LinkedWith.size();
}

/**
 * Links a new cluster
 */
void Link::add(Cluster cluster_id)
{
  LinkedWith.insert(cluster_id);
}

/**
 * Prints the linked clusters
 */
void Link::print(void)
{
  cout << "Links for cluster " << ClusterID << " -> ";
  PrintClusterGroup( cout, LinkedWith, ",");
  cout << endl;
}

/**
 * Computes the union of two links, e.g.
 * 1 -> [1, 2]
 * 1 -> [3]
 * --------------
 * 1 -> [1, 2, 3]
 */
void Link::join(Link *another_link)
{
  TClustersSet::iterator it;
  for (it=another_link->begin(); it!=another_link->end(); it++)
  {
    LinkedWith.insert( *it );
  }
}

/** 
 * Computes the intersection of two links, e.g.
 * 1 -> [1, 2]
 * 1 -> [2, 3]
 * -----------
 * 1 -> [2]
 */
void Link::intersect(Link *another_link)
{
  TClustersSet intersection;
  std::insert_iterator< TClustersSet > ii (intersection, intersection.begin());

  std::set_intersection(
    LinkedWith.begin(), LinkedWith.end(),
    another_link->begin(), another_link->end(),
    ii);

  LinkedWith = intersection;
}

/**
 * Returns an iterator pointing to the first linked cluster
 */
TClustersSet::iterator Link::begin()
{
  return LinkedWith.begin();
}

/**
 * Returns an iterator pointing to the element past the last linked cluster
 */
TClustersSet::iterator Link::end()
{
  return LinkedWith.end();
}

/**
 * Returns the current cluster identifier, e.g.
 * 1 -> [3, 4, 5]
 * --------------
 * Returns: 1
 */
Cluster Link::get_Cluster()
{
  return ClusterID;
}

/**
 * Returns the set of clusters that are linked to the current one, e.g.
 * 1 -> 3, 4, 5
 * ------------------
 * Returns: [3, 4, 5]
 */
TClustersSet Link::get_Links()
{
  return LinkedWith;
}

/**
 * Assumes that the input vectors are ordered by the cluster number, so that first position in the 
 * array corresponds to the 1-way correlations of cluster 1 and so on
 */
DoubleLink::DoubleLink(vector<Link *> &Forward, vector<Link *> &Backward)
{
  for (unsigned int i=0; i<Forward.size(); i++)
  {
    TClustersSet LeftGroup, RightGroup;
    Cluster LeftClusterID = Forward[i]->get_Cluster();

    LeftGroup.insert( LeftClusterID );
    RightGroup = Forward[i]->get_Links();

    TClustersSet::iterator it1, it2;
    for (it1 = RightGroup.begin(); it1 != RightGroup.end(); it1 ++)
    {
      Cluster RightClusterID = *it1;
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
      Cluster RightClusterID = Backward[j]->get_Cluster();
      TClustersSet BackwardCorrelatingClusters = Backward[j]->get_Links();
      if (BackwardCorrelatingClusters.find( LeftClusterID ) != BackwardCorrelatingClusters.end())
      {
        RightGroup.insert( RightClusterID );
      }
    }

    add(LeftGroup, RightGroup);
  }

  Combine();
}

/**
 * Splits the rules of the current correlation with the more specialized rules of the correlation given by parameter, and returns 
 * a new correlation object that contains more specific rules.
 */
DoubleLink * DoubleLink::Split(DoubleLink *Specialized)
{
/*
  cout << "[DEBUG] DoubleLink::Split ====================================================\n";
  cout << "RULES:\n";
  print();
  cout << "SPECIALIZED:\n";
  Specialized->print();
  cout << "==============================================================================\n";
*/

  DoubleLink * res = NULL;
  res = new DoubleLink();

  vector<TLinkedGroups>::iterator it;
  vector<TLinkedGroups> tmp;

  for (it=begin(); it!=end(); ++it)
  {
    TClustersSet Left, Right;
    Left  = it->first;
    Right = it->second;

    vector<TLinkedGroups>::iterator it2;

    int over = 0;
    while (over < Specialized->size())
    {
        int min_size = -1;
	int min_index = -1;
	int cur_count = 0;
      
        for (it2=Specialized->begin(); it2!=Specialized->end(); ++it2)
        {
          int cur_size = 0;
          TClustersSet Left2, Right2;
          Left2  = it2->first;
          Right2 = it2->second;

	  if ((Left2.size() == 1) && ( Right2.size() == 1))
          {
		min_size = 2;
		min_index = cur_count;
		break;
          }
	  else
	  {
            cur_size = Left2.size() + Right2.size();
            if ((min_size == -1) || (cur_size > min_size))
	    {
		min_size = cur_size;
		min_index = cur_count;
	    }
	  }
	  cur_count ++;
        }

        TClustersSet Left2, Right2;
	Left2  = Specialized->Links[min_index].first;
	Right2 = Specialized->Links[min_index].second;

        if ( (subset(Left, Left2) == 2) ||(subset(Right, Right2) == 2) )
        {
          TClustersSet SubstractLeft, SubstractRight;
          set_difference(Left.begin(), Left.end(), Left2.begin(), Left2.end(),
            std::inserter(SubstractLeft, SubstractLeft.end()));
          set_difference(Right.begin(), Right.end(), Right2.begin(), Right2.end(),
            std::inserter(SubstractRight, SubstractRight.end()));
          Left = SubstractLeft;
          Right = SubstractRight;

          tmp.push_back( make_pair( Left2, Right2 ) );
        }
	over++;
    }
#if 0
    for (it2=Specialized->begin(); it2!=Specialized->end(); ++it2)
    {
      TClustersSet Left2, Right2;
      Left2  = it2->first;
      Right2 = it2->second;

      if ( (subset(Left, Left2) == 2) ||(subset(Right, Right2) == 2) )
      {
        TClustersSet SubstractLeft, SubstractRight;
        set_difference(Left.begin(), Left.end(), Left2.begin(), Left2.end(), 
          std::inserter(SubstractLeft, SubstractLeft.end()));
        set_difference(Right.begin(), Right.end(), Right2.begin(), Right2.end(), 
          std::inserter(SubstractRight, SubstractRight.end()));
        Left = SubstractLeft; 
        Right = SubstractRight;
	    
        tmp.push_back( make_pair( Left2, Right2 ) );
      }
    }
#endif
    if ((Left.size() > 0) || (Right.size() > 0))
    {
      tmp.push_back( make_pair( Left, Right ) );
    }
  }  
  res->add(tmp);

  res->Combine();

  return res;
}

DoubleLink::DoubleLink()
{
  Links.clear();
}

int DoubleLink::size()
{
  return Links.size();
}

void DoubleLink::add(TClustersSet LeftGroup, TClustersSet RightGroup)
{
  /* This loop checks for any pre-existing link with the same cluster groups, not to insert repeated links */
  for (int i=0; i<Links.size(); i++)
  {
    if ((Links[i].first == LeftGroup) && (Links[i].second == RightGroup))
    {
      return;
    }
  } 
  TLinkedGroups p = make_pair(LeftGroup, RightGroup);
  Links.push_back(p);
}

/* Sorts the given links by the lowest first cluster in the left set */
void DoubleLink::add(vector<TLinkedGroups> Input)
{
  vector<TLinkedGroups> UnsortedGroups = Input;

  while (UnsortedGroups.size() > 0)
  {
    int i = 0;
    int min_index = 0;
    int min_cluster = -1;

    for (i=0; i<UnsortedGroups.size(); i++)
    {
      TLinkedGroups CurrentLink;
      CurrentLink = UnsortedGroups[i];
      TClustersSet Left;
      Left = CurrentLink.first;

      if (Left.size() > 0)
      {
        Cluster first_cluster = *(Left.begin());
        if ((min_cluster < 0) || (first_cluster < min_cluster))
        {
          min_cluster = first_cluster;
          min_index = i;
        }
      }
    }
    add(UnsortedGroups[min_index].first, UnsortedGroups[min_index].second);
    UnsortedGroups.erase(UnsortedGroups.begin() + min_index);  
  }
}

vector<TLinkedGroups>::iterator DoubleLink::begin()
{
  return Links.begin();
}

vector<TLinkedGroups>::iterator DoubleLink::end()
{
  return Links.end();
}

void DoubleLink::print()
{
  vector< TLinkedGroups >::iterator it;
  for (it = Links.begin(); it != Links.end(); it ++)
  {
    TClustersSet LeftGroup, RightGroup;
    LeftGroup  = it->first;
    RightGroup = it->second;

    PrintClusterGroup( cout, LeftGroup, "," );
    cout << " <-> ";
    PrintClusterGroup( cout, RightGroup, "," );

    cout << endl;
  }
}

void DoubleLink::Combine()
{
  vector< TLinkedGroups >::iterator it1, it2;

  it1 = Links.begin();
  while (it1 != Links.end())
  {
    TClustersSet LeftGroup1, RightGroup1;
    TClustersSet LeftGroup2, RightGroup2;

    it2 = it1 + 1;

    while (it2 != Links.end())
    {
      TClustersSet isecLeft, isecRight;

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
TClustersSet DoubleLink::FindLink(TClustersSet &LeftGroup)
{
  vector< TLinkedGroups >::iterator it;
  vector< vector< TLinkedGroups >::iterator > Candidates;
  unsigned int CandidatesMaxSetSize = 0;

  for (it = begin(); it != end(); it ++)
  {
    TClustersSet Left, Right;
    Left  = it->first;
    Right = it->second;

    if (Left == LeftGroup)
    {
      return Right;
    }
    else
    {
      TClustersSet isec;
      set_intersection( Left.begin(), Left.end(), LeftGroup.begin(), LeftGroup.end(), inserter(isec, isec.begin()) );
      if (isec.size() > 0)
      {
#if 0
        /* If there are multiple links, returns the one that intersects more with the input set */
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
#else
        /* Joins all possible candidates */
        Candidates.push_back( it );
#endif
      }
    }
  }

  TClustersSet CandidatesAdded;

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
        TClustersSet::iterator it2;
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

void DoubleLink::GetUnique(map<Cluster, Cluster> &UniqueCorrelations)
{
  vector< TLinkedGroups >::iterator it;

  for (it = Links.begin(); it != Links.end(); it ++)
  {
    TClustersSet LeftGroup, RightGroup;
    LeftGroup  = it->first;
    RightGroup = it->second;
    if ((LeftGroup.size() == 1) && (RightGroup.size() == 1))
    {
      Cluster Left  = *(LeftGroup.begin());
      Cluster Right = *(RightGroup.begin());

      UniqueCorrelations[Left] = Right;
    }
  }
}

SequenceLink::SequenceLink(vector<DoubleLink *> &AllPairs)
{
  int count = 0;
  vector<TLinkedGroups>::iterator it;

  DoubleLink *First = AllPairs[0];

  First->print();

  for (it = First->begin(); it != First->end(); it ++)
  {
    count ++;
    /* DEBUG */ 
    cout << "SEQUENCE " << count << " ";

    TClustersSet Left, Right, Next;
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

void SequenceLink::UnifySequences(int s1, int s2)
{
  /* Insert s2 elements in s1 */
  for (int i=0; i<FinalSequence[s2].size(); i++)
  {
    TClustersSet CG = FinalSequence[s2][i];
    TClustersSet::iterator it;
    for (it=CG.begin(); it!=CG.end(); it++)
    {
      FinalSequence[s1][i].insert(*it);
    }
  }
  /* Delete s2 */
  FinalSequence.erase(FinalSequence.begin() + s2);
}

void SequenceLink::UnifySequences()
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
        TClustersSet isec;
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

void SequenceLink::write(ostream &Channel)
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

void SequenceLink::write(ostream &Channel, TClusterSequence &Sequence, string Delimiter)
{
  for (int i=0; i<Sequence.size(); i++)
  {
    PrintClusterGroup( Channel, Sequence[i], Delimiter );
    if (i < Sequence.size()-1) Channel << " <-> ";
    else Channel << endl;
  }
}

int SequenceLink::GetTranslationTable(int trace, int total_clusters, map< TTypeValuePair, TTypeValuePair > &TranslationTable)
{
  TranslationTable.clear();

  /* Set all clusters translating to noise by default */
  for (int i=FIRST_CLUSTER; i<total_clusters+FIRST_CLUSTER; i++)
  {
    TTypeValuePair p1 = make_pair(CLUSTER_EV, i);
    TTypeValuePair p2 = make_pair(CLUSTER_EV, NOISE);
    TranslationTable[p1] = p2;
  }

  /* Overwrite the tracked clusters with the appropriate translation */
  for (int i=0; i<FinalSequence.size(); i++)
  {
    TClusterSequence CurrentSequence = FinalSequence[i];

    TClustersSet OldClusters = CurrentSequence[trace];

    TClustersSet::iterator it;
    for (it = OldClusters.begin(); it != OldClusters.end(); it ++)
    {
      TTypeValuePair p1 = make_pair(CLUSTER_EV, *it + FIRST_CLUSTER - 1);
      TTypeValuePair p2 = make_pair(CLUSTER_EV, i + FIRST_CLUSTER);

//      if (p1.second != p2.second)
//      {
        TranslationTable[p1] = p2;
//      }
    }
  }
  return TranslationTable.size();
}

