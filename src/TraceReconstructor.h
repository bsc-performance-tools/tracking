#ifndef __TRACE_RECONSTRUCTOR_H__
#define __TRACE_RECONSTRUCTOR_H__

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <localkernel.h>

#define CLUSTER_EV 90000001

class TraceReconstructor
{
public:
  TraceReconstructor(
    vector<string> InputTraces, 
    vector<string> OutputTraces,
    vector< map<TTypeValuePair, TTypeValuePair> > &AllTranslationTables );
};

#endif /* __TRACE_RECONSTRUCTOR_H__ */
