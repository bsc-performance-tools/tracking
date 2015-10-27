#ifndef __TRACKING_H__
#define __TRACKING_H__

#include <vector>
using std::vector;
#include <string>
using std::string;
#include "ClusterIDs.h"
#include "Links.h"
#include "CallersHistogram.h"
#include "ClustersAlignment.h"
#include "DistanceTracker.h"
#include "CallersTracker.h"
#include "SequenceTracker.h"
#include "DensityTracker.h"
#include "SPMDTracker.h"
#include "ClustersInfo.h"

#define DEFAULT_MIN_SCORE      0.90
#define DEFAULT_CROSS_DISTANCE 1.00
#define DEFAULT_THRESHOLD      15

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
      DistanceTracker  *ByDistance;
      CallersTracker   *ByCallers;
      SPMDTracker      *BySPMD;
      SequenceTracker  *BySequence;
      DensityTracker   *ByDensity;

      FrameLinks *OneWay;
      DoubleLinks *TwoWay;
      DoubleLinks *Final;

    } FramesMatrix_t;
  
    Tracking(vector<string> traces, vector<ClusterID_t> last_clusters, double min_time_pct, double threshold, double max_distance, string callers_cfg, double min_score, bool use_density, string prefix, bool reconstruct, int verbose);
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
    vector<ClusterID_t>    NumClustersToTrack;
    vector<string> OrigCSVs;
    vector<string> InputCSVs;
    vector<string> InputCINFOs;
    vector<int>    TotalClusters;
    vector<string> InputAlignments;
    vector<string> OutputHistoCallers;
    FramesMatrix_t **FramesMatrix;
    vector<ClustersInfo *> ClustersInfoData;

    SequenceLink *FinalSequence;

    vector<ClustersAlignment *> Alignments;
    vector<CallersHistogram  *> Histograms;
  
    double Epsilon;
    double CrossDistance;
    double MinimumScore;
    double Threshold;
    string CallersCFG;
    string OutputPrefix;
    bool UseCallers;
    bool UseSequence;
    bool UseSPMDiness;
    bool UseDistance;
    bool UseDensity;
    bool UseAlignment;
    int  Verbose;
    bool Reconstruct;
    set<string> TrackersAppliedAtRound1;
    string OutputSequenceFile;
    double TimeThresholdAfter;

    void PrepareFileNames();
    void CompareFrames(int frame1, int frame2);
    void RunTrackers1();
    void RunTrackers2();
    void CombineTrackers1();
    void CombineTrackers2();

    void PrintCorrelations(ClusterID_t ClusterID, ObjectSet_t &ClusterCorrelations);

};

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
"  -d                         Enable the density tracker\n"\
"\n"\
"  -h                         Prints this help\n"\
"\n"\
"  -m <min-time-pct>          Discard clusters below the given duration time percentage\n"\
"\n"\
"  -o                         Set the prefix for all the output files\n"\
"\n"\
"  -p <max-distance>          Maximum Epsilon distance to use the position tracker\n"\
"\n"\
"  -r                         Enable the trace reconstruction with tracked clusters\n"\
"\n"\
"  -t <threshold>             Minimum likeliness percentage in order to match two clusters\n"\
"\n"\
"  -v[v]                      Run in verbose mode (-vv for extra debug messages)\n"\
"\n"\

#endif /* __TRACKING_H__ */
