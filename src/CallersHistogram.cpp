#include <iostream>
#include <fstream>
#include <sstream>
#include <libgen.h>
#include <stdlib.h>
#include "CallersHistogram.h"
#include "ClusterIDs.h"
#include "Utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::istringstream;
using std::stringstream;

CallersHistogram::CallersHistogram(string trace_name, string callers_cfg, string savefile)
{
  Tracename          = trace_name;
  CFG                = callers_cfg;
  SavedHistogramFile = savefile; 
  CallersPerClusterTable.clear();
  myKernel           = NULL;
  myConfig           = NULL;

  LocalKernel::init();

  myKernel = new LocalKernel( NULL );
  myConfig = ParaverConfig::getInstance();
  myConfig->readParaverConfigFile();

  compute3D();
}

CallersHistogram::~CallersHistogram()
{
  delete myKernel;
  CallersPerClusterTable.clear();
  myKernel           = NULL;
  myConfig           = NULL;
}

bool CallersHistogram::load(string filename)
{
  ifstream file;
  file.open (filename.c_str());

  if (file)
  {
    string token, line;
    stringstream iss;

    getline(file, line); // Header
    
    while ( getline(file, line) )
    {
      ClusterID_t     key;
      Caller_t value;
      
      iss << line;
      getline(iss, token, ',');
      istringstream ssClusterID_t(token);
      (ssClusterID_t >> key);
      getline(iss, token, ',');
      value.CallerName = token;
      getline(iss, token, ',');
      istringstream ssNumBursts(token);
      (ssNumBursts >> value.NumBursts);
      getline(iss, token, ',');
      istringstream ssPct(token);
      (ssPct >> value.Pct);
      iss.clear();
 
      insert(key, value);
    }
    return true;
  }
  return false;
}

bool CallersHistogram::load(Trace *&trace)
{
  bool loaded = false;

  try
  {
    trace  = Trace::create(myKernel, Tracename, false, NULL);
    loaded = true;
  }
  catch (ParaverKernelException& ex)
  {
    ex.printMessage();
  }

  return loaded;
}

void CallersHistogram::parseHistogram(Histogram *histogram)
{
  /* Configure the histogram */
  vector<TObjectOrder> selectedRows;
  histogram->getControlWindow()->getSelectedRows(histogram->getControlWindow()->getLevel(), selectedRows);
  histogram->setCompute2DScale(false);
  histogram->compute2DScale();
  if (histogram->getControlMax() < FIRST_VALID_CALLER)
  {
    /* There are no events with value above 2, this means all callers are Unresolved or Not found! */
    cout << "WARNING: CallersHistogram::parseHistogram: Callers are unresolved!" << endl;
    cout.flush();
    return;
  }
  histogram->setControlMin(FIRST_VALID_CALLER);
  histogram->setControlDelta(1);
  histogram->execute( histogram->getBeginTime(), histogram->getEndTime(), selectedRows );

  /* Select the histogram metric */
  HistogramTotals *histogram_totals = histogram->getColumnTotals();
  PRV_UINT16 stat_nbursts;
  if (!histogram->getIdStat("# Bursts", stat_nbursts))
  {
    cerr << "ERROR: CallersHistogram::parseHistogram: histogram->getIdStat(\"# Bursts\") failed!" << endl;
    exit(-1);
  }

  /* Iterate the planes (clusters) */
  for (THistogramColumn iPlane = 0; iPlane < histogram->getNumPlanes(); ++iPlane)
  {
    if (histogram->planeWithValues(iPlane))
    {
      TSemanticValue TotalBurstsPlane = 0, CurrentCallerBursts = 0;

      /* Iterate the columns (callers) */
      for (THistogramColumn iCol = 0; iCol < histogram->getNumColumns(); ++iCol)
      {
        histogram->setFirstCell(iCol, iPlane);
        if (!histogram->endCell(iCol, iPlane))
        {
          /* Accumulate the total number of callers for this cluster to compute percentages */
          TotalBurstsPlane += histogram_totals->getTotal(stat_nbursts, iCol, iPlane);
        }
      }
      /* Iterate the columns (callers) */
      for (THistogramColumn iCol = 0; iCol < histogram->getNumColumns(); ++iCol)
      {
        histogram->setFirstCell(iCol, iPlane);
    
        if (!histogram->endCell(iCol, iPlane))
        {
          /* Get the total # bursts for this caller */
          CurrentCallerBursts = histogram_totals->getTotal(stat_nbursts, iCol, iPlane);
  
          /* DEBUG 
          cout << histogram->getPlaneLabel(iPlane) << " (" << iPlane << ") " << " - " << histogram->getColumnLabel(iCol) << " = ";
          cout << CurrentCallerBursts << " (" << CurrentCallerBursts*100/TotalBurstsPlane << "%)" << endl; */

          ClusterID_t CurrentCluster;
          if (sscanf(histogram->getPlaneLabel(iPlane).c_str(), "Cluster %d", &CurrentCluster) == 1)
          {
            /* Store current caller for current cluster */
            Caller_t ClusterCaller;
            ClusterCaller.CallerName = histogram->getColumnLabel(iCol);
            ClusterCaller.NumBursts  = (int)CurrentCallerBursts;
            ClusterCaller.Pct        = CurrentCallerBursts*100/TotalBurstsPlane;
            insert(CurrentCluster, ClusterCaller);
          }
        }
      }
    }
  }
}

void CallersHistogram::compute3D()
{
  /* Try loading the histogram from file if it's been computed before */
  if (load(SavedHistogramFile))
  {
    print();
  }
  /* Load the trace and compute the histogram */
  else
  {
    Trace              *trace = NULL;
    vector<Window *>    windows;
    vector<Histogram *> histograms;
    SaveOptions         dummy_options;

    if (!load(trace))
    {
      cerr << "ERROR: CallersHistogram::Compute3D: Problem loading trace '" << Tracename << "'" << endl;
      exit(0);
    }
    if (CFGLoader::loadCFG(myKernel, CFG, trace, windows, histograms, dummy_options))
    {
      parseHistogram(histograms[0]);
      print();
      /* Store the histogram so it doesn't need to be computed again */
      write(SavedHistogramFile);
    }
    else
    {
      cerr << "ERROR: CallersHistogram::Compute3D: Can not load configuration file '" << CFG << "'" << endl;
      exit(0);
    }
    for (vector<Histogram *>::iterator it = histograms.begin(); it != histograms.end(); ++it)
    {
      delete *it;
    }
    histograms.clear();
    for (vector<Window *>::iterator it = windows.begin(); it != windows.end(); ++it)
    {
      delete *it;
    }
    windows.clear();
    delete trace;
  }
}


int CallersHistogram::size()
{
  int max = -1;
  map< ClusterID_t, vector<Caller_t> >::iterator it;

  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++it)
  {
    if (it->first > max)
    {
      max = it->first;
    }
  }
  return max;
}

void CallersHistogram::insert(ClusterID_t cluster, Caller_t caller)
{
  CallersPerClusterTable[cluster].push_back( caller );
}

void CallersHistogram::write(string filename)
{
  ofstream file;
  file.open (filename.c_str());

  file << "ClusterID, Caller, Hits, Pct" << endl;
  map< ClusterID_t, vector<Caller_t> >::iterator it;
  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++it)
  {
    ClusterID_t key = it->first;
    for (int i=0; i<it->second.size(); i++)
    {
       Caller_t value = it->second[i];
       file << key << "," << value.CallerName << "," << value.NumBursts << "," << value.Pct << endl;
    }
  }
  file.close();
}

void CallersHistogram::print()
{
  map< ClusterID_t, vector<Caller_t> >::iterator it;
  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++it)
  {
    ClusterID_t key = it->first;
    cout << "Cluster " << key << " (" << it->second.size() << " caller";
    if (it->second.size() > 1) cout << "s";
    cout << "):" << endl;
    for (int i=0; i<it->second.size(); i++)
    {
      Caller_t value = it->second[i];
      cout << " * " << value.CallerName << " #=" << value.NumBursts << " %=" << value.Pct << endl;
    }
  }
}

void CallersHistogram::getClusters(vector<ClusterID_t> &clusters)
{
  map< ClusterID_t, vector<Caller_t> >::iterator it;

  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++ it)
  {
    clusters.push_back( it->first );
  }
}

bool CallersHistogram::hasCallers(ClusterID_t cluster)
{
  return (CallersPerClusterTable.find(cluster) != CallersPerClusterTable.end());
}

void CallersHistogram::getCallersForCluster(ClusterID_t cluster, vector<Caller_t> &callers)
{
  for (int i=0; i<CallersPerClusterTable[cluster].size(); i++)
  {
    callers.push_back( CallersPerClusterTable[cluster][i] );
  }
}

void CallersHistogram::getClustersWithCaller(string caller_name, vector<ClusterID_t> &clusters)
{
  map< ClusterID_t, vector<Caller_t> >::iterator it;
  
  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++ it)
  {
    ClusterID_t key = it->first;
    for (int i = 0; i < it->second.size(); i ++)
    {
      Caller_t CurrentCaller = it->second[i];
      if (CurrentCaller.CallerName == caller_name)
      {
        clusters.push_back( it->first );
        break;
      }
    }
  }
}

bool CallersHistogram::getStatsForClusterAndCaller(ClusterID_t cluster, string caller_name, Caller_t &caller_stats)
{
  map< ClusterID_t, vector<Caller_t> >::iterator it;

  for (it = CallersPerClusterTable.begin(); it != CallersPerClusterTable.end(); ++ it)
  {
    ClusterID_t key = it->first;

    if (key == cluster)
    {
      for (int i = 0; i < it->second.size(); i ++)
      {
        Caller_t CurrentCaller = it->second[i];
        if (CurrentCaller.CallerName == caller_name)
        {
          caller_stats = CurrentCaller;
          return true;
        }
      }
    }
  }
  return false;
}

