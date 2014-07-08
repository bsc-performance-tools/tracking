#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
#include "Knapsack.h"
#include <stdio.h>

#define max(a,b) (a > b ? a : b)

int **matrix = NULL;
int **picks  = NULL;

void _knapsack_init(int nitems, int size)
{
  matrix = (int **)malloc(sizeof(int *) * nitems);
  picks  = (int **)malloc(sizeof(int *) * nitems);
  for (int i=0; i<nitems; i++)
  {
    matrix[i] = (int *)malloc(sizeof(int) * size+1);
    picks[i]  = (int *)malloc(sizeof(int) * size+1);
    for (int j=0; j<size+1; j++)
    {
      matrix[i][j] = 0;
      picks[i][j]  = 0;
    }
  }
}

int _knapsack(int index, int size, int weights[],int values[])
{
  int take = 0, dont_take = 0;

  if (matrix[index][size] != 0)
  {
    return matrix[index][size];
  }
  if (index == 0) 
  {
    if (weights[0] <= size) 
    {
      picks[index][size]  = 1;
      matrix[index][size] = values[0];
      return values[0];
    }
    else
    {
      picks[index][size]  = -1;
      matrix[index][size] = 0;
      return 0;
    }
  }
  if (weights[index] <= size)
  {
    take = values[index] + _knapsack(index-1, size-weights[index], weights, values);
  }
  dont_take = _knapsack(index-1, size, weights, values);
  matrix[index][size] = max(take, dont_take);
  if (take> dont_take)
  {
    picks[index][size]=1;
  }
  else
  {
    picks[index][size]=-1;
  }
  return matrix[index][size];
}

void _knapsack_picks(int item, int size, int weights[], vector<int> &PickedItems)
{
  /* cerr << "[DEBUG] Knapsack:: Picks were:" << endl; */
  while (item >= 0)
  {
    if (picks[item][size] == 1)
    {
      PickedItems.push_back (item);
      /* cerr << item << " "; */
      item --;
      size -= weights[item];
    }
    else
    {
      item --;
    }
  }
  /* cerr << endl; */
}

void _knapsack_fini(int nitems)
{
  for (int i=0; i<nitems; i++)
  {
    free(matrix[i]);
    free(picks[i]);
  }
  free(matrix);
  free(picks);
}

void Knapsack( int TargetDensity, map<ClusterID_t, int> &CandidateClusters, set<ClusterID_t> &PickedClusters )
{
    int  knapsack_limit = TargetDensity;
    int  nitems         = CandidateClusters.size();
    int *ids            = (int *)malloc(sizeof(int) * nitems);
    int *weights        = (int *)malloc(sizeof(int) * nitems);
    int *values         = (int *)malloc(sizeof(int) * nitems);
    int  best_fit       = 0;
    int  i              = 0;

    map<ClusterID_t, int>::iterator it;

    for (it = CandidateClusters.begin(); it != CandidateClusters.end(); ++ it)
    {
      /* DEBUG -- candidates weights & values 
      cout << it->first << " " << it->second << endl; */
      ids[i]     = it->first;
      weights[i] = it->second;
      values[i]  = it->second;
      i ++;
    }

    _knapsack_init(nitems, knapsack_limit);

    /* DEBUG -- input items 
    cout << "[DEBUG] Knapsack:: nitems=" << nitems << " limit=" << knapsack_limit << " weights="; 
    for (int i=0; i<nitems; i++)
    {
      cout << weights[i] << " ";
    }
    cout << "values=";
    for (int i=0; i<nitems; i++)
    {
      cout << values[i] << " ";
    }
    cout << endl; */

    best_fit = _knapsack(nitems-1, knapsack_limit, weights, values);

    /* DEBUG -- aggregate density of all clusters that fit 
    cerr << "[DEBUG] Knapsack:: best_fit=" << best_fit << endl; */

    vector<int> PickedItems;
    _knapsack_picks(nitems-1, knapsack_limit, weights, PickedItems);

    for (int i=0; i<PickedItems.size(); i++)
    {
      PickedClusters.insert( ids[i] );
    }

    _knapsack_fini(nitems);

    free(weights);
    free(values);
    free(ids);
}

