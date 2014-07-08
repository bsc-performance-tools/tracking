#include <iostream>
#include <sstream>
#include "DistanceTracker.h"
#include "Utils.h"

using std::cerr;
using std::cout;
using std::endl;
using std::stringstream;

DistanceTracker::DistanceTracker(int trace1, string CSV1, int trace2, string CSV2, double Epsilon)
{
  stringstream ssCrossClassifierOutput;

  CS1 = NULL;
  CS2 = NULL;
  CT  = NULL;

  ssCrossClassifierOutput << "cross-classifier-" << trace1 << "-" << trace2 << ".csv";
  CrossClassifierOutput = ssCrossClassifierOutput.str();

  /* Load clustering 1 information */
  if (FileExists(CSV1))
  {
    CS1 = new ClusteringState(CSV1.c_str(), Epsilon);
  }
  else
  {
    cout << "WARNING: DistanceTracker::DistanceTracker: Clustering data file '" << CSV1 << "' from trace " << trace1 << " not found!" << endl;
  } 

  /* Load clustering 2 information */
  if (FileExists(CSV2))
  {
    CS2 = new ClusteringState(CSV2.c_str(), Epsilon);
  }
  else
  {
    cout << "WARNING: DistanceTracker::DistanceTracker: Clustering data file '" << CSV2 << "' from trace " << trace2 << " not found!" << endl;
  }

  /* Build a classification index for clustering 2 */
  if (CS2 != NULL)
  {
    CT  = new ClassifyTool(CS2);
  }
}

DistanceTracker::~DistanceTracker()
{
  if (CT  != NULL) delete CT;
  if (CS1 != NULL) delete CS1; 
  if (CS2 != NULL) delete CS2;
}

void DistanceTracker::RunTracker()
{
  vector<ClusterID_t> Clustered1, Clustered1In2;
  
  if ((CS1 != NULL) && (CT != NULL))
  {
    /* Get the clustered points from clustering 1 */
    Clustered1 = CS1->getClusterIDs();

    /* Classify the points from clustering 1 into clustering 2 */
    CT->ClassifyPoints(CS1->getNumDimensions(), CS1->getNormDimensions(), Clustered1In2, CrossClassifierOutput.c_str());

    /* Build the correlation matrix between points in clustering 1 and the same points classified in clustering 2 */
    WhoIsWho = new CorrelationMatrix(Clustered1, Clustered1In2);
  }
}

