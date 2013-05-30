#ifndef __LIB_ANN_CLASSIFY_H__
#define __LIB_ANN_CLASSIFY_H__

#include <ANN/ANN.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

class _ANNClassify
{
  static const int NOISE_CLUSTERID = 4;

  private:
    int            DataSize;
    int            Dimensions;
    double         Eps;

    vector<double> MinValues;
    vector<double> MaxValues;
    
    vector<int>    ClustersIDs;
    ANNpointArray  ANNDataPoints;
    ANNkd_tree*    SpatialIndex;

    int            CurrentPoint;

    bool           _Error;
    string         LastError;

  public:

    _ANNClassify();
    
    bool Init(int                   DataSize,
              int                   Dimensions,
              double                Eps,
              const vector<double>& MinValues,
              const vector<double>& MaxValues);
    
    bool AddPoint(vector<double> PointDimensions, int ClusterId);

    bool BuildClassifier(void);

    bool ClassifyPoint(vector<double> PointDimensions, int& ClusterId, bool DoNormalization = true);

    bool Error(void);

    string GetLastError(void);

  private:
    double Normalize(double Dimension, double MinValue, double MaxValue);
};
  
#endif /* __LIB_ANN_CLASSIFY_H__ */
