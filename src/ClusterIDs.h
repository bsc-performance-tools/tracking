#ifndef __CLUSTER_IDS_H__
#define __CLUSTER_IDS_H__

#include <set>
using std::set;

#define NOISE         5
#define FIRST_CLUSTER 6

#define CLUSTER_ID(id) (id - NOISE)

typedef int CID;
typedef int Cluster;
typedef set<Cluster> TClustersSet;

#endif /* __CLUSTER_IDS_H__ */
