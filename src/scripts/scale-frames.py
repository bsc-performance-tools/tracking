#!/usr/bin/python

import sys
import os
import matplotlib.mlab as mlab
import numpy as np
from numpy.lib.recfunctions import append_fields
import subprocess
import glob
import math
from collections import defaultdict

SuffixClustersData  = ".DATA.csv"
SuffixPlot          = ".*.gnuplot"
SuffixScaledPlot    = ".scaled"
SuffixNormalizedCSV = ".norm"

### Prints help
def PrintUsage():
  print "SYNTAX"
  print "  " + sys.argv[0] + " [-s DIMENSION1,DIMENSION2...] TRACE1 TRACE2 ...\n"

if (len(sys.argv) < 2): # FIXME
  PrintUsage()
  sys.exit(-1)

### Exception log(0) returns 0
def log(x):
  if (x == 0):
    return 0
  else:
    return math.log(x)

### Returns the xrange/yrange limits for the GNUplot script
def ComputeRanges(trace, dimension):
  dimension = "d_" + dimension.lower()
  if dimension in DimensionsToScale:
    return (float(NormalMinPerTrace[dimension][trace]), float(NormalMaxPerTrace[dimension][trace]))
  else:
    return (float(GlobalMin[dimension]), float(GlobalMax[dimension]))

### Normalizes the values of one dimension of the CSV data file
def Transform(cell, trace, dimension):
  dimension = dimension.lower()

  min = MinPerTrace[dimension][trace]
  max = MaxPerTrace[dimension][trace]

  if ((cell == 0) or (max - min == 0)):
    ret = 0
  else:
    ret = (log(cell) - log(min)) / (log(max) - log(min))

  return ret

### Make the function applicable to an ndarray
Normalize = np.vectorize(Transform)

#########################
###       MAIN        ### 
#########################

### Parse arguments
currentArg = 1
DimensionsToScale = [ ] # List of dimensions to scale with the number of tasks

while ((currentArg < len(sys.argv)) and (sys.argv[currentArg][0] == '-')):
  arg=sys.argv[currentArg]
  if (arg[1] == 's'):
    currentArg = currentArg + 1
    DimensionsToScale = sys.argv[currentArg].split(',')
    for i in range(0, len(DimensionsToScale)):
      if (DimensionsToScale[i][0:2] != "d_") and (DimensionsToScale[i][1:2] != "_"):
        DimensionsToScale[i] = "d_" + DimensionsToScale[i]
    DimensionsToScale = [dim.lower() for dim in DimensionsToScale]
  else:
    PrintUsage()
    print "*** INVALID PARAMETER "+sys.argv[currentArg];
    sys.exit(-1)
  currentArg = currentArg + 1

### 1 trace at least
if (len(sys.argv) - currentArg < 1):
  PrintUsage()
  print "*** Error: Pass 1 or more traces!"
  sys.exit(-1)

InputTraces = []                      # array[ TRACE1, TRACE2 ... ]
ClusteringDimensions = []             # array[ d_INSTRUCTIONS, d_IPC ... ]
TasksPerTrace = []                    # array[ num_tasks(TRACE1), num_tasks(TRACE2) ... ]

Data = { }                            # key: trace number; value: the CSV data file loaded in a ndarray

GlobalMin = defaultdict()             # key: dimension; value: global minimum
GlobalMax = defaultdict()             # key: dimension; value: global maximum
TraceWithMin = defaultdict()          # key: dimension; value: trace id for the trace with minimum dimension
TraceWithMax = defaultdict()          # key: dimension; value: trace id for the trace with maximum dimension
MinRatioPerTrace = defaultdict(list)     # key: dimension; value: array[ ratio_trace1, ratio_trace2 ... ]
MaxRatioPerTrace = defaultdict(list)     # key: dimension; value: array[ ratio_trace1, ratio_trace2 ... ]
MinPerTrace = defaultdict(list)       # key: dimension; value: array[ min(TRACE1), min(TRACE2) ... ]
MaxPerTrace = defaultdict(list)       # key: dimension; value: array[ max(TRACE1), max(TRACE2) ... ]
ScaledMinPerTrace = defaultdict(list) # key: dimension; value: array[ scaled_min(TRACE1), scaled_min(TRACE2) ... ] 
ScaledMaxPerTrace = defaultdict(list) # key: dimension; value: array[ scaled_max(TRACE1), scaled_max(TRACE2) ... ]
NormalMinPerTrace = defaultdict(list) # key: dimension; value: array[ normal_min(TRACE1), normal_min(TRACE2) ... ]
NormalMaxPerTrace = defaultdict(list) # key: dimension; value: array[ normal_max(TRACE1), normal_max(TRACE2) ... ]

### Process each trace
TraceNo = 0
for Trace in sys.argv[currentArg:]:
  InputTraces.append( Trace )
  TraceBasename = os.path.splitext(Trace)[0]
  CSVFile = TraceBasename + SuffixClustersData

  ### Get the number of tasks from the trace
  Header = subprocess.Popen(["head", "-n1", Trace], shell=False, stdout=subprocess.PIPE).communicate()[0]
  numTasks = float(Header.split("(")[2].split(":")[2])
  TasksPerTrace.append(numTasks)

  ### Load the clustering CSV
  Data[TraceNo] = mlab.csv2rec( CSVFile, comments='None' )

  ### Parse the CSV headings to find which are the clustering dimensions
  Headings = Data[TraceNo].dtype.names;
  FirstClusterDimension = Headings.index('line') + 1  # First dimension comes after column line
  LastClusterDimension  = FirstClusterDimension
 
  while ((LastClusterDimension + 1 < len(Headings)) and (Headings[LastClusterDimension+1][0] == 'd')):
    LastClusterDimension = LastClusterDimension + 1;

  ### All clusterings must have the same dimensions, we take them from the first one
  if (TraceNo == 0):
    for Dimension in Headings[FirstClusterDimension:LastClusterDimension+1]:
      ClusteringDimensions.append( Dimension )

  ### Process the clustering dimensions
  for Dimension in ClusteringDimensions:

    ### Compute local and global mins/maxs for every clustering dimension
    LocalMin = np.min(Data[TraceNo][Dimension])
    LocalMax = np.max(Data[TraceNo][Dimension])

    MinPerTrace[Dimension].append( LocalMin )
    MaxPerTrace[Dimension].append( LocalMax )

    GlobalMin[Dimension] = np.min( MinPerTrace[Dimension] )
    GlobalMax[Dimension] = np.max( MaxPerTrace[Dimension] )

    ### Compute the weighted dimensions per number of task
    ScaledMinPerTrace[Dimension] = np.array(MinPerTrace[Dimension]) * np.array(TasksPerTrace)
    ScaledMaxPerTrace[Dimension] = np.array(MaxPerTrace[Dimension]) * np.array(TasksPerTrace)
    
    ### Save an index to the trace that has the min/max value for this dimension
    TraceWithMin[Dimension] = ScaledMinPerTrace[Dimension].argmin()
    TraceWithMax[Dimension] = ScaledMaxPerTrace[Dimension].argmax() 

  TraceNo = TraceNo + 1

### Compute the ratios to apply in each trace 
for Dimension in ClusteringDimensions:
  for CurrentTrace in range(0, TraceNo):
    ### Compute the ratios for the current trace with respect to the min/max values for this dimension
    MinRatioPerTrace[Dimension].append( TasksPerTrace[ TraceWithMin[Dimension] ] / TasksPerTrace[ CurrentTrace ] )
    MaxRatioPerTrace[Dimension].append( TasksPerTrace[ TraceWithMax[Dimension] ] / TasksPerTrace[ CurrentTrace ] )

    ### Normalize the mins/maxs per trace with respect to the ratios
    NormalMinPerTrace[Dimension].append(
      MinPerTrace[Dimension][ TraceWithMin[Dimension] ] *
      MinRatioPerTrace[Dimension][CurrentTrace]
    )
    NormalMaxPerTrace[Dimension].append(
      MaxPerTrace[Dimension][TraceWithMax[Dimension]] *
      MaxRatioPerTrace[Dimension][CurrentTrace]
    )

### Generate the output plots/CSVs
for CurrentTrace in range(0, TraceNo):
  Trace = InputTraces[CurrentTrace]
  TraceBasename = os.path.splitext(Trace)[0]
  CSVFile = TraceBasename + SuffixClustersData
  PlotFile = TraceBasename + SuffixPlot
  AllPlots = glob.glob(PlotFile)

  ### Rewrite the plots with scaled ranges
  for Plot in AllPlots:
    OutputPlot = Plot + SuffixScaledPlot

    ### Get the plotted dimensions from the name of the plot
    PrintDimX = Plot.split(".")[-3]
    PrintDimY = Plot.split(".")[-2]

    ### Write the new range for the scaled plot
    XRange = ComputeRanges(CurrentTrace, PrintDimX)
    YRange = ComputeRanges(CurrentTrace, PrintDimY)

    pretty_x_min = "%.2f" % XRange[0]
    pretty_x_max = "%.2f" % XRange[1]
    pretty_y_min = "%.2f" % YRange[0]
    pretty_y_max = "%.2f" % YRange[1]
    os.system( "echo set xrange [" + pretty_x_min + ":" + pretty_x_max + "] > "  + OutputPlot )
    os.system( "echo set yrange [" + pretty_y_min + ":" + pretty_y_max + "] >> " + OutputPlot )
    os.system( "cat " + Plot + " | grep -v \"set xrange\" | grep -v \"set yrange\" >> " + OutputPlot )

  ### Rewrite the CSV file with normalized values for the clustering dimensions
  OutputCSV = CSVFile + SuffixNormalizedCSV 

  NormalizedDimensions = []
  for Dimension in ClusteringDimensions:
    tmp = list(Dimension)
    tmp[0] = 'n'
    NormalizedDimension = ''.join(tmp)

    if NormalizedDimension not in Data[CurrentTrace].dtype.names:
      ### Dimension not normalized in the CSV file, we normalize it now
      NormalizedData = Normalize( Data[CurrentTrace][Dimension], CurrentTrace, Dimension ) 
      Data[CurrentTrace] = append_fields( Data[CurrentTrace], NormalizedDimension, data=NormalizedData )

    NormalizedDimensions.append(NormalizedDimension)

  ### Write the file
  # mlab.rec2csv(D2, OutputCSV, delimiter=',') # Don't use this method, fields are sorted arbitrarily.

  fd = open(OutputCSV, 'w+')
  Header = ', '.join(NormalizedDimensions + ['clusterid'])
  fd.write(Header + "\n")
  for i in range(0, len(Data[CurrentTrace])):
    Point = Data[CurrentTrace][i]
    line  = ""
    for NormalizedDimension in NormalizedDimensions:
      line = line + str(Point[NormalizedDimension]) + ", "
    line = line + str(Point['clusterid'])
    fd.write(line + "\n")
  fd.close()
