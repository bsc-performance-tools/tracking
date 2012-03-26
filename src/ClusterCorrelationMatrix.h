#ifndef __CLUSTER_CORRELATION_MATRIX_H__
#define __CLUSTER_CORRELATION_MATRIX_H__

#include "Correlation.h"
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

class ClusterCorrelationMatrix
{
public:
    typedef map<CID, double> TCorrelationLikeliness;

	ClusterCorrelationMatrix(vector<CID> list_ids_1, vector<CID> list_ids_2);
	ClusterCorrelationMatrix(int n, int m);
	~ClusterCorrelationMatrix();
	void   Print();
	void   Stats();
    void   ComputePcts();
	OneWayCorrelation * getCorrelation(int cluster_id, double min_likeliness = 0.0);
    int    getCorrelationSeenCount (int fromCluster, int toCluster);
    double getCorrelationLikeliness(int fromCluster, int toCluster);

    void Hit(int i, int j, int value = 1);
    void SetStats(int i, int j, int seen, double pct);

    int getNumberOfClusters();

private:
	int N, M;
	TCorrelationStats **CorrelationMatrix;
    bool ApplyOffset;

    void Allocate();
	double getCorrelationPct(int fromCluster, int toCluster);
};

#endif /* __CLUSTER_CORRELATION_MATRIX_H__ */
