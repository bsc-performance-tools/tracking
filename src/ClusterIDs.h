#ifndef __CLUSTER_IDS_H__
#define __CLUSTER_IDS_H__

#include <set>

using std::set;

#define THRESHOLD_FILTERED 4
#define NOISE              5
#define FIRST_CLUSTER      6

#define CLUSTER_ID(id) (id - NOISE)

typedef int              ClusterID_t;
typedef set<ClusterID_t> ClustersSet_t;

#endif /* __CLUSTER_IDS_H__ */
