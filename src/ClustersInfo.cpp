#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/spirit/include/classic_rule.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_refactoring.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include "ClustersInfo.h"

using namespace std;
using namespace boost::spirit;
using namespace boost::spirit::classic;

ClustersInfo::ClustersInfo(std::string file_name)
{
  ClustersInfoFileName = file_name;

  ifstream ClustersInfoStream;
  std::string CurrentLine;

  ClustersInfoStream.open( ClustersInfoFileName.c_str(), ios_base::in );
  if (!ClustersInfoStream)
  {
    cerr << "Unable to open clusters information file '" << ClustersInfoFileName << "'" << endl;
  }
  else
  {
    getline(ClustersInfoStream, CurrentLine); /* Header */
    getline(ClustersInfoStream, CurrentLine); /* Density */

    ParseDensity( CurrentLine );

    getline(ClustersInfoStream, CurrentLine); /* Total duration */
    getline(ClustersInfoStream, CurrentLine); /* Avg duration */
    getline(ClustersInfoStream, CurrentLine); /* % Total duration */

    ParsePctTotalDuration( CurrentLine );
  }
}

ClustersInfo::~ClustersInfo()
{
  ClustersInfoFileName = "";
  ClustersDensity.clear();
  ClustersPctTotalDuration.clear();
}

bool ClustersInfo::ParseDensity (string Line)
{

  rule<> DensityRule;
  int_parser<int, 10, 1, -1> density_p;

  DensityRule
    = (str_p("Density")) >>
      *( ',' >> (density_p[push_back_a(ClustersDensity)]))
      >> end_p
    ;

  BOOST_SPIRIT_DEBUG_RULE(DensityRule);

  return parse(Line.c_str(), DensityRule).full;
}

bool ClustersInfo::ParsePctTotalDuration (string Line)
{
  rule<> PctTotalDurationRule;
  real_parser< double > duration_p;

  PctTotalDurationRule
    = (str_p("% Total Duration")) >>
      *( ',' >> (duration_p[push_back_a(ClustersPctTotalDuration)]))
      >> end_p
    ;

  BOOST_SPIRIT_DEBUG_RULE(PctTotalDurationRule);

  return parse(Line.c_str(), PctTotalDurationRule).full;
}

int ClustersInfo::GetNumClusters()
{
  return ClustersDensity.size() - 1; /* -1 because index 0 is NOISE */
}

int ClustersInfo::GetDensity(int ClusterID)
{
  if ((ClusterID > 0) && (ClusterID < ClustersDensity.size()))
  {
    return ClustersDensity[ClusterID];
  }
  else
  {
    return 0;
  }
}

double ClustersInfo::GetPctTotalDuration(int ClusterID)
{
  if ((ClusterID > 0) && (ClusterID < ClustersPctTotalDuration.size()))
  {
    return ClustersPctTotalDuration[ClusterID];
  }
  else
  {
    return 0;
  }
}
