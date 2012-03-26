#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
#include <string>
#include <stdlib.h>
#include <libgen.h>
#include "CallersTracker.h"
#include "ClusterIDs.h"
#include "Utils.h"

CallersTracker::CallersTracker()
{
  LocalKernel::init();
  myKernel = new LocalKernel( NULL );
  myConfig = ParaverConfig::getInstance();
  myConfig->readParaverConfigFile();
}

Histo3D * CallersTracker::Compute3D(string strTrace, string strCFG)
{
  Trace   *trace    = NULL;
  Histo3D *trace3DH = new Histo3D();

  string CFGBaseName = RemoveExtension(strCFG);
  string Suffix = CFGBaseName + ".3D";
  string SavedHistogramFileName = strTrace + "." + Suffix;

  if (trace3DH->load(SavedHistogramFileName))
  {
    /* Data loaded from file */
    trace3DH->dump();
    return trace3DH;
  }
 
  /* Load trace and compute 3D with Paraver kernel */
  if (!loadTrace(myKernel, strTrace, trace))
  {
    cerr << "Error loading trace '" << strTrace << "'" << endl;
    exit(0);
  }

  vector<Window *>    windows;
  vector<Histogram *> histograms;
  SaveOptions         dummy_options;

  if (CFGLoader::loadCFG( myKernel, strCFG, trace, windows, histograms, dummy_options ))
  {
    parseHistogram(histograms[0], trace3DH);
    trace3DH->dump();
    /* Store the histogram so it doesn't need to be computed again */
    trace3DH->write(SavedHistogramFileName);
  }
  else
  {
    cerr << "Error loading cfg  '" << strCFG << "'" << endl;
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
  return trace3DH;
}

bool CallersTracker::loadTrace( KernelConnection *myKernel, string strTrace, Trace * & trace )
{
  bool loaded = false;

  try
  {
    trace = Trace::create( myKernel, strTrace, false, NULL );
    loaded = true;
  }
  catch ( ParaverKernelException& ex )
  {
    ex.printMessage();
  }

  return loaded;
}


void CallersTracker::parseHistogram(Histogram * histo, Histo3D *&H)
{
  /* Configure the histogram */
  vector<TObjectOrder> selectedRows;
  histo->getControlWindow()->getSelectedRows(histo->getControlWindow()->getLevel(), selectedRows);
  histo->setCompute2DScale(false);
  histo->compute2DScale();
  histo->setControlMin(3);
  histo->execute( histo->getBeginTime(), histo->getEndTime(), selectedRows );

  /* Select the histogram metric */
  HistogramTotals *histototals = histo->getColumnTotals();
  PRV_UINT16 stat_nbursts;
  if (!histo->getIdStat("# Bursts", stat_nbursts))
  {
    cerr << "histo->getIdStat(\"# Bursts\") failed!" << endl;
    exit(-1);
  }

  /* Iterate the planes (clusters) */
  for (THistogramColumn iPlane = 0; iPlane < histo->getNumPlanes(); ++iPlane)
  {
    if (histo->planeWithValues(iPlane))
    {
      TSemanticValue TotalBurstsPlane = 0, CurrentCallerBursts = 0;

      /* Iterate the columns (callers) */
      for (THistogramColumn iCol = 0; iCol < histo->getNumColumns(); ++iCol)
      {
        histo->setFirstCell(iCol, iPlane);
        if (!histo->endCell(iCol, iPlane))
        {
          /* Accumulate the total number of callers for this cluster to compute percentages */
          TotalBurstsPlane += histototals->getTotal(stat_nbursts, iCol, iPlane);
        }
      }
      /* Iterate the columns (callers) */
      for (THistogramColumn iCol = 0; iCol < histo->getNumColumns(); ++iCol)
      {
        histo->setFirstCell(iCol, iPlane);
    
        if (!histo->endCell(iCol, iPlane))
        {
          /* Get the total # bursts for this caller */
          CurrentCallerBursts = histototals->getTotal(stat_nbursts, iCol, iPlane);
  
          /* DEBUG
          cout << histo->getPlaneLabel(iPlane) << " - " << histo->getColumnLabel(iCol) << " = ";
          cout << CurrentCallerBursts << " (" << CurrentCallerBursts*100/TotalBurstsPlane << "%)" << endl; */

          /* Store current caller for current cluster */
          CID CurrentCluster = iPlane - FIRST_CLUSTER + 1;  
          TCaller ClusterCaller;
          ClusterCaller.CallerName = histo->getColumnLabel(iCol);
          ClusterCaller.NumBursts  = (int)CurrentCallerBursts;
          ClusterCaller.Pct        = CurrentCallerBursts*100/TotalBurstsPlane;
  
          if (CurrentCluster > 0) H->insert(CurrentCluster, ClusterCaller);
        }
      }
    }
  }
}

ClusterCorrelationMatrix * CallersTracker::CrossCallers(Histo3D *H1, Histo3D *H2)
{
  ClusterCorrelationMatrix * CCM = new ClusterCorrelationMatrix(H1->size(), H2->size());

  for (int CurrentCluster = 1; CurrentCluster <= H1->size(); CurrentCluster++)
  {
    vector<TCaller> ClusterCallers;

    H1->getCallersForCluster(CurrentCluster, ClusterCallers);

    /* DEBUG
    cout << "[DEBUG] CallersTracker::CrossCallers " << CurrentCluster << " --> "; */

    for (int i=0; i<ClusterCallers.size(); i++)
    {
      TCaller CurrentCaller = ClusterCallers[i];
      vector<CID> CorrespondingClusters;

      H2->getClustersWithCaller(CurrentCaller.CallerName, CorrespondingClusters);

      for (int j=0; j<CorrespondingClusters.size(); j++)
      {
        /* DEBUG
        cout << CorrespondingClusters[j] << " "; */

        CCM->Hit(CurrentCluster, CorrespondingClusters[j]);
      }
    }
    /* DEBUG
    cout << endl; */
  }
  CCM->ComputePcts();

  return CCM;
}


void Histo3D::insert(CID key, TCaller value)
{
  H[key].push_back( value );
}

int Histo3D::size()
{
  return H.size();
}

void Histo3D::dump()
{
  map< CID, vector<TCaller> >::iterator it;
  for (it = H.begin(); it != H.end(); ++it)
  {
    CID key = it->first;
    cout << "Cluster " << key << " (" << it->second.size() << " caller";
    if (it->second.size() > 1) cout << "s";
    cout << "):" << endl;
    for (int i=0; i<it->second.size(); i++)
    {
      TCaller value = it->second[i];
      cout << " * " << value.CallerName << " #=" << value.NumBursts << " %=" << value.Pct << endl;
    }
  }
}

void Histo3D::write(string filename)
{
  ofstream file;
  file.open (filename.c_str());

  map< CID, vector<TCaller> >::iterator it;
  for (it = H.begin(); it != H.end(); ++it)
  {
    CID key = it->first;
    for (int i=0; i<it->second.size(); i++)
    {
       TCaller value = it->second[i];
       file << key << "," << value.CallerName << "," << value.NumBursts << "," << value.Pct << endl;
    }
  }
  file.close();
}

bool Histo3D::load(string filename)
{
  ifstream file;
  file.open (filename.c_str());

  if (file)
  {
    string token, line;
    stringstream iss;

    while ( getline(file, line) )
    {
      CID     key;
      TCaller value;
      
      iss << line;
      getline(iss, token, ',');
      istringstream ssCID(token);
      (ssCID >> key);
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

void Histo3D::getClusters(vector<CID> &clusters)
{
  map< CID, vector<TCaller> >::iterator it;

  for (it = H.begin(); it != H.end(); ++ it)
  {
    clusters.push_back( it->first );
  }
}

void Histo3D::getCallersForCluster(CID cluster, vector<TCaller> &callers)
{
  for (int i=0; i<H[cluster].size(); i++)
  {
    callers.push_back( H[cluster][i] );
  }
}

void Histo3D::getClustersWithCaller(string caller_name, vector<CID> &clusters)
{
  map< CID, vector<TCaller> >::iterator it;
  
  for (it = H.begin(); it != H.end(); ++ it)
  {
    CID key = it->first;
    for (int i = 0; i < it->second.size(); i ++)
    {
      TCaller CurrentCaller = it->second[i];
      if (CurrentCaller.CallerName == caller_name)
      {
        clusters.push_back( it->first );
        break;
      }
    }
  }
}


