#ifndef __CALLERS_HISTOGRAM_H__
#define __CALLERS_HISTOGRAM_H__

#include <map>
#include <string>
#include <vector>
#include "localkernel.h"
#include "paraverconfig.h"
#include "cfg.h"
#include "trace.h"
#include "window.h"
#include "histogram.h"
#include "paraverkernelexception.h"
#include "ClusterIDs.h"

using std::map;
using std::string;
using std::vector;

#define FIRST_VALID_CALLER 3

typedef struct
{
  string CallerName;
  int    NumBursts;
  double Pct;
} Caller_t;

class CallersHistogram
{
public:
  CallersHistogram(string trace_name, string callers_cfg, string savefile);
  ~CallersHistogram();

  int  size();

  void getClusters(vector<ClusterID_t> &clusters);
  void getCallersForCluster(ClusterID_t cluster, vector<Caller_t> &callers);
  void getClustersWithCaller(string caller_name, vector<ClusterID_t> &clusters);
  bool getStatsForClusterAndCaller(ClusterID_t cluster, string caller_name, Caller_t &caller_stats);
  bool hasCallers(ClusterID_t cluster_id);

private:
  string Tracename;
  string CFG;
  string SavedHistogramFile;
  map< ClusterID_t, vector<Caller_t> >     CallersPerClusterTable;
  KernelConnection                *myKernel;
  ParaverConfig                   *myConfig;

  bool load(string filename);
  bool load(Trace *&trace);
  void parseHistogram(Histogram *histogram);
  void compute3D();

  void insert(ClusterID_t cluster, Caller_t caller);
  void write(string filename);
  void print();
};

#endif /* __CALLERS_HISTOGRAM_H__ */

