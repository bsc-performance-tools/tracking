#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <sstream>
using std::stringstream;
#include "Tracking.h"
#include "CrossClassifier.h"
#include "CallersTracker.h"
#include "SequenceTracker.h"
#include "Utils.h"
#include "TraceReconstructor.h"

Tracking::Tracking(vector<string> traces, vector<CID> last_clusters, string callers_cfg, double min_score, string prefix, bool reconstruct, bool verbose)
{
  InputTraces       = traces;
  InputLastClusters = last_clusters;
  NumberOfTraces    = traces.size();
  Epsilon           = 0.018;
  CallersCFG        = callers_cfg;
  OutputPrefix      = prefix;
  Reconstruct       = reconstruct;
  Verbose           = verbose;
  FinalSequence     = NULL;
  MinimumScore      = min_score;

  UseCallers       = false;
  if (callers_cfg.size() > 0) UseCallers = true;
  UseSequence      = true;
  UseCrossClassify = true;

  TracesCorrelationMatrix = NULL;

  TracesCorrelationMatrix = (ClustersCorrelations_t **)malloc(NumberOfTraces * sizeof(ClustersCorrelations_t *));
  for (int i=0; i<NumberOfTraces; i++)
  {
    TracesCorrelationMatrix[i] = (ClustersCorrelations_t *)malloc(NumberOfTraces * sizeof(ClustersCorrelations_t));
    TracesCorrelationMatrix[i]->ByCross    = NULL;
    TracesCorrelationMatrix[i]->ByCallers  = NULL;
  }

  PrepareFileNames();
}

Tracking::~Tracking()
{
  if (TracesCorrelationMatrix != NULL)
  {
    for (int i=0; i<NumberOfTraces; i++)
    {
      if (TracesCorrelationMatrix[i]->ByCross    != NULL) delete TracesCorrelationMatrix[i]->ByCross;
      if (TracesCorrelationMatrix[i]->ByCallers  != NULL) delete TracesCorrelationMatrix[i]->ByCallers;
      free(TracesCorrelationMatrix[i]);
    }
    free(TracesCorrelationMatrix);
  }
}

void Tracking::PrepareFileNames()
{
  for (int i=0; i<NumberOfTraces; i++)
  {
    string CurrentTrace  = InputTraces[i];
    string TraceBaseName = (RemoveExtension(CurrentTrace).c_str());
    string CSVFile       = TraceBaseName + CSV_NORM_SUFFIX;
    string CINFOFile     = TraceBaseName + CINFO_SUFFIX;
    string AlignFile     = TraceBaseName + ALIGN_SUFFIX;

    InputCSVs.push_back( CSVFile );
    InputCINFOs.push_back( CINFOFile );
    InputAlignments.push_back( AlignFile );
    OutputTraces.push_back( TraceBaseName + TRANSLATED_TRACE_SUFFIX );
  }
  OutputSequenceFile = OutputPrefix + ".SEQUENCES";

}


void Tracking::CorrelateTraces()
{
  vector<Histo3D *>         H3Ds;
  vector<SequenceTracker *> STs;
  vector<ClusterCorrelationMatrix *> BySimultaneity;

  for (int i=0; i<NumberOfTraces; i++)
  {
    if (UseCallers)
    {
      cout << "+ Computing histogram of callers per cluster for trace " << i+1 << "..." << endl;
      CallersTracker CT = CallersTracker();
      H3Ds.push_back( CT.Compute3D(InputTraces[i], CallersCFG) ) ;
      if (H3Ds[i]->size() <= 0)
      {
        cout << "WARNING: Histogram is empty. Callers will not be used for trace " << i+1 << ". Try increasing the callers level!" << endl;
      }
      cout << endl;
    }
    if (UseSequence)
    { 
      cout << endl << "+ Computing sequence alignment for trace " << i+1 << "..." << endl << endl;
      STs.push_back( new SequenceTracker(InputAlignments[i], InputCINFOs[i]) );
      
      if (STs[i]->isAvailable())
      {
        cout << endl << "+ Building simultaneity matrix for trace " << i+1 << "..." << endl << endl;
        BySimultaneity.push_back( STs[i]->getClustersSimultaneity() );
        if (Verbose)
        {
          BySimultaneity[i]->Print();
          BySimultaneity[i]->Stats();
        }
      }
      else
      {
        BySimultaneity.push_back( NULL );
      }
    }
  }
  for (int trace1=0; trace1<NumberOfTraces-1; trace1++)
  {
    int trace2 = trace1+1;

    if (UseCrossClassify)
    {
      cout << "+ Cross-classifying traces " << trace1+1 << " and " << trace2+1 << endl << endl;
      TracesCorrelationMatrix[trace1][trace2].ByCross = TrackByCrossClassify(trace1, trace2);
      if (Verbose)
      {
        TracesCorrelationMatrix[trace1][trace2].ByCross->Print();
        TracesCorrelationMatrix[trace1][trace2].ByCross->Stats();
      }
      cout << "+ Cross-classifying traces " << trace2+1 << " and " << trace1+1 << endl << endl;
      TracesCorrelationMatrix[trace2][trace1].ByCross = TrackByCrossClassify(trace2, trace1);
      if (Verbose)
      {
        TracesCorrelationMatrix[trace2][trace1].ByCross->Print();
        TracesCorrelationMatrix[trace2][trace1].ByCross->Stats();
      }
    }
    if ((UseCallers) && (H3Ds[trace1]->size() > 0) && (H3Ds[trace2]->size() > 0))
    {
      cout << "+ Crossing callers between traces " << trace1+1 << " and " << trace2+1 << endl << endl;
      TracesCorrelationMatrix[trace1][trace2].ByCallers = TrackByCallers(trace1, trace2, H3Ds);
      if (Verbose)
      {
        TracesCorrelationMatrix[trace1][trace2].ByCallers->Print();
        TracesCorrelationMatrix[trace1][trace2].ByCallers->Stats();
      }
      cout << "+ Crossing callers between traces " << trace2+1 << " and " << trace1+1 << endl << endl;
      TracesCorrelationMatrix[trace2][trace1].ByCallers = TrackByCallers(trace2, trace1, H3Ds);
      if (Verbose)
      {
        TracesCorrelationMatrix[trace2][trace1].ByCallers->Print();
        TracesCorrelationMatrix[trace2][trace1].ByCallers->Stats();
      }
    }

    vector<OneWayCorrelation *> Forward, Backward;
    Mix(trace1, trace2, STs, BySimultaneity, Forward);
    Mix(trace2, trace1, STs, BySimultaneity, Backward);

    TracesCorrelationMatrix[trace1][trace2].Paired = new TwoWayCorrelation(Forward, Backward);
    if (Verbose)
    {
      cout << "+ 2-way correlation between traces " << trace1+1 << " and " << trace2+1 << ":" << endl << endl;
      TracesCorrelationMatrix[trace1][trace2].Paired->print();
    }

    if ((UseSequence) && (STs[trace1]->isAvailable()) && (STs[trace2]->isAvailable()))
    {
      double Score1 = STs[trace1]->getGlobalScore();
      double Score2 = STs[trace2]->getGlobalScore();
      if ((Score1 < MinimumScore) || (Score2 < MinimumScore))
      {
        cout << "WARNING: Skipping time sequences correlation between traces " << trace1+1 << " and " << trace2+1;
        cout << " due to low scores (Score1=" << Score1 << ", Score2=" << Score2 << ", Minimum= " << MinimumScore << ")" << endl;
      }
      else 
      {
        /* Get the correlations that are univocal */
        map<CID, CID> UniqueCorrelations;
        TracesCorrelationMatrix[trace1][trace2].Paired->GetUnique(UniqueCorrelations);
        if (Verbose)
        {
          cout << "(" << UniqueCorrelations.size() << " univocal correlations)" << endl;
        }
        TracesCorrelationMatrix[trace1][trace2].PairedBySequence = STs[trace1]->PairWithAnother(
           UniqueCorrelations, 
           STs[trace2], 
           InputLastClusters[trace1], 
           InputLastClusters[trace2]);

        TracesCorrelationMatrix[trace1][trace2].PairedBySequence->Combine();
        if (Verbose)
        {
          cout << "+ 2-way correlation between traces " << trace1+1 << " and " << trace2+1 << " (AS REPORTED BY SEQUENCE TRACKER):" << endl << endl;
          TracesCorrelationMatrix[trace1][trace2].PairedBySequence->print();
        }
      }
    }
  }

  BuildFinalSequence();

  GeneratePlots();

  Recolor();

  if (Reconstruct)
  {
    ReconstructTraces();
  }

  cout << "+ Tracking finished!" << endl;
}

void Tracking::BuildFinalSequence()
{
  cout << endl << "+ Computing the final clusters sequences..." << endl << endl;

  unlink(OutputSequenceFile.c_str());
  ofstream fd;
  fd.open (OutputSequenceFile.c_str(), ios::out | ios::trunc);

  vector<TwoWayCorrelation *> AllPairs;

  for (int trace1=0; trace1<NumberOfTraces-1; trace1++)
  {
    int trace2 = trace1+1;
    AllPairs.push_back ( TracesCorrelationMatrix[trace1][trace2].Paired );
  }

  FinalSequence = new NWayCorrelation(AllPairs);

  FinalSequence->write(fd);

  fd.close();
}

ClusterCorrelationMatrix * Tracking::TrackByCrossClassify(int trace1, int trace2)
{
  stringstream ssCrossClassifyOutput;
  ssCrossClassifyOutput << "cross-classify-" << trace1 << "-" << trace2 << ".csv";
  
  CrossClassifier CC = CrossClassifier(InputCSVs[trace1], InputCSVs[trace2], ssCrossClassifyOutput.str(), 1.0);

  ClusterCorrelationMatrix *CCM = CC.CrossClassify();
  return CCM;
}

ClusterCorrelationMatrix * Tracking::TrackByCallers(int trace1, int trace2, vector<Histo3D *> &H3Ds)
{
  CallersTracker CC = CallersTracker();
  Histo3D *H1, *H2;
  
  H1 = H3Ds[trace1];
  H2 = H3Ds[trace2];

  ClusterCorrelationMatrix *CCM = CC.CrossCallers(H1, H2);

  return CCM;
}

void Tracking::Mix(int trace1, int trace2, vector<SequenceTracker *> &STs, vector<ClusterCorrelationMatrix *> &BySimultaneity, vector <OneWayCorrelation *> &ResultingMatch)
{
  ClusterCorrelationMatrix *ByCrossClassify = TracesCorrelationMatrix[trace1][trace2].ByCross;
  ClusterCorrelationMatrix *ByCallers       = TracesCorrelationMatrix[trace1][trace2].ByCallers;

  bool CrossApplicable    = ((UseCrossClassify) && (ByCrossClassify != NULL));
  bool CallersApplicable  = ((UseCallers) && (ByCallers != NULL));
  bool SequenceApplicable = ((UseSequence) && (STs[trace2]->isAvailable()) && (BySimultaneity[trace2] != NULL));

  ResultingMatch.clear();

  if (SequenceApplicable)
  {
    double GlobalScore  = STs[trace2]->getGlobalScore();

    if (GlobalScore < MinimumScore)
    {
      cout << "WARNING: Global score for trace " << trace2+1 << " is too low=" << GlobalScore << " (Minimum=" << MinimumScore << ")" << endl;
      SequenceApplicable = false;
    }
  }

  if ((!CrossApplicable) && (!CallersApplicable) && (!SequenceApplicable))
  {
    cerr << endl;
    cerr << "ERROR: Any heuristic can be applied to cross traces " << trace1+1 << " and " << trace2+1 << "." << endl;
    cerr << "Please review the execution log for previous errors and change the configuration. Aborting..." << endl << endl;
    exit(-1);
  }

  //int ClustersPerTrace = ByCrossClassify->getNumberOfClusters(); /* Process all clusters for this trace */ 
  int ClustersPerTrace = InputLastClusters[trace1];

  for (CID CurrentClusterID = 1; CurrentClusterID <= ClustersPerTrace; CurrentClusterID ++)
  {
    OneWayCorrelation *CombinedCorrelation = new OneWayCorrelation(CurrentClusterID);
    OneWayCorrelation *ClassifyCorrelation = NULL;

    /* Get initial correlations from classification */
    if (UseCrossClassify)
    {
      ClassifyCorrelation = ByCrossClassify->getCorrelation(CurrentClusterID, -1);
      CombinedCorrelation->join(ClassifyCorrelation);
    }

    /* Find simultaneous clusters for every possible correlation */
    if (SequenceApplicable)
    {
      double ClusterScore = STs[trace2]->getClusterScore(CurrentClusterID);

      if (ClusterScore < MinimumScore)
      {
        cout << "WARNING: Cluster " << CurrentClusterID << " score in trace " << trace2+1;
        cout << " is too low=" << ClusterScore << " (Minimum=" << MinimumScore << ")" << endl;
      }
      else
      {
        TClusterGroup::iterator it;
        {
          for (it = ClassifyCorrelation->begin(); it != ClassifyCorrelation->end(); it++)
          {
            /* Union all the possible simultaneous clusters */
            OneWayCorrelation *Simultaneous = BySimultaneity[trace2]->getCorrelation(*it, 5);
            CombinedCorrelation->join(Simultaneous);
          }
        }
      }
    }

    /* Intersect with callers information */
    if ((UseCallers) && (ByCallers != NULL))
    {
      OneWayCorrelation *CallersCorrelation = ByCallers->getCorrelation(CurrentClusterID, 0);
      CombinedCorrelation->intersect(CallersCorrelation);
    
      /* If the heuristics are contradictory, or callers are unique, follow callers */
      if ((CombinedCorrelation->size() == 0) || (CallersCorrelation->size() == 1))
      {
        CombinedCorrelation = CallersCorrelation;
      } 
    }

    ResultingMatch.push_back( CombinedCorrelation );
  }

  if (Verbose)
  {
    cout << "+ 1-way correlation from trace " << trace1+1 << " to " << trace2+1 << ":" << endl << endl;
    for (int i=0; i<ResultingMatch.size(); i++)
    {
      ResultingMatch[i]->print();
    }
    cout << endl;
  }
}
 
void Tracking::GeneratePlots()
{
  cout << "+ Generating plots...\n";
  cout.flush();

  string CMD = "perl -I " + string(getenv("TRACKING_HOME")) + "/bin " + string(getenv("TRACKING_HOME")) + "/bin/GeneratePlots.pl ";
  for (int i=0; i<InputTraces.size(); i++)
  {
    string TraceBaseName (RemoveExtension(InputTraces[i]).c_str());
    CMD += TraceBaseName + " ";
  }
  CMD += OutputPrefix;

  system(CMD.c_str());
  cout << "Plotting done!" << endl << endl;
}

void Tracking::Recolor()
{
  vector<string> InPlots;

  /* Clean previous files */
  for (int i=0; i<InputTraces.size(); i++)
  {
    string TraceBaseName (RemoveExtension(InputTraces[i]).c_str());
    string ScaledPlot = TraceBaseName + "*" + SCALED_PLOT_SUFFIX;

    InPlots.push_back ( ScaledPlot );
  }
  string CMD = string(getenv("TRACKING_HOME")) + "/bin/Recoloring.pl ";
  for (int i=0; i<InPlots.size(); i++)
  {
    CMD += InPlots[i] + " ";
  }
  CMD += OutputPrefix;
  /* cout << "[DEBUG] CMD=" << CMD << endl; */
  cout << "+ Recoloring plots... " << endl; cout.flush();
  system(CMD.c_str());
  cout << endl;
}

void Tracking::ReconstructTraces()
{
  vector< map<TTypeValuePair, TTypeValuePair> > AllTranslationTables;

  for (int i=0; i<NumberOfTraces; i++)
  {
    map<TTypeValuePair, TTypeValuePair> CurrentTraceTranslationTable;

    FinalSequence->GetTranslationTable(i, CurrentTraceTranslationTable);
    AllTranslationTables.push_back( CurrentTraceTranslationTable );
  }

  TraceReconstructor *TR = new TraceReconstructor( InputTraces, OutputTraces, AllTranslationTables );
}

