#include <stdlib.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ios;
using std::ios_base;
#include <sstream>
using std::stringstream;
using std::ofstream;
#include "Tracking.h"
#include "Utils.h"
#include "TraceReconstructor.h"
#include <boost/algorithm/string/join.hpp>

Tracking::Tracking(vector<string> traces, vector<ClusterID_t> num_clusters_to_track, double threshold, double max_distance, string callers_cfg, double min_score, string prefix, bool reconstruct, int verbose)
{
  /* Configure the tracking algorithm and the different trackers */
  InputTraces        = traces;
  NumClustersToTrack = num_clusters_to_track;
  NumberOfTraces     = traces.size();
  Epsilon            = 0.018;
  CallersCFG         = callers_cfg;
  OutputPrefix       = prefix;
  Reconstruct        = reconstruct;
  Verbose            = verbose;
  FinalSequence      = NULL;
  MinimumScore       = min_score;
  CrossDistance      = max_distance;
  Threshold          = threshold;
  TimeThresholdAfter = 5;
  TrackersAppliedAtRound1.clear();
  ClustersInfoData.clear();

  /* Select which trackers will be used */
  UseDistance = (max_distance > 0);
  UseCallers       = (callers_cfg.size() > 0 ? true : false);
  UseAlignment     = true;
  UseDensity       = false;

  Alignments.clear();
  Histograms.clear();

  FramesMatrix = NULL;

  FramesMatrix = (FramesMatrix_t **)malloc(NumberOfTraces * sizeof(FramesMatrix_t *));
  for (int i=0; i<NumberOfTraces; i++)
  {
    FramesMatrix[i] = (FramesMatrix_t *)malloc(NumberOfTraces * sizeof(FramesMatrix_t));
  }

  for (int i=0; i<NumberOfTraces; i++)
  {
    for (int j=0; j<NumberOfTraces; j++)
    {
      FramesMatrix[i][j].ByCallers   = NULL;
      FramesMatrix[i][j].ByDensity   = NULL;
      FramesMatrix[i][j].ByDistance  = NULL;
      FramesMatrix[i][j].BySequence  = NULL;
      FramesMatrix[i][j].BySPMD = NULL;

      FramesMatrix[i][j].OneWay = NULL;
      FramesMatrix[i][j].TwoWay = NULL;
      FramesMatrix[i][j].Final  = NULL;
      
    }
  }

  PrepareFileNames();
}

Tracking::~Tracking()
{
  if (FramesMatrix != NULL)
  {
    for (int i=0; i<NumberOfTraces; i++)
    {
      for (int j=0; j<NumberOfTraces; j++)
      {
        if (FramesMatrix[i][j].ByCallers   != NULL) delete FramesMatrix[i][j].ByCallers;
        if (FramesMatrix[i][j].ByDensity   != NULL) delete FramesMatrix[i][j].ByDensity;
        if (FramesMatrix[i][j].ByDistance  != NULL) delete FramesMatrix[i][j].ByDistance;
        if (FramesMatrix[i][j].BySequence  != NULL) delete FramesMatrix[i][j].BySequence;
        if (FramesMatrix[i][j].BySPMD != NULL) delete FramesMatrix[i][j].BySPMD;

        if (FramesMatrix[i][j].OneWay      != NULL) delete FramesMatrix[i][j].OneWay;
        if (FramesMatrix[i][j].TwoWay      != NULL) delete FramesMatrix[i][j].TwoWay;
        if (FramesMatrix[i][j].Final       != NULL) delete FramesMatrix[i][j].Final;
      }
      free(FramesMatrix[i]);
      FramesMatrix[i] = NULL;
    }
    free(FramesMatrix);
    FramesMatrix = NULL;
  }

  for (int i=0; i<Alignments.size(); i++)
  {
    delete Alignments[i];
  }
  Alignments.clear();

  for (int i=0; i<Histograms.size(); i++)
  {
    delete Histograms[i];
  }
  Histograms.clear();
}

/**
 * Count the total number of clusters from the clusters_info.csv file
 */
int ReadNumClustersFromFile(string ClustersInfoFileName)
{
  ifstream     ClustersInfoStream;
  string       Token;
  string       Header;
  stringstream HeaderStream;
  int          TotalClusters = 0;

  ClustersInfoStream.open(ClustersInfoFileName.c_str(), ios_base::in);
  if (!ClustersInfoStream)
  {
    cerr << "Error: Unable to open clusters information file '"+ClustersInfoFileName+"'";
    exit(-1);
  }

  getline(ClustersInfoStream, Header); /* Header */

  HeaderStream << Header;
  while (getline(HeaderStream, Token, ','))
  {
    TotalClusters ++;
  }
  TotalClusters -= 2; /* The two first columns are "Cluster name" and "NOISE" */

  return TotalClusters;
}

void Tracking::PrepareFileNames()
{
  for (int i=0; i<NumberOfTraces; i++)
  {
    string CurrentTrace  = InputTraces[i];
    string TraceBaseName = (RemoveExtension(CurrentTrace).c_str());
    string CSVOrig       = TraceBaseName + SUFFIX_ORIG_CSV;
    string CSVNorm       = TraceBaseName + SUFFIX_NORM_CSV;
    string CINFOFile     = TraceBaseName + SUFFIX_CINFO;
    string AlignFile     = TraceBaseName + SUFFIX_ALIGN;
    if (UseCallers)
    {
      string HistoCallersFile = CurrentTrace + "." + RemoveExtension(CallersCFG) + SUFFIX_HISTOGRAM_CALLERS_PER_CLUSTER;
      OutputHistoCallers.push_back( HistoCallersFile );
    }
    OrigCSVs.push_back( CSVOrig );
    InputCSVs.push_back( CSVNorm );
    InputCINFOs.push_back( CINFOFile );
    ClustersInfoData.push_back( new ClustersInfo( CINFOFile ) );
    TotalClusters.push_back( ReadNumClustersFromFile(CINFOFile) );
    InputAlignments.push_back( AlignFile );
    OutputTraces.push_back( TraceBaseName + SUFFIX_TRACKED_TRACE );
  }
  OutputSequenceFile = OutputPrefix + SUFFIX_SEQUENCES;

  /* Write the configuration file for the GUI */
  string GUIConfigFile = OutputPrefix + SUFFIX_GUI_CONFIG;
  ofstream fd(GUIConfigFile.c_str());
  fd << "[Info]" << endl;
  fd << "Frames=" << NumberOfTraces << endl << endl;
  for (int i=0; i<NumberOfTraces; i++)
  {
    fd << "[Frame " << i+1 << "]" << endl;
    fd << "iTrace=" << InputTraces[i] << endl;
    fd << "oTrace=" << OutputTraces[i] << endl;
    fd << "Orig=" << OrigCSVs[i] << endl;
    fd << "Data=" << OrigCSVs[i]+SUFFIX_RECOLORED_CSV << endl;
    if (UseCallers)
    {
      fd << "Callers=" << OutputHistoCallers[i] << endl;
    }
    fd << endl;
  }
  fd.close();
}


void Tracking::CorrelateTraces()
{
  /* Run density, distance, SPMD and callers trackers */
  RunTrackers1();
 
  /* Merge their results */
  CombineTrackers1();

  /* Run the sequence tracker */ 
  RunTrackers2();

  /* Merge with previous results */
  CombineTrackers2();

  BuildFinalSequence();

  Recolor();

  if (Reconstruct)
  {
    ReconstructTraces();
  }

  cout << "+ Tracking finished!" << endl;
}

void Tracking::RunTrackers1()
{
  /* First compute the clusters alignments and the callers per cluster histograms for each trace */
  for (int i = 0; i < NumberOfTraces; i++)
  {
    if (UseAlignment)
    {
      cout << endl << "+ Computing clusters alignment sequence for trace " << i+1 << "..." << endl;

      Alignments.push_back( new ClustersAlignment( InputAlignments[i], InputCINFOs[i] ) );

      cout << endl;
    }
    if (UseCallers)
    {
      cout << "+ Computing histogram of callers per cluster for trace " << i+1 << "..." << endl;
      
      Histograms.push_back( new CallersHistogram( InputTraces[i], CallersCFG, OutputHistoCallers[i]) );
      if (Histograms[i]->size() <= 0)
      {
        cout << "WARNING: Callers histogram for trace " << i+1 << " is empty! Callers tracker will not be used, try increasing the callers level..." << endl;
      }

      cout << endl;
    }
  }

  /* Run the SPMDiness tracker on each trace */
  if (UseAlignment)
  {
    for (int i = 0; i < NumberOfTraces; i++)
    {
      cout << "+ Running SPMDiness tracker on trace " << i+1 << "..." << endl << endl;
      FramesMatrix[i][i].BySPMD = new SPMDTracker( Alignments[i] );
      FramesMatrix[i][i].BySPMD->Track();
    }
  }
  else
  {
    cout << "+ SPMDiness tracker has been disabled" << endl << endl;
  }

  for (int frame1=0; frame1<NumberOfTraces-1; frame1++)
  {
    int frame2 = frame1+1;

    if (UseDensity)
    {
      cout << "+ Running density tracker between traces " << frame1+1 << " and " << frame2+1 << endl << endl;
      FramesMatrix[frame1][frame2].ByDensity = new DensityTracker( ClustersInfoData[frame1], ClustersInfoData[frame2] );
      FramesMatrix[frame1][frame2].ByDensity->Track();
    }
    else
    {
      cout << "+ Density tracker has been disabled" << endl << endl;
    }

    if (UseDistance)
    {
      cout << "+ Running distance tracker between traces " << frame1+1 << " and " << frame2+1 << endl << endl;
      FramesMatrix[frame1][frame2].ByDistance = new DistanceTracker(frame1, InputCSVs[frame1], frame2, InputCSVs[frame2]);
      FramesMatrix[frame1][frame2].ByDistance->Track();

      cout << "+ Running distance tracker between traces " << frame2+1 << " and " << frame1+1 << endl << endl;
      FramesMatrix[frame2][frame1].ByDistance = new DistanceTracker(frame2, InputCSVs[frame2], frame1, InputCSVs[frame1]);
      FramesMatrix[frame2][frame1].ByDistance->Track();
    }
    else
    {
      cout << "+ Distance tracker has been disabled" << endl << endl;
    }

    if ((UseCallers) && (Histograms[frame1]->size() > 0) && (Histograms[frame2]->size() > 0))
    {
      cout << "+ Running callers tracker between traces " << frame1+1 << " and " << frame2+1 << endl << endl;
      FramesMatrix[frame1][frame2].ByCallers = new CallersTracker( Histograms[frame1], Histograms[frame2] );
      FramesMatrix[frame1][frame2].ByCallers->Track();

      cout << "+ Running callers tracker between traces " << frame2+1 << " and " << frame1+1 << endl << endl;
      FramesMatrix[frame2][frame1].ByCallers = new CallersTracker( Histograms[frame2], Histograms[frame1] );
      FramesMatrix[frame2][frame1].ByCallers->Track();
    }
    else
    {
      cout << "+ Callers tracker has been disabled" << endl << endl;
    }
  }
}

void Tracking::CombineTrackers1()
{
  for (int frame1=0; frame1<NumberOfTraces-1; frame1++)
  {
    int frame2 = frame1+1;

    CompareFrames( frame1, frame2 );

    CompareFrames( frame2, frame1 );

    FramesMatrix[frame1][frame2].TwoWay = new DoubleLinks( FramesMatrix[frame1][frame2].OneWay, FramesMatrix[frame2][frame1].OneWay );
  }
}

void Tracking::RunTrackers2()
{
  for (int frame1=0; frame1<NumberOfTraces-1; frame1++)
  {
    int frame2 = frame1+1;

    if ((UseAlignment) && (Alignments[frame1]->Exists()) && (Alignments[frame2]->Exists()))
    {
      double Score1 = Alignments[frame1]->GlobalScore();
      double Score2 = Alignments[frame2]->GlobalScore();

      if ((Score1 < MinimumScore) || (Score2 < MinimumScore))
      {
        cout << "WARNING: Skipping time sequences correlation between traces " << frame1+1 << " and " << frame2+1;
        cout << " due to low scores (Score1=" << Score1 << ", Score2=" << Score2 << ", Minimum= " << MinimumScore << ")" << endl;
      }
      else
      {
        /* Get the links that are univocal from the first round of combinations */
        DoubleLinks *UnivocalLinks = FramesMatrix[frame1][frame2].TwoWay->GetUnivocal();

        if (Verbose)
        {
          cout << "(" << UnivocalLinks->size() << " univocal links)" << endl << endl;
        }
        if ((UnivocalLinks->size() > 0) && (UnivocalLinks->size() < FramesMatrix[frame1][frame2].TwoWay->size()))
        {
          cout << "+ Running sequence tracker between traces " << frame2+1 << " and " << frame1+1 << endl << endl;
          FramesMatrix[frame1][frame2].BySequence = new SequenceTracker( Alignments[frame1], Alignments[frame2], UnivocalLinks );
          FramesMatrix[frame1][frame2].BySequence->Track();
        }
      }
    }
  }
}

void Tracking::CombineTrackers2()
{
  for (int frame1=0; frame1<NumberOfTraces-1; frame1++)
  {
    int frame2 = frame1+1;
    Tracker    *BySequence    = FramesMatrix[frame1][frame2].BySequence;

    if (BySequence != NULL)
    {
      DoubleLinks *SequenceLinks = BySequence->getLinks( Threshold );

      if (SequenceLinks->size() > 0)
      {
        cout << endl << "+ Combining all trackers for frames " << frame1+1 << " and " << frame2+1 << "..." << endl << endl;

        if (Verbose)
        {
          string TrackersApplied = boost::algorithm::join(TrackersAppliedAtRound1, " + ");

          cout << "... 2-way links from " << TrackersApplied << " trackers: " << endl << endl;
          FramesMatrix[frame1][frame2].TwoWay->print();
          cout << endl;
          cout << "... 2-way links from SEQUENCE tracker:" << endl << endl;
          SequenceLinks->print();
          cout << endl;
        }

        FramesMatrix[frame1][frame2].Final = FramesMatrix[frame1][frame2].TwoWay->Split( SequenceLinks );

        cout << "+ Resulting links between frames " << frame1+1 << " and " << frame2+1 << ":" << endl << endl;
        FramesMatrix[frame1][frame2].Final->print();
      }
    }

    if (FramesMatrix[frame1][frame2].Final == NULL)
    {
      FramesMatrix[frame1][frame2].Final = FramesMatrix[frame1][frame2].TwoWay;
      FramesMatrix[frame1][frame2].TwoWay = NULL; /* To avoid double free at destructor */
    }
  }
}

void Tracking::BuildFinalSequence()
{
  cout << endl << "+ Computing the final clusters sequences..." << endl << endl;

  unlink(OutputSequenceFile.c_str());
  ofstream fd;
  fd.open (OutputSequenceFile.c_str(), ios::out | ios::trunc);

  vector<DoubleLinks *> AllPairs;

  for (int frame1=0; frame1<NumberOfTraces-1; frame1++)
  {
    int frame2 = frame1+1;
    AllPairs.push_back ( FramesMatrix[frame1][frame2].Final );
  }

  FinalSequence = new SequenceLink(AllPairs, ClustersInfoData, TimeThresholdAfter);

  cout << "+ Final sequences: " << endl << endl;

  FinalSequence->write(cout, true, true);
  FinalSequence->write(fd,   true, false);

  fd.close();
}

void Tracking::CompareFrames(int frame1, int frame2)
{
  int ClustersInFrame1 = NumClustersToTrack[frame1];

  Tracker *ByCallers   = FramesMatrix[frame1][frame2].ByCallers;
  Tracker *ByDensity   = FramesMatrix[frame1][frame2].ByDensity;
  Tracker *ByDistance  = FramesMatrix[frame1][frame2].ByDistance;
  Tracker *BySPMD = FramesMatrix[frame2][frame2].BySPMD;

  bool DensityApplicable   = ((UseDensity)   && (ByDensity  != NULL));
  bool DistanceApplicable  = ((UseDistance)  && (ByDistance != NULL));
  bool CallersApplicable   = ((UseCallers)   && (ByCallers  != NULL));
  bool AlignmentApplicable = ((UseAlignment) && (Alignments[frame1]->Exists()) && (Alignments[frame2]->Exists()));
  bool SPMDinessApplicable = false;

  if (AlignmentApplicable)
  {
    double GlobalScoreFrame2 = Alignments[frame2]->GlobalScore();

    if (GlobalScoreFrame2 < MinimumScore)
    {
      cout << "WARNING: SPMD tracker ignored for frame " << frame2+1 << " because Global Score=" << GlobalScoreFrame2 << " is below the minimum " << MinimumScore << ")" << endl;
      AlignmentApplicable = false;
    }
    else
    {
      SPMDinessApplicable = true;
    }
  }
  if ((!DensityApplicable) && (!DistanceApplicable) && (!CallersApplicable) && (!SPMDinessApplicable))
  {
    cerr << endl;
    cerr << "ERROR: Any tracker meets the conditions on frames " << frame1+1 << " and " << frame2+1 << "." << endl;
    cerr << "Please review the execution log for previous errors and change the configuration. Aborting..." << endl << endl;
    exit(-1);
  }

  FramesMatrix[frame1][frame2].OneWay = new FrameLinks();

  int ObjectsInFrame1 = NumClustersToTrack[frame1];

  for (ClusterID_t CurrentClusterID = 1; CurrentClusterID <= ObjectsInFrame1; CurrentClusterID ++)
  {
    ObjectLinks *CombinedLinks = new ObjectLinks(CurrentClusterID);

    if (Verbose > 1)
    {
      cout << "[DEBUG] ... Finding links for cluster " << CurrentClusterID << " between frames " << frame1+1 << " and " << frame2+1 << endl;
    }

    /* Retrieve links discovered by the density tracker */
    if (DensityApplicable)
    {
      ObjectLinks *DensityLinks = ByDensity->getLinks( CurrentClusterID, Threshold );
      
      if (Verbose > 1)
      {
        cout << "[DEBUG] ...... According to DENSITY tracker: ";
        DensityLinks->print();
      }

      CombinedLinks->intersect( DensityLinks );
      delete DensityLinks;
      TrackersAppliedAtRound1.insert( "DENSITY" );
    }

    /* Retrieve links discovered by the distance tracker and intersect with the previous */
    if (DistanceApplicable)
    {
      ObjectLinks *DistanceLinks = ByDistance->getLinks( CurrentClusterID, Threshold );

      if (Verbose > 1)
      {
        cout << "[DEBUG] ...... According to DISTANCE tracker: ";
        DistanceLinks->print();
      }

      CombinedLinks->intersect( DistanceLinks );
      delete DistanceLinks;
      TrackersAppliedAtRound1.insert( "DISTANCE" );
    }

    /* For the links found so far, find their SPMD links in the second frame */
    if (SPMDinessApplicable)
    {
      vector< ClusterID_t > ClustersToExpand;
      ObjectLinks_iterator_t it;

      for (it = CombinedLinks->begin(); it != CombinedLinks->end(); ++ it)
      {
        ClusterID_t    TargetClusterID    = *it;
        double TargetClusterScore = Alignments[frame2]->ClusterScore(TargetClusterID);

        if (TargetClusterScore >= MinimumScore)
        {
          ClustersToExpand.push_back( TargetClusterID );
        }
        else
        {
          cout << "WARNING: SPMD links ignored for cluster " << TargetClusterID << " at frame " << frame2 
               << " because SPMD score (" << TargetClusterScore << ") is below minimum (" << MinimumScore 
               << ")" << endl;
        }
      }
      /* Extend the links with the results from the SPMD tracker */
      for (int i = 0; i < ClustersToExpand.size(); i ++)
      {
        ObjectLinks *SPMDLinks = BySPMD->getLinks( ClustersToExpand[i], Threshold );
          
        if (Verbose > 1)
        {
          cout << "[DEBUG] ...... According to SPMD tracker: ";
          SPMDLinks->print();
        }

        CombinedLinks->join( SPMDLinks );
        delete SPMDLinks;
        TrackersAppliedAtRound1.insert( "SPMD" );
      }
    }

    /* Intersect with callers tracker */
    if (CallersApplicable) 
    {
      ObjectLinks *CallersLinks = ByCallers->getLinks(CurrentClusterID, Threshold);

      if (Verbose > 1)
      {
        cout << "[DEBUG] ...... According to CALLERS tracker: ";
        CallersLinks->print();
      }

      if (CallersLinks->size() > 0)
      {
        CombinedLinks->intersect(CallersLinks);
    
        /* In case of disagreement or if callers are unique and not empty, trust callers */
        if ((CombinedLinks->size() <= 0) || (CallersLinks->size() == 1))
        {
          delete CombinedLinks;
          CombinedLinks = CallersLinks;
          TrackersAppliedAtRound1.clear();
        } 
        else
        {
          delete CallersLinks;
        }
        TrackersAppliedAtRound1.insert( "CALLERS" );
      }
    }

    if (Verbose > 1)
    {
      cout << "[DEBUG] ... Trackers agreement: ";
      CombinedLinks->print();
      cout << endl;
    }

    /* Store the links for the current cluster */
    FramesMatrix[frame1][frame2].OneWay->add( CombinedLinks );
  }

  if (Verbose)
  {
    cout << "+ 1-way links from frame " << frame1+1 << " to " << frame2+1 << ":" << endl << endl;
    FramesMatrix[frame1][frame2].OneWay->print();
    cout << endl;
  }
}

void Tracking::Recolor()
{
  vector<string> InPlots;

  /* Clean previous files */
  for (int i=0; i<InputTraces.size(); i++)
  {
    string TraceBaseName (RemoveExtension(InputTraces[i]).c_str());
    string ScaledPlot = TraceBaseName + "*" + SUFFIX_SCALED_PLOT;

    InPlots.push_back ( ScaledPlot );
  }
  string CMD = string(getenv("TRACKING_HOME")) + "/bin/recoloring.pl ";
  for (int i=0; i<InPlots.size(); i++)
  {
    CMD += InPlots[i] + " ";
  }
  CMD += OutputPrefix;
  cout << "+ Recoloring plots... " << endl; cout.flush();
  if (Verbose > 1)
  {
    cout << "[DEBUG] CMD=" << CMD << endl;
  }
  system(CMD.c_str());
  cout << endl;
}

void Tracking::ReconstructTraces()
{
  vector< map<TTypeValuePair, TTypeValuePair> > AllTranslationTables;

  for (int i=0; i<NumberOfTraces; i++)
  {
    map<TTypeValuePair, TTypeValuePair> CurrentTraceTranslationTable;

    FinalSequence->GetTranslationTable(i, TotalClusters[i], CurrentTraceTranslationTable);
    AllTranslationTables.push_back( CurrentTraceTranslationTable );
  }

  TraceReconstructor *TR = new TraceReconstructor( InputTraces, OutputTraces, AllTranslationTables );
}

#if 0
// void Tracking::GeneratePlots()
// {
//   cout << "+ Generating plots...\n";
//   cout.flush();
//   string CMD = "perl -I " + string(getenv("TRACKING_HOME")) + "/bin " + string(getenv("TRACKING_HOME")) + "/bin/generate-plots.pl ";
//   for (int i=0; i<InputTraces.size(); i++)
//   {
//     string TraceBaseName (RemoveExtension(InputTraces[i]).c_str());
//     CMD += TraceBaseName + " ";
//   }
//   CMD += OutputPrefix;
//   cout << CMD << endl;
//   system(CMD.c_str());
//   cout << "Plotting done!" << endl << endl;
// }
#endif 

