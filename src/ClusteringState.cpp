#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <limits.h>
#include "ClusteringState.h"

/**
 * Default constructor.
 */
ClusteringState::ClusteringState()
{
  NumberOfPoints     = 0;
  NumberOfDimensions = 0;
  Epsilon            = 0;
  TotalClusters      = 0;

  ClusterIDs.clear();
  NormalizedDimensions.clear();
  MinimumDimensions.clear();
  MaximumDimensions.clear();
}

/**
 * Constructor from arguments. The new object represents the information 
 * that would be comprised in a "clustered.csv" file that is described 
 * by the given arguments.
 */
ClusteringState::ClusteringState(
  int     n_points, 
  int     n_dims, 
  double  eps,
  int    *list_ids, 
  double *dims, 
  double *min_dims, 
  double *max_dims)
{
  NumberOfPoints     = n_points;
  NumberOfDimensions = n_dims;
  Epsilon            = eps;
  TotalClusters      = 0;

  ClusterIDs.clear();
  NormalizedDimensions.clear();
  MinimumDimensions.clear();
  MaximumDimensions.clear();
	
  for (int i=0; i<NumberOfPoints; i++)
  {
    if (list_ids[i] <= NOISE) continue;

    ClusterIDs.push_back(list_ids[i]);
    TotalClusters = (TotalClusters > list_ids[i] ? TotalClusters : list_ids[i] );
		
    for (int j=0; j<NumberOfDimensions; j++)
    {
      NormalizedDimensions.push_back(dims[(i * NumberOfDimensions) + j]);
    }
  }

  for (int j=0; j<NumberOfDimensions; j++)
  {
    MinimumDimensions.push_back(min_dims[j]);
    MaximumDimensions.push_back(max_dims[j]);
  }
}

/**
 * Constructor from file. The new object represents the information 
 * comprised in the specified "clustered.csv" file that is loaded and parsed.
 */
ClusteringState::ClusteringState(const char *file, double eps)
{
  FILE *fp;
  char  line[LINE_MAX];
  int   current_line;

  NumberOfPoints     = 0;
  NumberOfDimensions = 0;
  Epsilon            = 0;
  TotalClusters      = 0;

  ClusterIDs.clear();
  NormalizedDimensions.clear();
  MinimumDimensions.clear();
  MaximumDimensions.clear();

  if ((fp = fopen(file, "r")) == NULL) 
  {
    cerr << "ClusteringState::ClusteringState: Error opening file '" << file << "'" << endl;
    return;
  }

  Epsilon = eps;
  current_line = NumberOfPoints = 0;

  while (fgets(line, LINE_MAX, fp) != NULL)
  {
    if (current_line > 0)
    {
      int n_tokens, n_dims, cluster_id;
      vector<string> PointInfo;

      PointInfo  = explode(line, ",");
      n_tokens   = PointInfo.size();
      n_dims     = n_tokens - 1;

      cluster_id = atoi(PointInfo[n_tokens - 1].c_str());
      if (cluster_id <= NOISE) continue;

      ClusterIDs.push_back( cluster_id );
      TotalClusters = (TotalClusters > cluster_id ? TotalClusters : cluster_id );

      NumberOfPoints ++;
      NumberOfDimensions = n_dims;

      for (int dim=0; dim<n_dims; dim++)
      {
        double dim_value = atof(PointInfo[dim].c_str());

        NormalizedDimensions.push_back( dim_value );

        if (current_line == 1)
        {
          MinimumDimensions.push_back(dim_value);
          MaximumDimensions.push_back(dim_value);
        }
        else
        {
          if (dim_value < MinimumDimensions[dim]) MinimumDimensions[dim] = dim_value;
          if (dim_value > MaximumDimensions[dim]) MaximumDimensions[dim] = dim_value;
        }
      }
    }
    current_line ++;
  }
}

void ClusteringState::getPointDimensions(int point_id, vector<double> & point_dimensions)
{
	int base_index = point_id * NumberOfDimensions;

	for (int i=0; i<NumberOfDimensions; i++)
	{
		point_dimensions.push_back(NormalizedDimensions[base_index+i]);
	}
}

int ClusteringState::getPointClusterID(int point_id)
{
	return ClusterIDs[point_id];
}

int ClusteringState::getNumPoints()
{
	return NumberOfPoints;
}

int ClusteringState::getNumDimensions()
{
	return NumberOfDimensions;
}

int ClusteringState::getTotalClusters()
{
	return TotalClusters+1;
}

double ClusteringState::getEpsilon()
{
	return Epsilon;
}

const vector<double> & ClusteringState::getMinDimensions()
{
	return MinimumDimensions;
}

const vector<double> & ClusteringState::getMaxDimensions()
{
	return MaximumDimensions;
}

const vector<double> & ClusteringState::getNormDimensions()
{
	return NormalizedDimensions;
}

const vector<int> & ClusteringState::getClusterIDs()
{
	return ClusterIDs;
}

/**
 * Dump all the clustering information.
 */
void ClusteringState::dump()
{
  cout << "Epsilon=" << Epsilon << endl;
  cout << "NumberOfDimensions=" << NumberOfDimensions << endl;
  cout << "NumberOfPoints=" << NumberOfPoints << endl;

  vector<double>::iterator it;
  cout << "MinimumDimensions=";
  for (it=MinimumDimensions.begin(); it!=MinimumDimensions.end(); it++)
  {
    cout << *it << " ";
  }
  cout << endl;

  cout << "MaximumDimensions=";
  for (it=MaximumDimensions.begin(); it!=MaximumDimensions.end(); it++)
  {
    cout << *it << " ";
  }
  cout << endl;
	
  cout << "NormalizedDimensions.size()=" << NormalizedDimensions.size() << endl;
}

vector<string> ClusteringState::explode(string str, string separator)
{
  vector<string> results;
  int found = str.find_first_of(separator);
  while (found != string::npos)
  {
    if (found > 0)
    {
      results.push_back(str.substr(0,found));
    }
    str   = str.substr(found+1);
    found = str.find_first_of(separator);
  }
  if (str.length() > 0)
  {
    results.push_back(str);
  }
  return results;
}
