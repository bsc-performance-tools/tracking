#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <stdlib.h>
#include "ClusterCorrelationMatrix.h"
#include "ClusterIDs.h"

/**
 * Correlate the sequence of cluster in vector list_ids_1 to the clusters in list_ids_2. 
 * Number of clusters in both lists have to be the same. This function is called to 
 * build a cluster correlation by cross-classification, where the number of points  
 * considered is the same for two clusterings, thus the size of the input vector is the same.
 */ 
ClusterCorrelationMatrix::ClusterCorrelationMatrix(vector<CID> list_ids_1, vector<CID> list_ids_2)
{
  ApplyOffset = false;
  N = M = 0;
  CorrelationMatrix = NULL;

  if (list_ids_1.size() == list_ids_2.size())
  {
    /* Find how many different clusters there are in each clustering */
    for (unsigned int i=0; i<list_ids_1.size(); i++)
    {
      N = (N > list_ids_1[i] ? N : list_ids_1[i]);
      M = (M > list_ids_2[i] ? M : list_ids_2[i]);
    }
    N ++; M ++;

    /* DEBUG 
    cerr << "[DEBUG] ClusterCorrelationMatrix: N=" << N << " M=" << M << endl; */

    /* Allocate the matrix */
    Allocate();

    /* Increase counts for clusters equivalences list_ids_1 --> list_ids_2 */
    for (unsigned int i=0; i<list_ids_1.size(); i++)
    {
      Hit(list_ids_1[i],list_ids_2[i]);
    }
    ComputePcts();
  }
  else
  {
    cerr << "ClusterCorrelationMatrix: Error: Input vectors sizes differ! ";
    cerr << "(list_ids_1.size()=" << list_ids_1.size() << ", list_ids_2.size()=" << list_ids_2.size() << ")" << endl;
  }
}

ClusterCorrelationMatrix::ClusterCorrelationMatrix(int n, int m)
{
  ApplyOffset = true;
  N = n + FIRST_CLUSTER;
  M = m + FIRST_CLUSTER;
  CorrelationMatrix = NULL;

  /* Allocate the matrix */
  Allocate();
}

/**
 * Free the correlation matrix.
 */
ClusterCorrelationMatrix::~ClusterCorrelationMatrix()
{
  if (CorrelationMatrix != NULL)
  {
    for (int i=0; i<N; i++)
    {
      free(CorrelationMatrix[i]);
    }
    free(CorrelationMatrix);
  }
}

void ClusterCorrelationMatrix::Allocate()
{
  /* Allocate the matrix */
  CorrelationMatrix = (TCorrelationStats **)malloc(sizeof(TCorrelationStats *) * N);
  for (int i=0; i<N; i++)
  {
    CorrelationMatrix[i] = (TCorrelationStats *)malloc(sizeof(TCorrelationStats) * M);
    for (int j=0; j<M; j++)
    {
      TCorrelationStats stats;
      stats.SeenCount  = 0;
      stats.Likeliness = 0;
      CorrelationMatrix[i][j] = stats;
    }
  }
}

void ClusterCorrelationMatrix::Hit(int i, int j, int value)
{
  if (ApplyOffset)
  {
    i += FIRST_CLUSTER - 1;
    j += FIRST_CLUSTER - 1;
  }
  CorrelationMatrix[i][j].SeenCount += value;
}

void ClusterCorrelationMatrix::SetStats(int i, int j, int seen, double pct)
{
  if (ApplyOffset)
  {
    i += FIRST_CLUSTER - 1;
    j += FIRST_CLUSTER - 1;
  }
  CorrelationMatrix[i][j].SeenCount  = seen;
  CorrelationMatrix[i][j].Likeliness = pct;
}

void ClusterCorrelationMatrix::ComputePcts()
{
  for (int i=0; i<N; i++)
  {
    int TotalSeenCount = 0;

    for (int j=0; j<M; j++)
    {
      TotalSeenCount += CorrelationMatrix[i][j].SeenCount;
    }
    for (int j=0; j<M; j++)
    {
      CorrelationMatrix[i][j].Likeliness = (CorrelationMatrix[i][j].SeenCount * 100) / TotalSeenCount;
    }
  }
}

/**
 * Print the correlation matrix in the standard output.
 */
void ClusterCorrelationMatrix::Print()
{
  for (int i=FIRST_CLUSTER; i<N; i++)
  {
    for (int j=FIRST_CLUSTER; j<M; j++)
    {
      fprintf(stdout, "%5d ", CorrelationMatrix[i][j].SeenCount);
    }
    fprintf(stdout, "\n");
  }
    fprintf(stdout, "\n");
}

/**
 * Print statistics showing with percentages how clusters correlate.
 */ 
void ClusterCorrelationMatrix::Stats()
{
  for (int i=FIRST_CLUSTER; i<N; i++)
  {
    int CorrelatesInto = 0;

    for (int j=FIRST_CLUSTER; j<M; j++)
    {
      if (getCorrelationSeenCount(i, j) > 0) CorrelatesInto ++;
    }

    if (CorrelatesInto > 0)
    {
      fprintf(stdout, "Cluster %d correlates to %d cluster%s\n", CLUSTER_ID(i), CorrelatesInto, (CorrelatesInto > 1 ? "s" : ""));
      for (int j=0; j<M; j++)
      {
        if (getCorrelationSeenCount(i, j) > 0)
        fprintf(stdout, " * %.2f%% into cluster %d (hits=%d)\n", CorrelationMatrix[i][j].Likeliness, CLUSTER_ID(j), CorrelationMatrix[i][j].SeenCount);
      }
    }
  }
  fprintf(stdout, "\n");
} 

/**
 * If min_likeliness is set to -1, it returns the correlation with maximum percentage. Otherwise,
 * it returns all correlations above the specified percentage. 
 */
OneWayCorrelation * ClusterCorrelationMatrix::getCorrelation(int cluster_id, double min_likeliness)
{
  OneWayCorrelation *ClusterCorrelation = new OneWayCorrelation(cluster_id);

  int i = cluster_id - 1 + FIRST_CLUSTER;

  if (i >= N) 
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl; 
    return ClusterCorrelation; 
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }

  CID MaxCluster;
  double max_percentage = -1;
  for (int j=FIRST_CLUSTER; j<M; j++)
  {
    if ((getCorrelationSeenCount(i, j) > 0) && (getCorrelationLikeliness(i, j) >= min_likeliness))
    {
      if (min_likeliness == -1)
      {
        /* Look for the correlation with maximum percentage only */
        if (getCorrelationLikeliness(i, j) > max_percentage)
        { 
          max_percentage = getCorrelationLikeliness(i, j);
          MaxCluster    = j;
        }
      }
      else
      {
        /* Return all correlations above the percentage */
        ClusterCorrelation->add(CLUSTER_ID(j));
      }
    }
  }
  if ((min_likeliness == -1) && (max_percentage > -1))
  {
    /* Return the correlation with maximum percentage only */
    ClusterCorrelation->add(CLUSTER_ID(MaxCluster));
  }

  return ClusterCorrelation;
}



int ClusterCorrelationMatrix::getCorrelationSeenCount(int fromCluster, int toCluster)
{
  return CorrelationMatrix[fromCluster][toCluster].SeenCount;
} 

double ClusterCorrelationMatrix::getCorrelationLikeliness(int fromCluster, int toCluster)
{
  return CorrelationMatrix[fromCluster][toCluster].Likeliness;
} 

int ClusterCorrelationMatrix::getNumberOfClusters()
{
  return N-FIRST_CLUSTER;
}

