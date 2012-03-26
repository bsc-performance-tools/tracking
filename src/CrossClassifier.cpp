#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <sstream>
using std::stringstream;
#include "CrossClassifier.h"
#include "ClusterCorrelationMatrix.h"

CrossClassifier::CrossClassifier(string CSV1, string CSV2, string OutputFile, double Epsilon)
{
  CrossClassifyOutputFile = OutputFile;

  /* Load clustering 1 information */
  CS1 = new ClusteringState(CSV1.c_str(), Epsilon);
  /* Load clustering 2 information */
  CS2 = new ClusteringState(CSV2.c_str(), Epsilon);

  /* Build a classification index for clustering 2 */
  CT  = new ClassifyTool(CS2);
}

CrossClassifier::~CrossClassifier()
{
  delete CT;
  delete CS1;
  delete CS2;
}

ClusterCorrelationMatrix * CrossClassifier::CrossClassify()
{
  vector<CID> Clustered1, Clustered1In2;

  /* Get the clustered points from clustering 1 */
  Clustered1 = CS1->getClusterIDs();

  /* Classify the points from clustering 1 into clustering 2 */
  CT->ClassifyPoints(CS1->getNumDimensions(), CS1->getNormDimensions(), Clustered1In2, CrossClassifyOutputFile.c_str());

  /* Return a correlation between points in clustering 1 and the same points classified in clustering 2 */
  ClusterCorrelationMatrix *CCM = new ClusterCorrelationMatrix(Clustered1, Clustered1In2);

  return CCM;
}

