#ifndef __CORRELATION_H__
#define __CORRELATION_H__

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
#include "kernelconnection.h"

class OneWayCorrelation
{
public:
  OneWayCorrelation(CID cluster_id);
  TClusterGroup CorrelatingClusters;

  int size();
  void add(CID cluster_id);
  void print();
  void join(OneWayCorrelation *AnotherCorrelation);
  void intersect(OneWayCorrelation *AnotherCorrelation);
  TClusterGroup::iterator begin();
  TClusterGroup::iterator end();
  CID ID();
  TClusterGroup CorrelatesTo();

private:
  CID ClusterID;
};

typedef pair<TClusterGroup, TClusterGroup> TLinkedGroups;

class TwoWayCorrelation
{
public:
  TwoWayCorrelation();
  TwoWayCorrelation(vector<OneWayCorrelation *> &ForwardCorrelation, vector<OneWayCorrelation *> &BackwardCorrelation);

  void print();
  void add(TClusterGroup LeftGroup, TClusterGroup RightGroup);
  void Combine();
  vector<TLinkedGroups>::iterator begin();
  vector<TLinkedGroups>::iterator end();
  TClusterGroup FindLink( TClusterGroup &LeftGroup );
  void GetUnique(map<CID, CID> &UniqueCorrelations);

private:
  vector< TLinkedGroups > Links;
};

class NWayCorrelation
{
public:
  typedef vector<TClusterGroup>    TClusterSequence;
  typedef vector<TClusterSequence> TTraceSequence;

  NWayCorrelation(vector<TwoWayCorrelation *> &AllPairs);
  void write(ostream &Channel);
  void write(ostream &Channel, TClusterSequence &Sequence, string Delimiter);

  int GetTranslationTable(int trace, map< TTypeValuePair, TTypeValuePair > &TranslationTable);

private:

  TTraceSequence FinalSequence;

  void UnifySequences();
  void UnifySequences(int s1, int s2);

};

#endif /* __CORRELATION_H__ */
