#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "ClusterIDs.h"
#include "Links.h"
#include "CorrelationMatrix.h"
#include "CallersTracker.h"
#include "SequenceTracker.h"

#define SUFFIX_ORIG_CSV                      ".DATA.csv"
#define SUFFIX_NORM_CSV                      SUFFIX_ORIG_CSV".norm"
#define SUFFIX_CINFO                         ".clusters_info.csv"
#define SUFFIX_ALIGN                         ".seq"
#define SUFFIX_SCALED_PLOT                   ".gnuplot.scaled"
#define SUFFIX_TRACKED_TRACE                 ".tracked.prv"
#define SUFFIX_SEQUENCES                     ".SEQUENCES"
#define SUFFIX_GUI_CONFIG                    ".xtrack"
#define SUFFIX_HISTOGRAM_CALLERS_PER_CLUSTER ".3D"
#define SUFFIX_RECOLORED_CSV                 ".recolored.data"

class Tracking
{
public:
  typedef struct
  {
    CorrelationMatrix *ByCross;
    CorrelationMatrix *ByCallers;
    DoubleLink        *PairedByMixer;
    DoubleLink        *PairedBySequence;
    DoubleLink        *PairedCombo;
  } ClustersCorrelations_t;
  
  Tracking(vector<string> traces, vector<CID> last_clusters, string callers_cfg, double min_score, string prefix, bool reconstruct, bool verbose);
  ~Tracking();

  void CorrelateTraces();
  void BuildFinalSequence();
  // void GeneratePlots();
  void Recolor();
  void ReconstructTraces();

private:
  int NumberOfTraces;
  vector<string> InputTraces;
  vector<string> OutputTraces;
  vector<CID>    NumClustersToTrack;
  vector<string> OrigCSVs;
  vector<string> InputCSVs;
  vector<string> InputCINFOs;
  vector<int>    TotalClusters;
  vector<string> InputAlignments;
  vector<string> OutputHistoCallers;
  ClustersCorrelations_t **TracesCorrelationMatrix;

  SequenceLink *FinalSequence;
  
  double Epsilon;
  double MinimumScore;
  string CallersCFG;
  string OutputPrefix;
  bool UseCallers;
  bool UseAlignment;
  bool UseCrossClassify;
  bool Verbose;
  bool Reconstruct;
  string OutputSequenceFile;

  void PrepareFileNames();
  CorrelationMatrix * TrackByCrossClassify(int trace1, int trace2);
  CorrelationMatrix * TrackByCallers      (int trace1, int trace2, vector<Histo3D *> &H3Ds);
  CorrelationMatrix * TrackBySequence     (int trace1, int trace2);

  void Mix(
    int trace1, 
    int trace2, 
    vector<SequenceTracker *>          &STs,
    vector<CorrelationMatrix *> &BySimultaneity,
    vector<Link *>        &ResultingMatch);

  void PrintCorrelations(CID ClusterID, TClustersSet &ClusterCorrelations);

};

#define DEFAULT_MIN_SCORE 0.90

#define HELP \
"\n"\
"Automatic cluster scalability tracker\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"\n"\
"USAGE\n"\
"  %s [OPTIONS] TRACE1.prv[:max_clusters_to_track] ... TRACEN.prv[:max_clusters_to_track]\n\n"\
"OPTIONS\n"\
"\n"\
"  -a <min-score>             Minimum SPMD score to use the alignment tracker\n"\
"\n"\
"  -c <callers.cfg>           CFG filtering one level of callers to use the callstack tracker\n"\
"\n"\
"  -h                         Prints this help\n"\
"\n"\
"  -v                         Run in verbose mode\n"\
"\n"\

#endif /* __TRACKING_H__ */
