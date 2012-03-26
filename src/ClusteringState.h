#ifndef __CLUSTERING_STATE_H__
#define __CLUSTERING_STATE_H__

#include <string>
using std::string;
#include <vector>
using std::vector;
#include "ClusterIDs.h"

/**
 * This object represents in memory the information comprised in a "clustered.csv" file.
 */
class ClusteringState
{
public:
  int            NumberOfDimensions;
  int            NumberOfPoints;
  int            TotalClusters;
  double         Epsilon;
  vector<int>    ClusterIDs;
  vector<double> NormalizedDimensions;
  vector<double> MinimumDimensions;
  vector<double> MaximumDimensions;

  ClusteringState();
  ClusteringState(int n_points, int n_dims, double eps, int *list_ids, double *dims, double *min_dims, double *max_dims);
  ClusteringState(const char *file, double epsilon);

  void   getPointDimensions(int point_id, vector<double> &point_dimensions);
  int    getPointClusterID (int point_id);
  int    getNumPoints();
  int    getNumDimensions();
  int    getTotalClusters();
  double getEpsilon();

  const vector<double> & getMinDimensions();
  const vector<double> & getMaxDimensions();
  const vector<double> & getNormDimensions();
  const vector<int>    & getClusterIDs();

  void dump();

private:
  vector<string> explode(string str, string delim);
};

#endif /* __CLUSTERING_STATE_H__ */
