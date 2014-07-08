#ifndef __CORRELATION_MATRIX_H__
#define __CORRELATION_MATRIX_H__

#include "Links.h"
#include "ClusterIDs.h"

#include <vector>
using std::vector;
#include <map>
using std::map;

typedef struct
{
  int    SeenCount;
  double Likeliness;
} TCorrelationStats;

class CorrelationMatrix
{
  public:
    typedef map<ClusterID_t, double> TCorrelationLikeliness;

    CorrelationMatrix(vector<ClusterID_t> list_ids_1, vector<ClusterID_t> list_ids_2);
    CorrelationMatrix(int n, int m);
    ~CorrelationMatrix();
    void   Print();
    void   Stats();
    void   ComputePcts();

    ObjectLinks * getCorrelation(int cluster_id, double min_likeliness = 0.0);
    ObjectLinks * getReverseCorrelation(int cluster_id, double min_likeliness = 0.0);
    DoubleLinks * getCorrelations(double min_likeness = 0.0);

    int    getCorrelationSeenCount (int fromCluster, int toCluster);
    double getCorrelationLikeliness(int fromCluster, int toCluster);

 
    ObjectLinks * getLinks(int cluster_id, double min_likeliness);

    FrameLinks * getLinks( double min_likeliness );

    void Hit(int i, int j, int value = 1);
    void SetStats(int i, int j, int seen, double pct);
    void RaisePct(int i, int j, double pct);


    int getNumberOfClusters();

private:
    int N, M;
    TCorrelationStats **Matrix;
    bool ApplyOffset;

    void Allocate();
    double getCorrelationPct(int fromCluster, int toCluster);
};

#endif /* __CORRELATION_MATRIX_H__ */
