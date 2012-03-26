#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
#include "ClassifyTool.h"

/**
 * Initializes the libANN classifier with the given clustering state.
 */
ClassifyTool::ClassifyTool(ClusteringState *CS)
{
  Classifier = new ANNClassify();

  if (!Classifier->Init(
         CS->getNumPoints(),
         CS->getNumDimensions(),
         CS->getEpsilon(),
         CS->getMinDimensions(),
         CS->getMaxDimensions()))
  {
    cerr << "ClassifyTool::Error: Classifier initialization failed! Aborting..." << endl;
    exit(-1);
  }

  /* Insert the clustering points */
  for (int current_point=0; current_point < CS->getNumPoints(); current_point ++)
  {
    vector<double> dims;
    CS->getPointDimensions(current_point, dims);
    int cluster_id = CS->getPointClusterID (current_point);

    Classifier->AddPoint(dims, cluster_id);
  }

  /* Generate the classification index */
  Classifier->BuildClassifier();
}

/**
 * Classify the given points using the classification index we've previously built. 
 */
void ClassifyTool::ClassifyPoints(
  int                 dims_per_point, 
  std::vector<double> all_dimensions, 
  std::vector<int>   &all_cluster_ids, 
  const char         *plot_data_file)
{
  ofstream fs;
  fs.open (plot_data_file);

  if (plot_data_file != NULL)
  {
    if (!fs.good())
    {
      cerr << "ClassifyPoints::Error opening file '" << plot_data_file << "' for writing." << endl;
    }
    else
    {
      for (int i=0; i<dims_per_point; i++)
      {
        fs << "Dim" << i+1;
        if (i < (dims_per_point - 1)) fs << ",";
      }
      fs << endl;
    }
  }
  
  /* Classify point per point */
  for (int i=0; i<all_dimensions.size(); i+=dims_per_point)
  {
    int cluster_id;
    vector<double> dims;

    for (int j=0; j<dims_per_point; j++)
    {
      double cur_dim = all_dimensions[i+j];
      dims.push_back(cur_dim);

      if (fs.good())
      {
        fs << cur_dim << ",";
      }

    }
    Classifier->ClassifyPoint(dims, cluster_id, false);
    if (fs.good())
    {
      fs << cluster_id << endl;
    }
    all_cluster_ids.push_back(cluster_id);
  }
}

