#include <libANNClassify.h>

_ANNClassify::_ANNClassify()
{
  _Error    = false;
}

bool
_ANNClassify::Init(int                   DataSize,
                   int                   Dimensions,
                   double                Eps,
                   const vector<double>& MinValues,
                   const vector<double>& MaxValues)
{
  this->DataSize   = DataSize;
  this->Dimensions = Dimensions;
  this->Eps        = Eps;

  CurrentPoint     = 0;
  
  /*
  printf ("DataSize   = %d\n", DataSize);
  printf ("Dimensions = %d\n", Dimensions);
  printf ("Eps        = %f\n", Eps);
  printf ("MinValues  = %d\n", MinValues.size());
  printf ("MaxValues  = %d\n", MaxValues.size());
  */
  
  ANNDataPoints = annAllocPts(DataSize, Dimensions);
  // ClustersIDs   = vector<int>(DataSize, 0);

  if (MinValues.size() != Dimensions)
  {
    LastError = "Minimum values vector has incorrect number of dimensions";
    _Error    = true;
    return false;
  }
  else
  {
    this->MinValues = MinValues;
  }

  if (MaxValues.size() != Dimensions)
  {
    LastError = "Maximum values vector has incorrect number of dimensions";
    _Error    = true;
    return false;
  }
  else
  {
    this->MaxValues = MaxValues;
  }

  return true;
}

bool
_ANNClassify::AddPoint(vector<double> PointDimensions, int ClusterId)
{
  ANNpoint NewPoint;

  if (PointDimensions.size () != Dimensions)
  {
    LastError = "Incorrect number of dimensions in the new point";
    _Error    = true;
    return false;
  }

  if (CurrentPoint == DataSize)
  {
    LastError = "Number of added points bigger than original data size";
    _Error    = true;
    return false;
  }
  
  NewPoint = annAllocPt(Dimensions);

  for (int i = 0; i < Dimensions; i++)
  {
    NewPoint[i] = PointDimensions[i];
  }

  ANNDataPoints[CurrentPoint] = NewPoint;
  ClustersIDs.push_back(ClusterId);

  CurrentPoint++;

  return true;
}

bool
_ANNClassify::BuildClassifier(void)
{
  if (CurrentPoint != DataSize)
  {
    LastError = "Total number of point less than the original data size";
    _Error     = true;
    return false;
  }

  SpatialIndex = new ANNkd_tree(ANNDataPoints,
                                DataSize,
                                Dimensions);

  return true;
}

bool
_ANNClassify::ClassifyPoint(vector<double> PointDimensions, int& ClusterId, bool DoNormalization)
{
  ANNpoint     QueryPoint;
  ANNidxArray  ResultPoint    = new ANNidx[1];
  ANNdistArray ResultDistance = new ANNdist[1];

  if (PointDimensions.size() != Dimensions)
  {
    LastError = "Incorrect number of dimensions in the query point";
    _Error     = true;
    return false;
  }
  
  QueryPoint = annAllocPt(Dimensions);

  for (int i = 0; i < Dimensions; i++)
  {
    if (DoNormalization)
    {
      QueryPoint[i] = Normalize (PointDimensions[i], MinValues[i], MaxValues[i]);
    }
    else
    {
      QueryPoint[i] = PointDimensions[i];
    }
  }
  
  /* Query for the nearest point to the current */
  SpatialIndex->annkSearch(QueryPoint, 1, ResultPoint, ResultDistance);
  
  if (ResultDistance[0] < pow(Eps, 2.0))
  {
    /* Current point must be classified in the same cluster as Neighbour */
    ClusterId = ClustersIDs[ResultPoint[0]];
  }
  else
  {
    /* Otherwise, current point is Noise! */
    ClusterId = _ANNClassify::NOISE_CLUSTERID;
  }
  
  return true;
}

bool
_ANNClassify::Error(void)
{
  return _Error;
}

string
_ANNClassify::GetLastError(void)
{
  return LastError;
}

double
_ANNClassify::Normalize(double Dimension, double MinValue, double MaxValue)
{
  return ((Dimension-MinValue)/(MaxValue-MinValue));
}
