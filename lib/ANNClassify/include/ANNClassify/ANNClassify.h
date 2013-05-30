#ifndef __ANN_CLASSIFY_H__
#define __ANN_CLASSIFY_H__

#include <vector>
using std::vector;

#include <string>
using std::string;

class _ANNClassify;

class ANNClassify
{
  private:
    _ANNClassify* ANNClassifyObj;
    
  public:

    ANNClassify();
    
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
};
  
#endif /* __ANN_CLASSIFY_H__ */
