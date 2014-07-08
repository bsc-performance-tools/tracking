#include <iostream>
#include <libgen.h>
#include "ClustersAlignment.h"
#include "Utils.h"

using std::cerr;
using std::cout;
using std::endl;

ClustersAlignment::ClustersAlignment(string AlignmentFile, string ClustersInfoFile)
{
  SS          = NULL;
  ScoreGlobal = 0.0;
  ScorePerCluster.clear();

  if (FileExists(AlignmentFile) && FileExists(ClustersInfoFile))
  {
    vector<INT32> Appearances;

    SS = new SequenceScoring(AlignmentFile);
    SS->LoadClustersInformation(ClustersInfoFile);
    SS->GetScores(ScorePerCluster, Appearances);

    if (!SS->GetGlobalScore(ScorePerCluster, ScoreGlobal))
    {
      cout << "ERROR: Unable to compute global score";
      cout << " (" << SS->GetLastError() << ")" << endl << endl;

      delete SS;
      SS          = NULL;
      ScoreGlobal = 0.0;
    }
  }
  else
  {
    cout << "WARNING: Time sequence not available (Alignment file '" << basename((char *)(AlignmentFile.c_str())) << "' not found)" << endl << endl;
  }
}

ClustersAlignment::~ClustersAlignment()
{
  if (SS != NULL)
  {
    delete SS;
  }

  SS          = NULL;
  ScoreGlobal = 0.0;
  ScorePerCluster.clear();
}

bool ClustersAlignment::Exists()
{
  return (SS != NULL);
}

int ClustersAlignment::NumberOfClusters()
{
  return ScorePerCluster.size() - 1; /* XXX -1 assumes there's always NOISE in index 0 */
}

double ClustersAlignment::ClusterScore(ClusterID_t cluster_id)
{
  if ((cluster_id < 1) || (cluster_id > ScorePerCluster.size()))
  {
    cerr << "WARNING: ClustersAlignment::ClusterScore(): Invalid Cluster ID '" << cluster_id << "' (Valid IDs from 1 to " << ScorePerCluster.size() << ")" << endl;
    return 0;
  }
  return ScorePerCluster[cluster_id]; /* XXX Assumes index 0 is for NOISE */
}

double ClustersAlignment::GlobalScore()
{
  return ScoreGlobal;
}

SequenceScoring * ClustersAlignment::Scoring()
{
  return SS;
}

void ClustersAlignment::Print()
{
  cout << "Global Score = " << ScoreGlobal << endl;
  cout << "Number of Clusters = " << NumberOfClusters() << endl;
  for (int i=1; i<=NumberOfClusters(); i++)
  {
    cout << "Score for Cluster " << i << " = " << ClusterScore(i) << endl;
  }
}

