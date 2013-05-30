#ifndef __MATCH_CALLERS_H__
#define __MATCH_CALLERS_H__

#include <vector>
using std::vector;
#include <set>
using std::set;
#include <map>
using std::map;
#include <string>
#include "localkernel.h"
#include "paraverconfig.h"
#include "cfg.h"
#include "trace.h"
#include "window.h"
#include "histogram.h"
#include "paraverkernelexception.h"
#include "CorrelationMatrix.h"
#include "ClusterIDs.h"

typedef struct
{
  string CallerName;
  int    NumBursts;
  double Pct;
} TCaller;

class Histo3D
{
public:
  void insert(CID key, TCaller value);
  void dump();
  int  size();
  void write(string filename);
  bool load(string filename);
  void getClusters(vector<CID> &clusters);
  void getCallersForCluster(CID cluster, vector<TCaller> &callers);
  void getClustersWithCaller(string caller_name, vector<CID> &clusters);
  bool getClusterCallerStats(CID cluster_id, string caller_name, TCaller &caller_stats);
  bool hasCallers(CID cluster_id);
private:
  map< CID, vector<TCaller> > H;
};

class CallersTracker
{
public:
  CallersTracker();
  Histo3D * Compute3D(string strTrace, string strCFG, string SavedHistogramFileName);
  CorrelationMatrix * CrossCallers(Histo3D *H1, Histo3D *H2);

private:
  KernelConnection *myKernel;
  ParaverConfig    *myConfig;

  bool loadTrace( KernelConnection *myKernel, string strTrace, Trace * & trace );
  void parseHistogram(Histogram * histo, Histo3D *&trace3DH);
};

#if 0
class CallersTrackerResult
{
private:
	vector< set<unsigned int> > Matchings;

public:
	CallersTrackerResult();

	void getMatchesByCaller (int ClusterID, set<unsigned int> & Matches);
	bool ClusterMatchesWith (int ClusterID, int MatchID);

	int size();
	unsigned int insertCluster();
	void insertClusterMatches(unsigned int key, set<unsigned int> values);
	void dump();

	void WriteFile(ofstream & fd);
	void LoadFile(ifstream & fd);
};

class CallersTracker
{
public:
	CallersTracker(string strTrace1, string strTrace2, string strCFG);
	CallersTracker(string InFile);
	void WriteFile(string OutFile);
	void LoadFile(string InFile);
	void getResults(CallersTrackerResult ** resDirect, CallersTrackerResult ** resReverse);

private:
	Trace *trace1, *trace2;
	vector< vector<string> > ClusterCallerMap1, ClusterCallerMap2;
	CallersTrackerResult * DirectMatch, * ReverseMatch;

	bool loadTrace( KernelConnection *myKernel, string strTrace, Trace * & trace );
	void parseHistogram(Histogram * histo, vector< vector< string > > & ClusterCallerMap);
	void FindClusterForCaller( string caller, vector< vector<string> > map, set<unsigned int> & values );
	void CompareClusters(vector< vector<string> > map1, vector< vector<string> > map2, CallersTrackerResult * match);
	void DumpClusters(vector< vector<string> > map);

};
#endif

#endif /* __MATCH_CALLERS_H__ */

