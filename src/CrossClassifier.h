#ifndef __CROSS_CLASSIFIER_H__
#define __CROSS_CLASSIFIER_H__

#include <string>
using std::string;
#include <vector>
using std::vector;
#include "ClassifyTool.h"
#include "CorrelationMatrix.h"

class CrossClassifier
{
public:
  CrossClassifier(string CSV1, string CSV2, string OutputFile, double Epsilon = 1.0);
  ~CrossClassifier();

  CorrelationMatrix * CrossClassify();

private:
  ClusteringState *CS1, *CS2;
  ClassifyTool    *CT;
  string           CrossClassifyOutputFile;
};

#endif /* __CROSS_CLASSIFIER_H__ */
