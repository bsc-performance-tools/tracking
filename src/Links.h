#ifndef __LINKS_H__
#define __LINKS_H__

#include <vector>
using std::vector;
using std::pair;
#include <fstream>
using std::ostream;
#include <string>
using std::string;
#include <map>
using std::map;
#include "ClusterIDs.h"
#include <localkernel.h>

void PrintClusterGroup( ostream & channel, TClustersSet &CG, string delim );
int subset(TClustersSet &g1, TClustersSet &g2);


/**
 * This class represents 1-way links of a single cluster to one or more clusters, e.g. 1 -> [ 1, 2 ]
 */
class Link
{
public:
  Link(Cluster cluster_id);

  Cluster      get_Cluster(void);
  TClustersSet get_Links(void);

  int  size     (void);
  void add      (Cluster cluster_id);
  void print    (void);
  void join     (Link *another_link);
  void intersect(Link *another_link);
  TClustersSet::iterator begin(void);
  TClustersSet::iterator end  (void);

private:
  Cluster      ClusterID;
  TClustersSet LinkedWith;
};

typedef pair<TClustersSet, TClustersSet> TLinkedGroups;

class DoubleLink
{
public:
  DoubleLink();
  DoubleLink(vector<Link *> &ForwardCorrelation, vector<Link *> &BackwardCorrelation);

 int size();
  void print();
  void add(TClustersSet LeftGroup, TClustersSet RightGroup);
  void add(vector<TLinkedGroups> UnsortedGroups);

  void Combine();
  DoubleLink * Split(DoubleLink *Specialized);
  vector<TLinkedGroups>::iterator begin();
  vector<TLinkedGroups>::iterator end();
  TClustersSet FindLink( TClustersSet &LeftGroup );
  void GetUnique(map<CID, CID> &UniqueCorrelations);

private:
  vector< TLinkedGroups > Links;
};

class SequenceLink
{
public:
  typedef vector<TClustersSet>    TClusterSequence;
  typedef vector<TClusterSequence> TTraceSequence;

  SequenceLink(vector<DoubleLink *> &AllPairs);
  void write(ostream &Channel);
  void write(ostream &Channel, TClusterSequence &Sequence, string Delimiter);

  int GetTranslationTable(int trace, int total_clusters, map< TTypeValuePair, TTypeValuePair > &TranslationTable);

private:

  TTraceSequence FinalSequence;

  void UnifySequences();
  void UnifySequences(int s1, int s2);

};

#endif /* __LINKS_H__ */
