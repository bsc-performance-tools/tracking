#include <algorithm>
#include <iostream>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::make_pair;
using std::ostream;
using std::string;

#include "ClusterIDs.h"
#include "Links.h"
#include "TraceReconstructor.h"

void print_objects_set( ostream & channel, ObjectSet_t &CG, string delim )
{
  ObjectSet_t::iterator it;

  int count = 0;
  for (it=CG.begin(); it!=CG.end(); it++)
  {
    channel << *it;
    if (count < CG.size() - 1) 
    {
      channel << delim;
    }
    count ++;
  }
  if (count == 0)
  {
    channel << "x";
  }
}

int subset(ObjectSet_t &g1, ObjectSet_t &g2)
{
  ObjectSet_t         *small;
  ObjectSet_t         *big;
  ObjectSet_iterator_t it;

  if (g1.size() <= g2.size()) 
  {
    small = &g1;
    big   = &g2;
  }
  else {
    small = &g2;
    big   = &g1;
  }

  for (it = small->begin(); it != small->end(); ++it)
  {
    if (big->find(*it) == big->end()) return 0;
  }

  return ((g1.size() < g2.size()) ? 1 : 2);
}


ObjectLinks::ObjectLinks(ClusterID_t cluster_id)
{
  ObjectID = cluster_id;
  clear();
}

ObjectLinks::~ObjectLinks()
{
  ObjectID = 0;
  clear();
}

void ObjectLinks::clear()
{
  Links.clear();
}

int ObjectLinks::size()
{
  return Links.size();
}

void ObjectLinks::add(ClusterID_t cluster_id)
{
  Links.insert(cluster_id);
}


ObjectLinks_iterator_t ObjectLinks::begin()
{
  return Links.begin();
}

ObjectLinks_iterator_t ObjectLinks::end()
{
  return Links.end();
}

void ObjectLinks::print()
{
  cout << ObjectID << " -> ";
  print_objects_set( cout, Links, ", " );
  cout << endl;
}

/**
 * Computes the union of two links, e.g.
 * 1 -> [1, 2]
 * 1 -> [3]
 * --------------
 * 1 -> [1, 2, 3]
 */
void ObjectLinks::join(ObjectLinks *object2)
{
  ObjectLinks_iterator_t it;
  for (it = object2->begin(); it != object2->end(); ++ it)
  {
    Links.insert( *it );
  }
}

/** 
 * Computes the intersection of two links, e.g.
 * 1 -> [1, 2]
 * 1 -> [2, 3]
 * -----------
 * 1 -> [2]
 * If current link is empty, does the union instead.
 */
void ObjectLinks::intersect(ObjectLinks *object2)
{
  if (size() > 0)
  {
    ObjectSet_t intersection;
    std::insert_iterator< ObjectSet_t > ii (intersection, intersection.begin());

    std::set_intersection(
      this->begin(), this->end(),
      object2->begin(), object2->end(),
      ii);

    this->Links = intersection;
  }
  else
  {
    this->join( object2 );
  }
}

ClusterID_t ObjectLinks::get_object()
{
  return ObjectID;
}

ObjectSet_t ObjectLinks::get_links()
{
  return Links;
}

FrameLinks::FrameLinks()
{
  clear();
}

FrameLinks::~FrameLinks()
{
  clear();
}

void FrameLinks::clear()
{
  Links.clear();
}

void FrameLinks::add( ObjectLinks *object_links )
{
  Links[ object_links->get_object() ] = object_links->get_links();
}

void FrameLinks::remove( ClusterID_t cluster_id )
{
  Links.erase( cluster_id );
}

void FrameLinks::print( )
{
  FrameLinks_t::iterator it;
  for (it=Links.begin(); it!=Links.end(); ++it)
  {
    int from_cluster = it->first;
    ObjectSet_t to_clusters = it->second;

    cout << "   [ " << from_cluster << " -> ";
    print_objects_set( cout, to_clusters, ", ");
    cout << " ]" << endl;
  }
}

int FrameLinks::size()
{
  return Links.size();
}

ObjectSet_t FrameLinks::get_links( ClusterID_t object_id )
{
  if (Links.find( object_id ) != Links.end())
  {
    return Links[object_id];
  }
  else
  {
    return ObjectSet_t();
  }
}

FrameLinks_t::iterator FrameLinks::begin()
{
  return Links.begin();
}

FrameLinks_t::iterator FrameLinks::end()
{
  return Links.end();
}

DoubleLinks::DoubleLinks(FrameLinks *Frame1, FrameLinks *Frame2)
{
  Links.clear();

  FrameLinks_iterator_t it;
  for (it = Frame1->begin(); it != Frame1->end(); ++ it)
  {
    ClusterID_t ObjAtFrame1, ObjAtFrame2;
    ObjectSet_t ForwardLinks, BackwardLinks;
    ObjectSet_t LeftGroup, RightGroup;

    ObjAtFrame1  = it->first;
    ForwardLinks = it->second;

    LeftGroup.insert( ObjAtFrame1 );
    
    ObjectLinks_iterator_t it2;
    for (it2 = ForwardLinks.begin(); it2 != ForwardLinks.end(); ++ it2)
    {
      ObjAtFrame2 = *it2;
      if (ObjAtFrame2 <= Frame2->size())
      {
        BackwardLinks = Frame2->get_links( ObjAtFrame2 );

        ObjectLinks_iterator_t it3;
        for (it3 = BackwardLinks.begin(); it3 != BackwardLinks.end(); ++ it3 )
        {
          ClusterID_t BackwardObjAtFrame1 = *it3;
          LeftGroup.insert( BackwardObjAtFrame1 );
        }
      }
    }

   FrameLinks_iterator_t it4;
    for (it4 = Frame2->begin(); it4 != Frame2->end(); ++ it4 )
    {
      ObjAtFrame2 = it4->first;
      BackwardLinks = it4->second;

      if (BackwardLinks.find( ObjAtFrame1 ) != BackwardLinks.end())
      {
        RightGroup.insert( ObjAtFrame2 );
      }
    }
      
    add( LeftGroup, RightGroup );
  } 

  Merge(); 
}

DoubleLinks::~DoubleLinks()
{
  Links.clear();
}

/**
 * Splits the rules of the current correlation with the more specialized rules of the correlation given by parameter, and returns 
 * a new correlation object that contains more specific rules.
 */
DoubleLinks * DoubleLinks::Split(DoubleLinks *Specialized)
{
  /* DEBUG 
  cout << "[DEBUG] DoubleLinks::Split ====================================================\n";
  cout << "RULES:\n";
  print();
  cout << "SPECIALIZED:\n";
  Specialized->print();
  cout << "==============================================================================\n"; */

  DoubleLinks *res = new DoubleLinks();
  DoubleLinks_iterator_t it;

  for (it=begin(); it!=end(); ++it)
  {
    ObjectSet_t Left  = GetFrom(it);
    ObjectSet_t Right = GetTo(it);

    /* DEBUG
    cout << "Considering ground rule ";
    print_objects_set(cout, Left, ", ");
    cout << " -> ";
    print_objects_set(cout, Right, ", ");
    cout << endl; */

    DoubleLinks_iterator_t it2;
    it2 = Specialized->begin();
    while (it2 != Specialized->end()) 
    {
      ObjectSet_t Left2    = Specialized->GetFrom(it2);
      ObjectSet_t Right2   = Specialized->GetTo(it2);

      /* DEBUG
      cout << "--- Checking for possible specializations using rule ";
      print_objects_set(cout, Left2, ", ");
      cout << " -> ";
      print_objects_set(cout, Right2, ", ");
      cout << endl; */

      if ( (subset(Left, Left2) == 2) || (subset(Right, Right2) == 2) )
      {
        ObjectSet_t SubstractLeft, SubstractRight;

        set_difference(Left.begin(), Left.end(), Left2.begin(), Left2.end(),
          std::inserter(SubstractLeft, SubstractLeft.end()));
         
        set_difference(Right.begin(), Right.end(), Right2.begin(), Right2.end(),
          std::inserter(SubstractRight, SubstractRight.end()));

        Left  = SubstractLeft;
        Right = SubstractRight;

        /* DEBUG 
        cout << "--- Found an specialization, ground rule reduced to ";
        print_objects_set(cout, Left, ", ");
        cout << " -> ";
        print_objects_set(cout, Right, ", ");
        cout << endl; */

        res->add( Left2, Right2 );
        it2 = Specialized->begin();
      }
      else
      {
        /* DEBUG
        cout << "--- No possible specializations found" << endl; */
        it2 ++;
      }
    }
    if ((Left.size() > 0) || (Right.size() > 0))
    {
      res->add( Left, Right );
    }
  }  
  res->Sort();

  res->Merge();

  return res;
}

DoubleLinks::DoubleLinks()
{
  Links.clear();
}

int DoubleLinks::size()
{
  return Links.size();
}


void DoubleLinks::add(ObjectSet_t LeftGroup, ObjectSet_t RightGroup)
{
  /* This loop checks for any pre-existing link with the same cluster groups, not to insert repeated links */
  for (int i=0; i<Links.size(); i++)
  {
    if ((Links[i].first == LeftGroup) && (Links[i].second == RightGroup))
    {
      return;
    }
  } 
  Links.push_back( make_pair( LeftGroup, RightGroup ) );
}

/* Sorts the given links by the lowest object ID in the left set */

void DoubleLinks::Sort(void)
{
  DoubleLinks_t Sorted;
  DoubleLinks_t Unsorted = Links;
  DoubleLinks_iterator_t it;
  
  /* DEBUG -- rules before sorting
  print(); */

  while (Unsorted.size() > 0)
  {
    ClusterID_t min_cluster = -1;
    DoubleLinks_iterator_t min_index;
    
    for (it = Unsorted.begin(); it != Unsorted.end(); ++ it)
    {
      ObjectSet_t Left = GetFrom( it );

      /* DEBUG
      print_objects_set(cerr, Left, ", " );
      cerr << endl; */

      if (Left.size() > 0)
      {
        ObjectSet_iterator_t it2;
        for (it2 = Left.begin(); it2 != Left.end(); ++ it2)
        { 
          ClusterID_t cluster_id = *it2;

          if ((min_cluster < 0) || (cluster_id < min_cluster))
          {
            min_cluster = cluster_id;
            min_index   = it;
          }
        }
      }
      else if (min_cluster < 0)
      {
        min_index = it;
      }
    }
    //cerr << "Unsorted.size=" << Unsorted.size() << " min_cluster=" << min_cluster << endl;
    Sorted.push_back( make_pair(GetFrom(min_index), GetTo(min_index)) );

    Unsorted.erase( min_index );
  }

  Links = Sorted;
}

DoubleLinks_iterator_t DoubleLinks::begin()
{
  return Links.begin();
}

DoubleLinks_iterator_t DoubleLinks::end()
{
  return Links.end();
}

void DoubleLinks::print()
{
  DoubleLinks_iterator_t it;

  for (it = Links.begin(); it != Links.end(); it ++)
  {
    ObjectSet_t LeftGroup, RightGroup;
    LeftGroup  = it->first;
    RightGroup = it->second;

    cout << "   [ ";
    print_objects_set( cout, LeftGroup, "," );
    cout << " <-> ";
    print_objects_set( cout, RightGroup, "," );
    cout << " ]" << endl;
  }
}

DoubleLinks * DoubleLinks::Reverse()
{
  DoubleLinks *Reversed = new DoubleLinks();
  DoubleLinks_iterator_t it;

  for (it = begin(); it != end(); ++it)
  {
    Reversed->add(it->second, it->first);
  }
  return Reversed;
}

void DoubleLinks::Merge()
{
  DoubleLinks_iterator_t it1, it2;

  it1 = Links.begin();
  while (it1 != Links.end())
  {
    ObjectSet_t LeftGroup1, RightGroup1;
    ObjectSet_t LeftGroup2, RightGroup2;

    it2 = it1 + 1;

    while (it2 != Links.end())
    {
      ObjectSet_t isecLeft, isecRight;

      LeftGroup1  = it1->first;
      RightGroup1 = it1->second;
      LeftGroup2  = it2->first;
      RightGroup2 = it2->second;
       
      /* DEBUG 
      print_objects_set ( cout, LeftGroup1, "," );
      cout << " vs ";
      print_objects_set ( cout, LeftGroup2, "," );
      cout << " AND ";
      print_objects_set ( cout, RightGroup1, "," );
      cout << " vs ";
      print_objects_set ( cout, RightGroup2, "," ); 
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
ObjectSet_t DoubleLinks::FindLink(ObjectSet_t &LeftGroup)
{
  DoubleLinks_iterator_t it;
  vector< DoubleLinks_iterator_t > Candidates;
  unsigned int CandidatesMaxSetSize = 0;

  for (it = begin(); it != end(); it ++)
  {
    ObjectSet_t Left, Right;
    Left  = it->first;
    Right = it->second;

    if (Left == LeftGroup)
    {
      return Right;
    }
    else
    {
      ObjectSet_t isec;
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

  ObjectSet_t CandidatesAdded;

  if (Candidates.size() > 1)
  {
    cerr << "Multiple links (" << Candidates.size() << ") for cluster group (";
    print_objects_set( cerr, LeftGroup, "," );
    cerr << ") : ";
    for (int i=0; i<Candidates.size(); i++)
    {
      cerr << "(";
      print_objects_set( cerr, Candidates[i]->second, "," );
      cerr << ") ";
    }
    cerr << endl;

    for (int i=0; i<Candidates.size(); i++)
    {
//    if (Candidates[i]->first.size() == CandidatesMaxSetSize)
      {
        ObjectSet_t::iterator it2;
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

DoubleLinks * DoubleLinks::GetUnivocal()
{
  DoubleLinks *UnivocalLinks = new DoubleLinks();
  DoubleLinks_iterator_t it;

  for (it = begin(); it != end(); it ++)
  {
    ObjectSet_t LeftGroup, RightGroup;
    LeftGroup  = GetFrom(it);
    RightGroup = GetTo(it);

    if ((LeftGroup.size() == 1) && (RightGroup.size() == 1))
    {
      UnivocalLinks->add( LeftGroup, RightGroup );
    }
  }
  return UnivocalLinks;
}

DoubleLinks_iterator_t DoubleLinks::find( ObjectSet_t from )
{
  DoubleLinks_iterator_t it;

  for (it = begin(); it != end(); ++ it)
  {
    if (GetFrom(it) == from)
    {
      return it;
    }	    
  }
  return end();
}

ObjectSet_t DoubleLinks::GetFrom( DoubleLinks_iterator_t it )
{
  return it->first;
}

ObjectSet_t DoubleLinks::GetTo( DoubleLinks_iterator_t it )
{
  return it->second;
}

ObjectSet_t DoubleLinks::GetFrom( int index )
{
  if ((index >= Links.size()) || (index < 0))
  {
    return ObjectSet_t();
  }
  else
  {
    return Links[index].first;
  }
}

ObjectSet_t DoubleLinks::GetTo( int index )
{
  return Links[index].second;
}


ObjectSet_t DoubleLinks::get_links( ObjectSet_t from )
{
  DoubleLinks_iterator_t it = find(from);

  if (it != end())
  {
    return GetTo(it);
  }
  else return ObjectSet_t();
}

SequenceLink::SequenceLink(vector<DoubleLinks *> &AllPairs, vector<ClustersInfo *> &clusters_info_data, double time_threshold) : ClustersInfoData(clusters_info_data)
{
  NumFrames     = clusters_info_data.size();
  TimeThreshold = time_threshold;

  ComputeGlobalSequences(AllPairs);
}

void SequenceLink::ComputeGlobalSequences(vector<DoubleLinks *> &AllPairs)
{
  int count = 0;
  DoubleLinks_iterator_t it;
  //AtLeastOneUntracked = false;

  DoubleLinks *First = AllPairs[0];

  //First->print();

  for (it = First->begin(); it != First->end(); it ++)
  {
    count ++;

    ObjectSet_t Left, Right, Next;
    ObjectSequence_t CurrentSequence;

    Left  = it->first;
    Right = it->second;
    //if ((Left.size() == 0) || (Right.size() == 0)) continue;

    CurrentSequence.push_back(Left);
    CurrentSequence.push_back(Right);

    Left = Right;

    bool Lost = false;
    for (int i=1; i<AllPairs.size(); i++)
    {
      Next = AllPairs[i]->FindLink( Left );
      if (Next.size() == 0)
      {
        cerr << "*** TRACKER IS LOST! ***";
        cerr << " at frame " << i+1 << ", object (";
        print_objects_set( cerr, Left, "," );
        cerr << ")" << endl << "              ";
//        exit(EXIT_FAILURE);
        Lost = true;
      }
      CurrentSequence.push_back(Next);
      Left = Next;
    }
    if (!Lost)
    {
      TrackedObjects.push_back(CurrentSequence);
    }

    /* DEBUG 
    cout << "   " << count << "=";
    write(cout, CurrentSequence, "   ", ",", " <-> "); */
  }

  write(cout, TrackedObjects, "   ", "+", " <-> ");
  Merge();

  FindUntracked();

  FilterByTime(TimeThreshold);
}

void SequenceLink::Merge(int s1, int s2)
{
  /* Insert s2 elements in s1 */
  for (int i=0; i<TrackedObjects[s2].size(); i++)
  {
    ObjectSet_t CG = TrackedObjects[s2][i];
    ObjectSet_t::iterator it;
    for (it=CG.begin(); it!=CG.end(); it++)
    {
      TrackedObjects[s1][i].insert(*it);
    }
  }
  /* Delete s2 */
  TrackedObjects.erase(TrackedObjects.begin() + s2);
}

void SequenceLink::Merge()
{
  int SequenceLength = TrackedObjects[0].size();
  int i=0, j=0, k=0;

  cout << endl << "+ Merging sequences that share clusters..." << endl << endl;

  while (i < TrackedObjects.size())
  {
    j = i+1;
    while (j < TrackedObjects.size())
    {
      for (k=0; k<SequenceLength; k++)
      {
        ObjectSet_t isec;

        set_intersection( TrackedObjects[i][k].begin(), TrackedObjects[i][k].end(), TrackedObjects[j][k].begin(), TrackedObjects[j][k].end(), inserter(isec, isec.begin()) );
        if (isec.size() > 0)
        {
          /* DEBUG 
          cout << "Merging sequence " << i+1 << " and " << j+1 << " (column " << k+1 << ")   ";
          print_objects_set( cout, TrackedObjects[i][k], "," );
          cout << endl; */

          Merge(i, j);

          j --;
          break;
        }
      }
      j++;
    }
    i++;
  }

  /* DEBUG */
  write(cout, TrackedObjects, "   ", "+", " <-> ");
}

void SequenceLink::FindUntracked()
{
  int NumFrames = ClustersInfoData.size();
  int NumTrackedObjects = TrackedObjects.size();
  ObjectSequence_t tmp;

  cout << "+ Filtering untracked objects..." << endl << endl;

  for (int Frame = 0; Frame < NumFrames; Frame ++)
  {
    int NumUntrackedObjectsPerFrame = ClustersInfoData[Frame]->GetNumClusters() - NumTrackedObjects;

    /* DEBUG 
    cout << "[DEBUG] Frame: " << Frame << " Untracked objects: " << NumUntrackedObjectsPerFrame << endl; */

//    if (NumUntrackedObjectsPerFrame > 0) 
    {
      ObjectSet_t Tracked;
      ObjectSet_t Untracked;

      for (int CurrentObjectID = 0; CurrentObjectID < NumTrackedObjects; CurrentObjectID ++)
      {
	ObjectSet_t CurrentObject = TrackedObjects[CurrentObjectID][Frame];
        ObjectSet_iterator_t it;

        for (it = CurrentObject.begin(); it != CurrentObject.end(); ++ it)
        {
          Tracked.insert ( *it );
        }
      }
      for (int CurrentObjectID = 1; CurrentObjectID <= ClustersInfoData[Frame]->GetNumClusters(); CurrentObjectID ++)
      {
        if (Tracked.find( CurrentObjectID ) == Tracked.end())
        {
          Untracked.insert( CurrentObjectID );
        }
      }

      /*
      if (Untracked.size() > 0)
      {
        AtLeastOneUntracked = true;
      }
      */
      tmp.push_back( Untracked );
    }
  }

//  if (AtLeastOneUntracked)
//  {
    UntrackedObjects.push_back( tmp );
    /* DEBUG */
    write(cout, UntrackedObjects, "   ", "+", " <-> "); 
//  }
}

void SequenceLink::FilterByTime(double TimeThreshold)
{
  SequenceLink_iterator_t it;

  SequenceLink_t PassFilter;
  SequenceLink_t DontPassFilter;

  if (TimeThreshold > 0)
  {
    cout << "+ Filtering objects below " << TimeThreshold << "% of time..." << endl << endl;
    for (it = TrackedObjects.begin(); it != TrackedObjects.end(); ++ it)
    {
      ObjectSequence_t CurrentSequence = *it;
      ObjectSequence_iterator_t it2;
      int CurrentFrame = 0;
      int DontPass = 0;

      for (it2 = CurrentSequence.begin(); it2 != CurrentSequence.end(); ++ it2)
      {
        ObjectSet_t ObjectsAtCurrentFrame = *it2;
        ObjectSet_iterator_t it3;
        double TotalTimePct = 0;
    
        for (it3 = ObjectsAtCurrentFrame.begin(); it3 != ObjectsAtCurrentFrame.end(); ++ it3)
        {
          ClusterID_t CurrentCluster = *it3;
  
          TotalTimePct += ClustersInfoData[CurrentFrame]->GetPctTotalDuration( CurrentCluster );
        }
        TotalTimePct *= 100;

        if (TotalTimePct < TimeThreshold)
        {
          DontPass ++;
        }
      }

      if (DontPass > 1)
      {
        DontPassFilter.push_back( CurrentSequence );
      }
      else 
      {
        PassFilter.push_back( CurrentSequence );
      }
    }
    TrackedObjects  = PassFilter;
    FilteredObjects = DontPassFilter;

    /* DEBUG */
    write(cout, FilteredObjects, "   ", "+", " <-> ");
  }
}

void SequenceLink::write(ostream &Channel, ObjectSequence_t &Sequence, string Prefix, string GroupDelimiter, string FrameDelimiter)
{
  Channel << Prefix;
  for (int j=0; j<Sequence.size(); j++)
  {
    print_objects_set( Channel, Sequence[j], GroupDelimiter );
    if (j < Sequence.size()-1) Channel << FrameDelimiter;
    else Channel << endl;
  }
}

void SequenceLink::write(ostream &Channel, SequenceLink_t &Sequence, string Prefix, string GroupDelimiter, string FrameDelimiter)
{
  Channel << Prefix << "Objects=" << Sequence.size() << endl;
  for (int i=0; i<Sequence.size(); i++)
  {
    Channel << Prefix << i+1 << "=";
    write(Channel, Sequence[i], "", GroupDelimiter, FrameDelimiter);
  }
  Channel << endl;
}


void SequenceLink::write(ostream &Channel, bool All, bool PrettyPrint)
{
  string Prefix;
  string FrameDelimiter;
  string GroupDelimiter;

  if (PrettyPrint)
  {
    Prefix         = "   ";
    FrameDelimiter = " <-> ";
    GroupDelimiter = "+";
  }
  else
  {
    Prefix         = "";
    GroupDelimiter = ",";
    FrameDelimiter = ";";

    Channel << "[Info]" << endl;
    Channel << "Frames=" << NumFrames << endl << endl;
  }
 
  Channel << Prefix << "[Tracked]" << endl;
  write( Channel, TrackedObjects, Prefix, GroupDelimiter, FrameDelimiter );
  if (All)
  {
    Channel << Prefix << "[Filtered]" << endl;
    write( Channel, FilteredObjects, Prefix, GroupDelimiter, FrameDelimiter );
    Channel << Prefix << "[Untracked]" << endl;
    write( Channel, UntrackedObjects, Prefix, GroupDelimiter, FrameDelimiter );
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
  for (int i=0; i<TrackedObjects.size(); i++)
  {
    ObjectSequence_t CurrentSequence = TrackedObjects[i];

    ObjectSet_t OldClusters = CurrentSequence[trace];

    ObjectSet_iterator_t it;
    for (it = OldClusters.begin(); it != OldClusters.end(); it ++)
    {
      TTypeValuePair p1 = make_pair(CLUSTER_EV, *it + FIRST_CLUSTER - 1);
      TTypeValuePair p2 = make_pair(CLUSTER_EV, i + FIRST_CLUSTER);

      TranslationTable[p1] = p2;
    }
  }
  
  for (int i=0; i<FilteredObjects.size(); i++)
  {
    ObjectSequence_t CurrentSequence = FilteredObjects[i];
    ObjectSet_t OldClusters = CurrentSequence[trace];
    ObjectSet_iterator_t it;
    for (it = OldClusters.begin(); it != OldClusters.end(); it ++)
    {
      TTypeValuePair p1 = make_pair(CLUSTER_EV, *it + FIRST_CLUSTER - 1);
      TTypeValuePair p2 = make_pair(CLUSTER_EV, THRESHOLD_FILTERED);
      TranslationTable[p1] = p2;
    }
  }

  for (int i=0; i<UntrackedObjects.size(); i++)
  {
    ObjectSequence_t CurrentSequence = UntrackedObjects[i];
    ObjectSet_t OldClusters = CurrentSequence[trace];
    ObjectSet_iterator_t it;
    for (it = OldClusters.begin(); it != OldClusters.end(); it ++)
    {
      TTypeValuePair p1 = make_pair(CLUSTER_EV, *it + FIRST_CLUSTER - 1);
      TTypeValuePair p2 = make_pair(CLUSTER_EV, NOISE);
      TranslationTable[p1] = p2;
    }
  }

  return TranslationTable.size();
}

