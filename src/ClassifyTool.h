#ifndef __CLASSIFY_TOOL_H__
#define __CLASSIFY_TOOL_H__

#include <ANNClassify/ANNClassify.h>
#include "ClusteringState.h"

class ClassifyTool 
{
public:
  ClassifyTool(ClusteringState *CS);
  void ClassifyPoints(
    int                 dims_per_point, 
    std::vector<double> all_dimensions, 
    std::vector<int>   &all_cluster_ids, 
    const char         *plot_data_file = NULL);

private:
  ANNClassify *Classifier;
};

#endif /* __CLASSIFY_TOOL_H__ */
