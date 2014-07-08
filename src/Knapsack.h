#ifndef __KNAPSACK_H__
#define __KNAPSACK_H__

#include <map>
#include <set>
#include <vector>
#include "ClusterIDs.h"

using std::map;
using std::set;
using std::vector;

int _knapsack(int index, int size, int weights[],int values[]);

void _knapsack_picks(int item, int size, int weights[], vector<int> &PickedItems);

void _knapsack_init(int nitems, int size);

void _knapsack_fini(int nitems);

void Knapsack( int TargetDensity, map<ClusterID_t, int> &CandidateClusters, set<ClusterID_t> &PickedClusters );

#endif /* __KNAPSACK_H__ */
