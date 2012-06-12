#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "ClusterCorrelationMatrix.h"
#include "CallersTracker.h"
#include "SequenceTracker.h"
#include "Correlation.h"

#define CSV_SUFFIX              ".clustered.csv"
#define CSV_NORM_SUFFIX         ".clustered.csv.norm"
#define CINFO_SUFFIX            ".clusters_info.csv"
#define ALIGN_SUFFIX            ".align"
#define SCALED_PLOT_SUFFIX      ".gnuplot.scaled"
#define TRANSLATED_TRACE_SUFFIX ".recolored.prv"

string RemoveExtension(string &filename);

class Tracking
{
public:
  typedef struct
  {
  	ClusterCorrelationMatrix *ByCross;
	ClusterCorrelationMatrix *ByCallers;
    TwoWayCorrelation        *PairedByMixer;
    TwoWayCorrelation        *PairedBySequence;
    TwoWayCorrelation        *PairedCombo;
  } ClustersCorrelations_t;
  
  Tracking(vector<string> traces, vector<CID> last_clusters, string callers_cfg, double min_score, string prefix, bool reconstruct, bool verbose);
  ~Tracking();

  void CorrelateTraces();
  void BuildFinalSequence();
  void GeneratePlots();
  void Recolor();
  void ReconstructTraces();

private:
  int NumberOfTraces;
  vector<string> InputTraces;
  vector<string> OutputTraces;
  vector<CID>    InputLastClusters;
  vector<string> InputCSVs;
  vector<string> InputCINFOs;
  vector<string> InputAlignments;
  ClustersCorrelations_t **TracesCorrelationMatrix;

  NWayCorrelation *FinalSequence;
  
  double Epsilon;
  double MinimumScore;
  string CallersCFG;
  string OutputPrefix;
  bool UseCallers;
  bool UseSequence;
  bool UseCrossClassify;
  bool Verbose;
  bool Reconstruct;
  string OutputSequenceFile;

  void PrepareFileNames();
  ClusterCorrelationMatrix * TrackByCrossClassify(int trace1, int trace2);
  ClusterCorrelationMatrix * TrackByCallers      (int trace1, int trace2, vector<Histo3D *> &H3Ds);
  //ClusterCorrelationMatrix * TrackBySequence     (int trace1, int trace2);

  void Mix(
    int trace1, 
    int trace2, 
    vector<SequenceTracker *>          &STs,
    vector<ClusterCorrelationMatrix *> &BySimultaneity,
    vector<OneWayCorrelation *>        &ResultingMatch);

  void PrintCorrelations(CID ClusterID, TClusterGroup &ClusterCorrelations);

};

#define HELP \
"\n"\
"Automatic cluster scalability tracker\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"\n"\
"Usage:\n"\
"  %s [-c callers.cfg] [-v] <trace1.prv>:<max_clusters> ... <traceN.prv>:<max_clusters>\n"\
"\n"\
"  -h                         This help\n"\
"\n"\
"  -v                         Run in verbose mode\n"\
"\n"\
"  -c <callers.cfg>           CFG filtering one level of callers\n"\
"\n"\

#endif /* __TRACKING_H__ */
