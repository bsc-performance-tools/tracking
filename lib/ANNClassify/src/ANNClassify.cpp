#include "ANNClassify/ANNClassify.h"
#include "libANNClassify.h"

ANNClassify::ANNClassify()
{
  ANNClassifyObj = new _ANNClassify();
}

bool
ANNClassify::Init(int                   DataSize,
                  int                   Dimensions,
                  double                Eps,
                  const vector<double>& MinValues,
                  const vector<double>& MaxValues)
{
  return ANNClassifyObj->Init(DataSize,
                              Dimensions,
                              Eps,
                              MinValues,
                              MaxValues);
}
    
bool
ANNClassify::AddPoint(vector<double> PointDimensions, int ClusterId)
{
  return ANNClassifyObj->AddPoint(PointDimensions, ClusterId);
}

bool
ANNClassify::BuildClassifier(void)
{
  return ANNClassifyObj->BuildClassifier();
}

bool
ANNClassify::ClassifyPoint(vector<double> PointDimensions,
                        int&           ClusterId,
                        bool           DoNormalization)
{
  return ANNClassifyObj->ClassifyPoint(PointDimensions, ClusterId, DoNormalization);
}

bool
ANNClassify::Error(void)
{
  return ANNClassifyObj->Error();
}

string
ANNClassify::GetLastError(void)
{
  return ANNClassifyObj->GetLastError();
}

