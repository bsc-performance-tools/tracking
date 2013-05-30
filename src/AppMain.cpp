#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <sstream>
using std::istringstream;
#include "ClusterIDs.h"
#include "Tracking.h"

string CallersCFG     = "";
bool   CallersCFGRead = false;
bool   Reconstruct    = false;
bool   Verbose        = false;
string OutputPrefix   = "";
double MinimumScore   = DEFAULT_MIN_SCORE;

void PrintUsage(char* ApplicationName)
{
  fprintf(stdout, HELP, ApplicationName);
  fflush(stdout);
}

int ReadArgs(int argc, char *argv[])
{
  int j = 1;

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {

    fprintf(stdout, HELP, basename(argv[0]));
    exit(EXIT_SUCCESS);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {

      switch (argv[j][1])
      {
        case 'a':
          j++;
          MinimumScore = atof(argv[j]);
          break;
        case 'c':
          j++;
          CallersCFG = argv[j];
          CallersCFGRead = true;
          break;
        case 'o':
          j++;
          OutputPrefix = argv[j];
          OutputPrefix += ".";
          break;
        case 'r':
          Reconstruct = true;
          break;
        case 'v':
          Verbose = true;
          break;
        default:
          cerr << "*** INVALID PARAMETER " << argv[j][1] << " *** " << endl << endl;
          PrintUsage(basename(argv[0]));
          exit(EXIT_FAILURE);
      }
    }
  }
  OutputPrefix += "TRACKING.RESULTS";

  return j;
}


int main(int argc, char **argv)
{
  vector<string> TracesArray;
  vector<CID>    NumClustersToTrack;

  int FirstTraceArg = ReadArgs(argc, argv);

  for (int i=FirstTraceArg; i<argc; i++)
  {
    string CurrentArg = argv[i];

    string CurrentTrace           = CurrentArg.substr( 0, CurrentArg.find(":") );
    string strLastClusterForTrace = CurrentArg.substr( CurrentArg.find(":") + 1 );
    CID    LastClusterForTrace;

    istringstream iss(strLastClusterForTrace);

    if (!(iss >> LastClusterForTrace))
    {
        cerr << "*** WRONG ARGUMENT '" << CurrentArg << "' ***" << endl;
        cerr << strLastClusterForTrace << " is not a valid Cluster ID" << endl << endl;
        PrintUsage(basename(argv[0]));
        exit(EXIT_FAILURE);
    }

    TracesArray.push_back(CurrentTrace);
    NumClustersToTrack.push_back(LastClusterForTrace);
  }

  Tracking *ClustersTracking = new Tracking(TracesArray, NumClustersToTrack, CallersCFG, MinimumScore, OutputPrefix, Reconstruct, Verbose);

  ClustersTracking->CorrelateTraces();
  delete ClustersTracking;

  return 0;
}

