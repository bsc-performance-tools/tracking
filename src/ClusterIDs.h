#ifndef __CLUSTER_IDS_H__
#define __CLUSTER_IDS_H__

#include <set>
using std::set;

#define NOISE         4
#define FIRST_CLUSTER 5

#define CLUSTER_ID(id) (id - NOISE)

typedef int CID;
typedef set<CID> TClusterGroup;

#endif /* __CLUSTER_IDS_H__ */
