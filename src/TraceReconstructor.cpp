#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <map>
using std::map;
using std::make_pair;
#include "localkernel.h"
#include "traceoptions.h"
#include "paraverconfig.h"
#include "TraceReconstructor.h"
#include "ClusterIDs.h"

TraceReconstructor::TraceReconstructor(
  vector<string> InputTraces, 
  vector<string> OutputTraces,
  vector< map<TTypeValuePair, TTypeValuePair> > &AllTranslationTables)
{
  cout << "+ Reconstructing " << InputTraces.size() << " traces... " << endl;

  KernelConnection *myKernel = new LocalKernel( NULL );

  TraceOptions *opts = myKernel->newTraceOptions( );

  opts->set_filter_by_call_time( false );

  opts->set_filter_states( true );
  opts->set_all_states( true );

  opts->set_filter_events( true );
  opts->set_discard_given_types( true );
  // TraceOptions::TFilterTypes dummyTypes;
  // dummyTypes[0].type = 1234567890;
  opts->set_filter_last_type( 1 );

  opts->set_filter_comms( true );
  opts->set_min_comm_size( 1 );

  for (int i=0; i<InputTraces.size(); i++)
  {
    /* DEBUG */
    if (AllTranslationTables[i].size() == 0)
    {
      cout << "Translation table for trace " << i+1 << " is empty." << endl;
    }
    else
    {
      cout << "Translation table for trace " << i+1 << " has " << AllTranslationTables[i].size() << " mappings:" << endl;
      map<TTypeValuePair, TTypeValuePair>::iterator it;
      for (it=AllTranslationTables[i].begin(); it!=AllTranslationTables[i].end(); it++)
      {
        cout << "   " << it->first.first << "," << it->first.second << " => " << it->second.first << "," << it->second.second << endl;
      }
    }

    if (AllTranslationTables[i].size() > 0)
    {
      cout << "Reconstructing trace '" << OutputTraces[i] << "'... ";
      cout.flush();
      TraceFilter *traceFilter = NULL;
      traceFilter = myKernel->newTraceFilter( (char *)InputTraces[i].c_str(), (char *)OutputTraces[i].c_str(), opts, AllTranslationTables[i] );
      myKernel->copyPCF( (char *)InputTraces[i].c_str(), (char *)OutputTraces[i].c_str() );
      cout << "done" << endl;
    }
    else
    {
      cout << "No reconstruction required for trace '" << InputTraces[i] << "'" << endl;
      cout.flush();
    }
  }
  cout << endl;
}

