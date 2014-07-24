import ConfigParser
import matplotlib.mlab as mlab
import numpy as np
import subprocess
import sys
import os.path

#
# Defines
#
CLUSTER_ID_THRESHOLD_FILTERED = 4
CLUSTER_ID_UNTRACKED          = 5

CSV_COLUMN_CLUSTERID    = 'clusterid'
CSV_FIRST_CLUSTER_ID    =  6
CSV_INDEX_FIRST_METRIC  =  7
CSV_INDEX_LAST_METRIC   = -1
SIZE_EMPTY_HISTOGRAM    = 29 # This is the size of the header when the histogram is empty
PARAVER_OFFSET          = 5

class DataManager:

  def __init__(self, parent, config_file):
    self.Data             = { }
    self.Metrics          = { }
    self.Callers          = { }
    self.NumberOfFrames   = 0
    self.NumberOfClusters = 0
    self.FirstCluster     = CSV_FIRST_CLUSTER_ID;
    self.LastCluster      = self.FirstCluster
    self.TasksPerFrame    = [ ]
    self.Traces           = [ ]

    # Load configuration
    self.Config = ConfigParser.ConfigParser()
    self.Config.read( config_file )

    self.NumberOfFrames = int( self.ReadConfig('Info')['frames'] )
    self.LoadData()

  def LoadData(self):

    for frame in range(1, self.NumberOfFrames+1):
      FrameConfig = self.ReadConfig('Frame '+str(frame))

      # Load the CSV data file 
      csv = FrameConfig['data']
      if not os.path.isfile(csv):
        csv = os.path.dirname(sys.argv[1]) + "/" + csv
      self.Data[frame] = mlab.csv2rec(csv, comments='None')

      # Read the list of metrics from the first frame (assumes all frames have the same metrics!)
      self.MetricsInCSV = self.Data[frame].dtype.names[CSV_INDEX_FIRST_METRIC:CSV_INDEX_LAST_METRIC]

      # Filter the normalized metrics, we only plot the clustering dimensions and the extrapolated metrics
      self.Metrics[frame] = [m for m in self.MetricsInCSV if self.isClusteringDimension(m) or self.isExtrapolated(m)]

      # Load the callers histogram        
      if 'callers' in FrameConfig:
        histogram = FrameConfig['callers']
        if not os.path.isfile(histogram):
          histogram = os.path.dirname(sys.argv[1]) + "/" + histogram
        # Ensure the histogram is not empty (may happen when all callers are unresolved)
        if (os.path.getsize(histogram) > SIZE_EMPTY_HISTOGRAM):
          self.Callers[frame] = mlab.csv2rec(histogram)

      # Read the number of tasks per trace
      trace = FrameConfig['otrace']
      if not os.path.isfile(trace):
        trace = os.path.dirname(sys.argv[1]) + "/" + trace
        if not os.path.isfile(trace):
          trace = "." + trace

      header = subprocess.Popen(["head", "-n1", trace], shell=False, stdout=subprocess.PIPE).communicate()[0]
      num_tasks = float(header.split("(")[2].split(":")[2])
      self.TasksPerFrame.append(num_tasks)

      # Save the trace files
      self.Traces.append(trace)

    self.LastCluster      = np.max( self.Data[1][CSV_COLUMN_CLUSTERID] )
    self.NumberOfClusters = self.LastCluster - self.FirstCluster + 1

  def isClusteringDimension(self, metric_name):
    if (metric_name[0:2] == 'd_'):
      return True
    else:
      return False

  def isExtrapolated(self, metric_name):
    if (metric_name[0:2] == 'x_'):
      return True
    else:
      return False

  def PrettyCluster(self, cluster_id):
    return cluster_id - PARAVER_OFFSET

  def PrettyMetric(self, metric_name):
    return metric_name[2:].upper()

  def GetFirstClusterID(self):
    return self.FirstCluster

  def GetFirstObject(self):
    return self.FirstCluster

  def GetLastObject(self):
    return self.LastCluster

  def GetNumberOfFrames(self):
    return self.NumberOfFrames

  def GetNumberOfObjects(self):
    return self.NumberOfClusters

  def GetFrameData(self, frame, metric):
    return self.Data[frame][metric]

  def GetClusterData(self, frame, cluster_id, metric):
    return self.Data[frame][self.Data[frame][CSV_COLUMN_CLUSTERID] == cluster_id][metric]

  def GetMetricID(self, frame, metric_name):
    return self.Metrics[frame].index( metric_name )
  
  def GetMetricName(self, frame, metric_id):
    return self.Metrics[frame][metric_id]

  def GetFrameMetrics(self, frame):
    return self.Metrics[ frame ]

  def GetCallers(self, frame, cluster_id):
    data   = self.Callers[frame]['pct'][self.Callers[frame]['clusterid'] == cluster_id - PARAVER_OFFSET]
    labels = self.Callers[frame]['caller'][self.Callers[frame]['clusterid'] == cluster_id - PARAVER_OFFSET]
    return (data, labels)

  def GetTasksPerFrame(self):
    return self.TasksPerFrame

  def GetTrace(self, frame):
    return self.Traces[frame-1]

  def ReadConfig(self, section):
    dict = {}
    options = self.Config.options(section)
    for option in options:
      dict[option] = self.Config.get(section, option)
    return dict

