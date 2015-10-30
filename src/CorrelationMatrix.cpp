#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <stdlib.h>
#include "CorrelationMatrix.h"
#include "ClusterIDs.h"

/**
 * Correlate the sequence of cluster in vector list_ids_1 to the clusters in list_ids_2. 
 * Number of clusters in both lists have to be the same. This function is called to 
 * build a cluster correlation by cross-classification, where the number of points  
 * considered is the same for two clusterings, thus the size of the input vector is the same.
 */ 
CorrelationMatrix::CorrelationMatrix(vector<ClusterID_t> list_ids_1, vector<ClusterID_t> list_ids_2)
{
  ApplyOffset = false;
  N = M = 0;
  Matrix = NULL;

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
    cerr << "[DEBUG] Matrix: N=" << N << " M=" << M << endl; */

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
    cerr << "CorrelationMatrix: Error: Input vectors sizes differ! ";
    cerr << "(list_ids_1.size()=" << list_ids_1.size() << ", list_ids_2.size()=" << list_ids_2.size() << ")" << endl;
  }
}

CorrelationMatrix::CorrelationMatrix(int n, int m)
{
  ApplyOffset = true;
  N = n + FIRST_CLUSTER;
  M = m + FIRST_CLUSTER;
  Matrix = NULL;

  /* Allocate the matrix */
  Allocate();
}

/**
 * Free the correlation matrix.
 */
CorrelationMatrix::~CorrelationMatrix()
{
  if (Matrix != NULL)
  {
    for (int i=0; i<N; i++)
    {
      free(Matrix[i]);
    }
    free(Matrix);
  }
}

void CorrelationMatrix::Allocate()
{
  /* Allocate the matrix */
  Matrix = (TCorrelationStats **)malloc(sizeof(TCorrelationStats *) * N);
  for (int i=0; i<N; i++)
  {
    Matrix[i] = (TCorrelationStats *)malloc(sizeof(TCorrelationStats) * M);
    for (int j=0; j<M; j++)
    {
      TCorrelationStats stats;
      stats.SeenCount  = 0;
      stats.Likeliness = 0;
      Matrix[i][j] = stats;
    }
  }
}

void CorrelationMatrix::Hit(int i, int j, int value)
{
  if (ApplyOffset)
  {
    i += FIRST_CLUSTER - 1;
    j += FIRST_CLUSTER - 1;
  }
  Matrix[i][j].SeenCount += value;
}

void CorrelationMatrix::SetStats(int i, int j, int seen, double pct)
{
  if (ApplyOffset)
  {
    i += FIRST_CLUSTER - 1;
    j += FIRST_CLUSTER - 1;
  }
  Matrix[i][j].SeenCount  = seen;
  Matrix[i][j].Likeliness = pct;
}

void CorrelationMatrix::RaisePct(int i, int j, double pct)
{
  if (ApplyOffset)
  {
    i += FIRST_CLUSTER - 1;
    j += FIRST_CLUSTER - 1;
  }
  Matrix[i][j].SeenCount ++;
  Matrix[i][j].Likeliness += pct;
}

void CorrelationMatrix::ComputePcts()
{
  for (int i=0; i<N; i++)
  {
    int TotalSeenCount = 0;

    for (int j=0; j<M; j++)
    {
      TotalSeenCount += Matrix[i][j].SeenCount;
    }
    for (int j=0; j<M; j++)
    {
      if (TotalSeenCount <= 0)
        Matrix[i][j].Likeliness = 0;
      else
        Matrix[i][j].Likeliness = (Matrix[i][j].SeenCount * 100) / TotalSeenCount;
    }
  }
}

/**
 * Print the correlation matrix in the standard output.
 */
void CorrelationMatrix::Print()
{
  for (int i=FIRST_CLUSTER; i<N; i++)
  {
    for (int j=FIRST_CLUSTER; j<M; j++)
    {
      fprintf(stdout, "%5d ", Matrix[i][j].SeenCount);
    }
    fprintf(stdout, "\n");
  }
    fprintf(stdout, "\n");
}

/**
 * Print statistics showing with percentages how clusters correlate.
 */ 
void CorrelationMatrix::Stats()
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
      fprintf(stdout, " ▄ Cluster %d matches to %d cluster%s\n", CLUSTER_ID(i), CorrelatesInto, (CorrelatesInto > 1 ? "s" : ""));
      for (int j=0; j<M; j++)
      {
        if (getCorrelationSeenCount(i, j) > 0)
        fprintf(stdout, " └─ %.2f%% into cluster %d (hits=%d)\n", Matrix[i][j].Likeliness, CLUSTER_ID(j), Matrix[i][j].SeenCount);
      }
    }
    else
    {
      fprintf(stdout, " ░ Cluster %d did not match to anybody\n", CLUSTER_ID(i));
    }
  }
  fprintf(stdout, "\n");
} 

/**
 * If min_likeliness is set to -1, it returns the correlation with maximum percentage. Otherwise,
 * it returns all correlations above the specified percentage. 
 */

DoubleLinks * CorrelationMatrix::getCorrelations(double min_likeness)
{
  int i, j;

  ObjectLinks *L = NULL;
  FrameLinks *ForwardLinks = new FrameLinks();
  FrameLinks *BackwardLinks = new FrameLinks();

  for (i=FIRST_CLUSTER; i<N; i++)
  {
    L = getCorrelation(CLUSTER_ID(i), min_likeness);
    if (L->size() > 0)
      ForwardLinks->add( L );
  }

  for (j=FIRST_CLUSTER; j<M; j++)
  {
    L = getReverseCorrelation(CLUSTER_ID(j), min_likeness);
    if (L->size() > 0)
      BackwardLinks->add( L );
  }

  DoubleLinks *DL = new DoubleLinks( ForwardLinks, BackwardLinks );
  
  delete ForwardLinks;
  delete BackwardLinks;
  return DL;
}


ObjectLinks * CorrelationMatrix::getCorrelation(int cluster_id, double min_likeliness)
{
  ObjectLinks *ClusterCorrelation = new ObjectLinks(cluster_id);

  int i = cluster_id - 1 + FIRST_CLUSTER;

  if (i >= N) 
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl; 
    return ClusterCorrelation; 
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }

  ClusterID_t MaxCluster;
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


#if 0
Link * CorrelationMatrix::getCorrelation(int cluster_id, double min_likeliness)
{
  Link *ClusterCorrelation = new Link(cluster_id);

  int i = cluster_id - 1 + FIRST_CLUSTER;

  if (i >= N) 
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl; 
    return ClusterCorrelation; 
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }

  ClusterID_t MaxCluster;
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
#endif


ObjectLinks * CorrelationMatrix::getReverseCorrelation(int cluster_id, double min_likeliness)
{
  ObjectLinks *ClusterCorrelation = new ObjectLinks(cluster_id);

  int j = cluster_id - 1 + FIRST_CLUSTER;

  if (j >= M)
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl;
    return ClusterCorrelation;
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }

  ClusterID_t MaxCluster;
  double max_percentage = -1;
  for (int i=FIRST_CLUSTER; i<N; i++)
  {
    if ((getCorrelationSeenCount(i, j) > 0) && (getCorrelationLikeliness(i, j) >= min_likeliness))
    {
      if (min_likeliness == -1)
      {
        /* Look for the correlation with maximum percentage only */
        if (getCorrelationLikeliness(i, j) > max_percentage)
        {
          max_percentage = getCorrelationLikeliness(i, j);
          MaxCluster    = i;
        }
      }
      else
      {
        /* Return all correlations above the percentage */
        ClusterCorrelation->add(CLUSTER_ID(i));
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

#if 0
Link * CorrelationMatrix::getReverseCorrelation(int cluster_id, double min_likeliness)
{
  Link *ClusterCorrelation = new Link(cluster_id);

  int j = cluster_id - 1 + FIRST_CLUSTER;

  if (j >= M)
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl;
    return ClusterCorrelation;
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }

  ClusterID_t MaxCluster;
  double max_percentage = -1;
  for (int i=FIRST_CLUSTER; i<N; i++)
  {
    if ((getCorrelationSeenCount(i, j) > 0) && (getCorrelationLikeliness(i, j) >= min_likeliness))
    {
      if (min_likeliness == -1)
      {
        /* Look for the correlation with maximum percentage only */
        if (getCorrelationLikeliness(i, j) > max_percentage)
        {
          max_percentage = getCorrelationLikeliness(i, j);
          MaxCluster    = i;
        }
      }
      else
      {
        /* Return all correlations above the percentage */
        ClusterCorrelation->add(CLUSTER_ID(i));
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
#endif


int CorrelationMatrix::getCorrelationSeenCount(int fromCluster, int toCluster)
{
  return Matrix[fromCluster][toCluster].SeenCount;
} 

double CorrelationMatrix::getCorrelationLikeliness(int fromCluster, int toCluster)
{
  return Matrix[fromCluster][toCluster].Likeliness;
} 

int CorrelationMatrix::getNumberOfClusters()
{
  return N-FIRST_CLUSTER;
}


ObjectLinks * CorrelationMatrix::getLinks(int cluster_id, double min_likeliness)
{
  ObjectLinks *links = new ObjectLinks( cluster_id );

  int i = cluster_id - 1 + FIRST_CLUSTER;

  if (i >= N)
  {
    cout << "WARNING: No correlations for Cluster " << cluster_id << " were found!" << endl;
    /* This shouldn't happen, except in the case of correlations built from sequence simultaneity, where the maximum number of clusters is 22 */
  }
  else
  {
    ClusterID_t MaxCluster;
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
          links->add( CLUSTER_ID(j) );
        }
      }
    }
    if ((min_likeliness == -1) && (max_percentage > -1))
    {
      /* Return the correlation with maximum percentage only */
      links->add( CLUSTER_ID(MaxCluster) );
    }
  }
  return links;
}

FrameLinks * CorrelationMatrix::getLinks( double min_likeliness )
{
  FrameLinks *Links = new FrameLinks();

  for (int FromCluster = FIRST_CLUSTER; FromCluster < N; FromCluster ++)
  {
    ObjectLinks *ToClusters;

    ToClusters = getLinks( FromCluster, min_likeliness );

    if (ToClusters->size() > 0)
    {
      Links->add( ToClusters );
    }
  }
  return Links;
}
