import decorations as Decorations
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import math
import numpy as np
import wx

from collections import defaultdict
from matplotlib import cbook
from matplotlib.patches import FancyArrowPatch
from matplotlib.pyplot import figure, show
from matplotlib.widgets import Cursor
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d import proj3d
from numpy import nanmin, nanmax
from scipy.optimize import curve_fit
from scipy.stats import nanmean
try:
  from matplotlib.patches import Polygon
  from scipy.spatial import ConvexHull
  AvailableHulls = True
except:
  AvailableHulls = False

CLUSTER_ID = 'clusterid'


HIDE_ALL       = 0
HIDE_SCATTERS  = 1
HIDE_CENTROIDS = 2
HIDE_HULLS     = 3
HIDE_ARROWS    = 4

FITTING_POLY_1  = 0
FITTING_POLY_2  = 1
FITTING_POLY_3  = 2
FITTING_LOG     = 3
FITTING_LIN_LOG = 4


class PlottingManager:
  def __init__(self, parent, data_manager):
    self.Data = data_manager

  def Initialize(self, GUI):
    self.GUI = GUI

    self.Scatters         = defaultdict()
    self.Thumbs           = defaultdict()
    self.Centroids        = defaultdict()
    self.Hulls            = defaultdict()
    self.Trajectories     = defaultdict()
    self.Arrows           = defaultdict(list)

    Figure1 = self.GUI.GetLeftFigure()
    Figure2 = self.GUI.GetRightFigure()

    self.ScatterPlotAxes = GUI.GetPlotAxes()
#    if (GUI.in_3D()):
#      self.ScatterPlotAxes  = Figure1.add_subplot(111, projection='3d')
#    else:
#      self.ScatterPlotAxes  = Figure1.add_subplot(111)

    self.EvolutionAxes1 = Figure2.add_subplot(311)
    self.EvolutionAxes2 = Figure2.add_subplot(312, sharex=self.EvolutionAxes1)
    self.EvolutionAxes3 = Figure2.add_subplot(313, sharex=self.EvolutionAxes1)

    plt.setp(self.EvolutionAxes1.get_xticklabels(), visible=True)
    plt.setp(self.EvolutionAxes2.get_xticklabels(), visible=True)

#    self.ClearAxes()

    self.Compute_Ranges()
    self.Rescale()

  def Replot(self, recompute_ranges=False):
    self.ScatterPlotAxes = self.GUI.GetPlotAxes()
    self.ClearAxes1()


    if (recompute_ranges):
      self.Compute_Ranges()

    self.ScatterGraph()
    self.Display_Objects()
    self.Rescale()
    return

  def ClearAnnotations(self):
    if hasattr(self, 'Annotations'):
      self.Annotations.hide_all()

  def ClearAxes(self):
    self.ClearAxes1()
    self.ClearAxes2()

  def ClearAxes1(self):
    self.GUI.ClearThumbs()
    self.ScatterPlotAxes.cla()
    self.Scatters.clear()
    self.Centroids.clear()
    self.Hulls.clear()
    self.Arrows.clear()
    self.Trajectories.clear()
    self.Thumbs.clear()
    if hasattr(self, 'Annotations'):
      self.Annotations.hide_all()

  def ClearAxes2(self):
    self.EvolutionAxes1.cla()
    self.EvolutionAxes2.cla()
    self.EvolutionAxes3.cla()

  def ComputePlots(self):
    self.ComputePlots1()
    self.ComputePlots2()

  def ComputePlots1(self):
    self.ClearAxes1()
    self.ScatterGraph()

  def ComputePlots2(self):
    self.ClearAxes2()
    self.MetricsCorrelation()
    self.ClustersCorrelation()
    self.MetricDispersion()
    self.UpdateCanvas2()

  def UpdateCanvas(self):
    self.UpdateCanvas1()
    self.UpdateCanvas2()

  def UpdateCanvas1(self):
    self.GUI.Canvas1.draw()

  def UpdateCanvas2(self):
    self.GUI.Canvas2.draw()

  def ScatterGraph(self):
    AnnotatedArtists = []

    #
    # Compute the scatter plot for this cluster
    #
    NumberOfFrames  = self.Data.GetNumberOfFrames()
    NumberOfObjects = self.Data.GetNumberOfObjects()
    FirstObject     = self.Data.GetFirstObject() - 2 # Include filtered and noise
    LastObject      = self.Data.GetLastObject()
    TasksPerFrame   = self.Data.GetTasksPerFrame()

    SelectedFrame   = self.GUI.GetSelectedFrame()
    SelectedCluster  = self.GUI.GetSelectedCluster()
    SelectedMetricX = self.GUI.GetSelectedMetricX()
    SelectedMetricY = self.GUI.GetSelectedMetricY()
    if self.GUI.in_3D():
      SelectedMetricZ = self.GUI.GetSelectedMetricZ()

#    cursor = Cursor(self.ScatterPlotAxes, useblit=True, color='black', linewidth=1 )

    for current_frame in range(1, NumberOfFrames+1):

      for current_object in range(FirstObject, LastObject+1):
        (r, g, b) = Decorations.RGBColor0_1(current_object)

        # 
        # Compute the scatter plot for this cluster
        # 
        xdata = self.Data.GetClusterData( current_frame, current_object, SelectedMetricX )
        ydata = self.Data.GetClusterData( current_frame, current_object, SelectedMetricY )
        if (len(xdata) == 0) or (len(ydata) == 0):
          continue
        if self.GUI.in_3D():
          zdata = self.Data.GetClusterData( current_frame, current_object, SelectedMetricZ )
          if (len(zdata) == 0):
            continue

        if (self.GUI.in_Trajectory_View()):
          if self.GUI.RatioX():
            xdata = xdata * TasksPerFrame[current_frame-1]
          if self.GUI.RatioY():
            ydata = ydata * TasksPerFrame[current_frame-1]
          if self.GUI.in_3D() and self.GUI.RatioZ():
            zdata = zdata * TasksPerFrame[current_frame-1]

        if self.GUI.in_3D():
          scatter = self.ScatterPlotAxes.scatter( xdata, ydata, zdata, color=(r, g, b), zorder=2, s=50, marker=Decorations.ChooseMarker(current_object), picker=True )
        else:
          scatter = self.ScatterPlotAxes.scatter( xdata, ydata, color=(r, g, b), zorder=2, s=50, marker=Decorations.ChooseMarker(current_object), picker=True )
        thumb = self.GUI.GetThumbAxes(current_frame).scatter( xdata, ydata, color=(r, g, b), zorder=2, s=100, marker=Decorations.ChooseMarker(current_object))
        thumb.set_visible( False )
        self.Thumbs[(current_object, current_frame)] = thumb

        scatter.set_gid( self.Data.PrettyCluster(current_object) )
        scatter.set_visible( False )
        self.Scatters[(current_object, current_frame)] = scatter
        AnnotatedArtists.append( scatter )

        # 
        # Compute the centroid for this cluster
        # 
        centroid_x = nanmean( xdata )
        centroid_y = nanmean( ydata )
        if self.GUI.in_3D():
          centroid_z = nanmean( zdata )
          centroid = self.ScatterPlotAxes.scatter( centroid_x, centroid_y, centroid_z, s=50, color=(r, g, b), edgecolor="black", marker="o", zorder=3, picker=True )
          self.Trajectories[(current_object, current_frame)] = (centroid_x, centroid_y, centroid_z)
        else:
          centroid = self.ScatterPlotAxes.scatter( centroid_x, centroid_y, s=50, color=(r, g, b), edgecolor="black", marker="o", zorder=3, picker=True)
          self.Trajectories[(current_object, current_frame)] = (centroid_x, centroid_y)

        centroid.set_gid( self.Data.PrettyCluster(current_object) )
        centroid.set_visible(False)
        self.Centroids[(current_object, current_frame)] = centroid
        AnnotatedArtists.append( centroid )

        # 
        # Compute the convex hull for this cluster
        # 
        if (AvailableHulls) and self.GUI.in_2D():
          points = np.array( zip(xdata, ydata) )
          if (len(points) < 3):
            continue

          try:
            hull     = ConvexHull( points, qhull_options='Pp' )
            vertices = hull.vertices
          except:
            vertices = []
 
          if (len(vertices) > 0):
            polygon_points = []
            for vertice in vertices:
              polygon_points.append( (points[vertice][0], points[vertice][1]) )
           
            hull_polygon = Polygon(polygon_points, closed=True, alpha=0.5, color=Decorations.RGBColor0_1(current_object), zorder=4, lw=10)
            hull_polygon.set_gid( self.Data.PrettyCluster(current_object) )
            hull_polygon.set_visible(False)
            self.Hulls[(current_object, current_frame)] = hull_polygon
            self.ScatterPlotAxes.add_artist( hull_polygon )
            AnnotatedArtists.append( hull_polygon )

    # Compute the arrows for the trajectories
    for current_object in range(FirstObject, LastObject+1):
      (r, g, b) = Decorations.RGBColor0_1(current_object)
      from_frame = 1
      to_frame   = from_frame + 1
      while (to_frame <= NumberOfFrames):
        tail = (current_object, from_frame)
        head = (current_object, to_frame)

        if not tail in self.Trajectories:
          from_frame = to_frame
          to_frame   = from_frame +1
          continue
        else:
          if not head in self.Trajectories:
            to_frame = to_frame + 1
            continue

        from_x = self.Trajectories[tail][0]
        from_y = self.Trajectories[tail][1]
        to_x   = self.Trajectories[head][0]
        to_y   = self.Trajectories[head][1]
        if self.GUI.in_3D():
          from_z = self.Trajectories[tail][2]
          to_z   = self.Trajectories[head][2]

        if ((to_x - from_x != 0) or (to_y - from_y != 0) or (self.GUI.in_3D() and (to_z - from_z != 0))):
          if (not (math.isnan(from_x) or math.isnan(from_y) or math.isnan(to_x) or math.isnan(to_y))):
            if (self.GUI.in_3D() and (not (math.isnan(from_z) or math.isnan(to_z)))):
              arrow = Arrow3D((from_x,to_x), (from_y, to_y), (from_z, to_z), arrowstyle='-|>', mutation_scale=20, color=(r, g, b), linewidth=1)
            else:
              arrow = FancyArrowPatch((from_x,from_y), (to_x,to_y), arrowstyle='-|>', mutation_scale=20, color=(r, g, b), linewidth=1)
            arrow.set_visible(False)
            self.Arrows[current_object].append(arrow)
            self.ScatterPlotAxes.add_artist(arrow)

        from_frame = to_frame
        to_frame   = from_frame +1

    self.Annotations = DataCursor(AnnotatedArtists)

  def ClustersCorrelation(self):
    NumberOfFrames  = self.Data.GetNumberOfFrames()
    SelectedMetric  = self.GUI.GetSelectedMetric()
    SelectedCluster = self.GUI.GetSelectedCluster()
    TasksPerFrame   = self.Data.GetTasksPerFrame()

    for cluster_id in self.GUI.GetDisplayingClusters():
      metric_min = -1
      metric_max = -1
      avg_max    = -1
      object_y   = [ ]
      (r, g, b)  = Decorations.RGBColor0_1( cluster_id )
      
      for frame in range(1, NumberOfFrames+1):
        data = self.Data.GetClusterData( frame, cluster_id, SelectedMetric )
        if (self.GUI.RatioMetric()):
          data = data * TasksPerFrame[frame-1]

        if (len(data) > 0):
          frame_min = nanmin(data) 
          if ((frame_min < metric_min) or (metric_min == -1)):
            metric_min = frame_min

          frame_max = nanmax(data) 
          if ((frame_max > metric_max) or (metric_max == -1)):
            metric_max = frame_max

          frame_avg = nanmean( data )
          if ((frame_avg > avg_max) or (avg_max == -1)):
            avg_max = frame_avg

      for frame in range(1, NumberOfFrames+1):
        data = self.Data.GetClusterData( frame, cluster_id, SelectedMetric )
        if (self.GUI.RatioMetric()):
          data = data * TasksPerFrame[frame-1]

        if (len(data) > 0):
          frame_avg = nanmean( data )
#          normalized_frame_avg = (frame_avg - metric_min) / (metric_max - metric_min)
#          normalized_frame_avg = (frame_avg / metric_max)
          if (avg_max == 0):
            normalized_frame_avg = 0
          else:
            normalized_frame_avg = (frame_avg / avg_max)
          object_y.append( normalized_frame_avg )

      if (cluster_id == SelectedCluster):
        line_size = 3
      else:
        line_size = 1

      if (len(object_y) > 1):
        self.EvolutionAxes2.plot (range(1, NumberOfFrames+1), object_y, label="Region "+str(self.Data.PrettyCluster(cluster_id)), lw=line_size, marker='o', ms=5, color=(r, g, b) )
      else:
        object_x = np.linspace(1, NumberOfFrames, len(object_y))
        self.EvolutionAxes2.scatter (object_x, object_y, color=(r, g, b), s=50, label="Region "+str(self.Data.PrettyCluster(cluster_id)))

    if (self.GUI.Toolbar2.GetToolState(self.GUI.Toolbar2.ON_LEGEND_CHECK) == True):
      self.EvolutionAxes2.legend(loc=(0,0), prop={'size':7})



  def MetricsCorrelation(self):
    NumberOfFrames  = self.Data.GetNumberOfFrames()
    SelectedFrame   = self.GUI.GetSelectedFrame()
    SelectedCluster  = self.GUI.GetSelectedCluster()
    SelectedMetric  = self.GUI.GetSelectedMetric()

    for metric in self.GUI.GetDisplayingMetrics():
      metric_id   = self.Data.GetMetricID( SelectedFrame, metric )
      (r, g, b)   = Decorations.MetricColor( metric_id )
      metric_min  = -1
      metric_max  = -1
      avg_max     = -1
      metric_y    = [ ]
      for frame in range(1, NumberOfFrames+1):
        data = self.Data.GetClusterData( frame, SelectedCluster, metric )

        if (len(data) > 0):
          frame_min = nanmin( data )
          if ((frame_min < metric_min) or (metric_min == -1)):
            metric_min = frame_min

          frame_max = nanmax( data )
          if ((frame_max > metric_max) or (metric_max == -1)):
            metric_max = frame_max

      for frame in range(1, NumberOfFrames+1):
        data = self.Data.GetClusterData( frame, SelectedCluster, metric )

        if (len(data) > 0):
          frame_avg = nanmean( data )
          if ((frame_avg > avg_max) or (avg_max == -1)):
            avg_max = frame_avg

      for frame in range(1, NumberOfFrames+1):
        data = self.Data.GetClusterData( frame, SelectedCluster, metric )

        if (len(data) > 0):
          frame_avg = nanmean( data )
#          normalized_frame_avg = (frame_avg - metric_min) / (metric_max - metric_min)
#          normalized_frame_avg = (frame_avg / metric_max)
          if (avg_max == 0):
            normalized_frame_avg = 0
          else:
            normalized_frame_avg = (frame_avg / avg_max)
          metric_y.append( normalized_frame_avg )
   
      if (metric.lower() == SelectedMetric.lower()):
        line_size = 3
      else:
        line_size = 1

      if (len(metric_y) > 1):
        self.EvolutionAxes1.plot (range(1, NumberOfFrames+1), metric_y, label=self.Data.PrettyMetric(metric), lw=line_size, marker='o', ms=5, color=(r, g, b) )
      else:
        metric_x = np.linspace(1, NumberOfFrames, len(metric_y))
        self.EvolutionAxes1.scatter (metric_x, metric_y, color=(r, g, b), label=self.Data.PrettyMetric(metric), s=50 )

    self.EvolutionAxes1.set_title('Region '+str(self.Data.PrettyCluster(SelectedCluster))+' metrics evolution')
    self.EvolutionAxes1.set_xticks(range(1, NumberOfFrames+1))
    self.EvolutionAxes1.set_xlim(1, NumberOfFrames)
    self.EvolutionAxes1.set_xbound(1, NumberOfFrames)

    if (self.GUI.Toolbar2.GetToolState(self.GUI.Toolbar2.ON_LEGEND_CHECK) == True):
      self.EvolutionAxes1.legend(loc=(0,0), prop={'size':7})


  def MetricDispersion(self):
    NumberOfFrames  = self.Data.GetNumberOfFrames()
    SelectedCluster  = self.GUI.GetSelectedCluster()
    SelectedMetric  = self.GUI.GetSelectedMetric()
    TasksPerFrame = self.Data.GetTasksPerFrame()

    metric_dispersion = [ ]
    metric_average    = [ ]
    metric_min        = [ ]
    metric_max        = [ ]
    metric_id = self.Data.GetMetricID(1, SelectedMetric)
    (r, g, b) = Decorations.MetricColor( metric_id )

    for frame in range(1, NumberOfFrames+1):
      frame_dispersion = self.Data.GetClusterData( frame, SelectedCluster, SelectedMetric )
      if (self.GUI.RatioMetric()):
        frame_dispersion = frame_dispersion * TasksPerFrame[frame-1]


      if (len(frame_dispersion) > 0):
        metric_dispersion.append( frame_dispersion[np.isfinite(frame_dispersion)] )
        metric_average.append( nanmean( frame_dispersion ) )
        metric_min.append( nanmin( frame_dispersion ) )
        metric_max.append( nanmax( frame_dispersion ) )

    self.EvolutionAxes3.set_title('Region '+str(self.Data.PrettyCluster(SelectedCluster))+' - '+SelectedMetric.upper()[2:])
    if (len(metric_dispersion) > 1):

      if (self.GUI.Toolbar2.GetToolState(self.GUI.Toolbar2.ON_BOXPLOT_CHECK)):
        self.EvolutionAxes3.boxplot(metric_dispersion)
        self.EvolutionAxes3.plot( range(1, NumberOfFrames+1), metric_average, lw=2, marker='o', ms=5, color=(r, g, b) )
      else:
        x = range(1, NumberOfFrames+1)
        self.EvolutionAxes3.plot(x, metric_min, ls='--', color=(r, g, b))
        self.EvolutionAxes3.plot(x, metric_average, marker='o', ms=5, label=self.Data.PrettyMetric(SelectedMetric), color=(r, g, b))
        self.EvolutionAxes3.plot(x, metric_max, ls='--', color=(r, g, b))
        self.EvolutionAxes3.fill_between(x, metric_min, metric_max, alpha=0.25, color=(r, g, b))
    else:
      metric_x = np.linspace(1, NumberOfFrames, len(metric_average))
      self.EvolutionAxes3.scatter (metric_x, metric_average, label=self.Data.PrettyMetric(SelectedMetric), color=(r, g, b))

    self.EvolutionAxes3.set_xticks(range(1, NumberOfFrames+1))
    self.EvolutionAxes3.set_xlim(1, NumberOfFrames)
    self.EvolutionAxes3.set_xbound(1, NumberOfFrames)

    if (self.GUI.GetFittingSamples() > 0):
      fit_x, fit_y, fit_label = self.Predict(metric_average)
      self.EvolutionAxes3.plot(fit_x, fit_y, ls=':', color='black', label='Fitting curve', marker='o', ms=4)
      self.EvolutionAxes3.set_xticks(range(1, len(fit_x)+1))
      self.EvolutionAxes3.set_xlim(1, len(fit_x))
      self.EvolutionAxes3.set_xbound(1, len(fit_x))
      self.GUI.SetFittingFormula( fit_label )
    else:
      self.GUI.SetFittingFormula( '' )

    if (self.GUI.Toolbar2.GetToolState(self.GUI.Toolbar2.ON_LEGEND_CHECK) == True):
      self.EvolutionAxes3.legend(loc=(0,0), prop={'size':7})


  def fitting_log(self, x, a, b):
    return a*np.log(x)+b

  def fitting_lin_log(self, x, a, b):
    return a*x*np.log(x)+b

  def fitting_poly_1(self, x, a, b):
    return (a*x) + b

  def fitting_poly_2(self, x, a, b, c):
    return a*np.power(x,2) + b*x + c

  def fitting_poly_3(self, x, a, b, c, d):
    return a*np.power(x,3) + b*np.power(x,2) + c*x + d

  def Predict(self, y):
    x = range(1, len(y)+1)

    x = [float(xn) for xn in x]
    y = [float(yn) for yn in y]
    x = np.array(x)
    y = np.array(y)

    type = self.GUI.GetFittingModel()

    if (type == FITTING_POLY_1):
      popt, pcov = curve_fit(self.fitting_poly_1, x, y)
      func = self.fitting_poly_1
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*x+"+str("%.2f" % popt[1])
    elif (type == FITTING_POLY_2) and (len(y) > 2):
      popt, pcov = curve_fit(self.fitting_poly_2, x, y)
      func = self.fitting_poly_2
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*x^2+"+str("%.2f" % popt[1])+"*x+"+str("%.2f" % popt[2])
    elif (type == FITTING_POLY_3) and (len(y) > 3):
      popt, pcov = curve_fit(self.fitting_poly_3, x, y)
      func = self.fitting_poly_3
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*x^3+"+str("%.2f" % popt[1])+"*x^2+"+str("%.2f" % popt[2])+"*x+"+str("%.2f" % popt[3])
    elif (type == FITTING_LOG):
      popt, pcov = curve_fit(self.fitting_log, x, y)
      func = self.fitting_log
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*log(x)+"+str("%.2f" % popt[1])
    elif (type == FITTING_LIN_LOG):
      popt, pcov = curve_fit(self.fitting_lin_log, x, y)
      func = self.fitting_lin_log
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*x*log(x)+"+str("%.2f" % popt[1])
    else:
      popt, pcov = curve_fit(self.fitting_poly_1, x, y)
      func = self.fitting_poly_1
      fit_label = "f(x)="+str("%.2f" % popt[0])+"*x+"+str("%.2f" % popt[1])

    fit_x = range(1, self.Data.GetNumberOfFrames() + self.GUI.GetFittingSamples() + 1)
    fit_y = [ ]
    for x in fit_x:
      fit_y.append( func(x, *popt) )

    fit_label = fit_label.replace("+-", "-")
    return fit_x, fit_y, fit_label

  def GetXRange(self, frame):
    return self.XRange[frame]
  def GetYRange(self, frame):
    return self.YRange[frame]
  def GetZRange(self, frame):
    return self.ZRange[frame]

  def Compute_Ranges(self):
    NumberOfFrames = self.Data.GetNumberOfFrames()
    TasksPerFrame  = self.Data.GetTasksPerFrame()

    self.XRange       = defaultdict()
    self.YRange       = defaultdict()
    self.ZRange       = defaultdict()
    self.XRangeRatio = defaultdict()
    self.YRangeRatio = defaultdict()
    self.ZRangeRatio = defaultdict()

    dim_x = self.GUI.GetSelectedMetricX()
    dim_y = self.GUI.GetSelectedMetricY()
    if (self.GUI.in_3D() == True):
      dim_z = self.GUI.GetSelectedMetricZ()

    X_local_min = []
    X_local_max = []
    Y_local_min = []
    Y_local_max = []
    Z_local_min = []
    Z_local_max = []

    for frame in range(1, NumberOfFrames+1):
      xdata = self.Data.GetFrameData( frame, dim_x )
      X_local_min.append( nanmin( xdata ) )
      X_local_max.append( nanmax( xdata ) )
      ydata = self.Data.GetFrameData( frame, dim_y )
      Y_local_min.append( nanmin( ydata ) )
      Y_local_max.append( nanmax( ydata ) )
      if self.GUI.in_3D():
        zdata = self.Data.GetFrameData( frame, dim_z )
        Z_local_min.append( nanmin( zdata ) )
        Z_local_max.append( nanmax( zdata ) )

    X_local_min_scaled = np.array(X_local_min) * np.array(TasksPerFrame)
    X_local_max_scaled = np.array(X_local_max) * np.array(TasksPerFrame)
    FrameWithMinX = X_local_min_scaled.argmin()
    FrameWithMaxX = X_local_max_scaled.argmax()
    Y_local_min_scaled = np.array(Y_local_min) * np.array(TasksPerFrame)
    Y_local_max_scaled = np.array(Y_local_max) * np.array(TasksPerFrame)
    FrameWithMinY = Y_local_min_scaled.argmin()
    FrameWithMaxY = Y_local_max_scaled.argmax()

    if self.GUI.in_3D():
      Z_local_min_scaled = np.array(Z_local_min) * np.array(TasksPerFrame)
      Z_local_max_scaled = np.array(Z_local_max) * np.array(TasksPerFrame)
      FrameWithMinZ = Z_local_min_scaled.argmin()
      FrameWithMaxZ = Z_local_max_scaled.argmax()

    XMinRatioPerFrame = []
    YMinRatioPerFrame = []
    ZMinRatioPerFrame = []

    XMaxRatioPerFrame = []
    YMaxRatioPerFrame = []
    ZMaxRatioPerFrame = []

    for frame in range(1, NumberOfFrames+1):
      ### Compute the ratios for the current trace with respect to the min/max values for this dimension
      XMinRatioPerFrame.append( TasksPerFrame[ FrameWithMinX ] / TasksPerFrame[ frame-1 ] )
      YMinRatioPerFrame.append( TasksPerFrame[ FrameWithMinY ] / TasksPerFrame[ frame-1 ] )
      if self.GUI.in_3D():
        ZMinRatioPerFrame.append( TasksPerFrame[ FrameWithMinZ ] / TasksPerFrame[ frame-1 ] )

      XMaxRatioPerFrame.append( TasksPerFrame[ FrameWithMaxX ] / TasksPerFrame[ frame-1 ] )
      YMaxRatioPerFrame.append( TasksPerFrame[ FrameWithMaxY ] / TasksPerFrame[ frame-1 ] )
      if self.GUI.in_3D():
        ZMaxRatioPerFrame.append( TasksPerFrame[ FrameWithMaxZ ] / TasksPerFrame[ frame-1 ] )

      ### Normalize the mins/maxs per trace with respect to the ratios
      self.XRangeRatio[frame] = ( X_local_min[ FrameWithMinX ] * XMinRatioPerFrame[frame-1], X_local_max[ FrameWithMaxX ] * XMaxRatioPerFrame[frame-1] )
      self.YRangeRatio[frame] = ( Y_local_min[ FrameWithMinY ] * YMinRatioPerFrame[frame-1], Y_local_max[ FrameWithMaxY ] * YMaxRatioPerFrame[frame-1] )
      if self.GUI.in_3D():
        self.ZRangeRatio[frame] = ( Z_local_min[ FrameWithMinZ ] * ZMinRatioPerFrame[frame-1], Z_local_max[ FrameWithMaxZ ] * ZMaxRatioPerFrame[frame-1] )

      ### Store the max range per frame
      self.XRange[frame] = ( X_local_min[frame-1], X_local_max[frame-1] )
      self.YRange[frame] = ( Y_local_min[frame-1], Y_local_max[frame-1] )
      if self.GUI.in_3D():
        self.ZRange[frame] = ( Z_local_min[frame-1], Z_local_max[frame-1] )
      else:
        self.ZRange[frame] = (0, 0)

    ### Store the max range for all frames
    self.XRangeGlobal = ( np.min( X_local_min ), np.max( X_local_max ) )
    self.YRangeGlobal = ( np.min( Y_local_min ), np.max( Y_local_max ) )
    if self.GUI.in_3D():
      self.ZRangeGlobal = ( np.min( Z_local_min ), np.max( Z_local_max ) )

    self.XRangeGlobalRatio = ( np.min( X_local_min_scaled ), np.max( X_local_max_scaled ) )
    self.YRangeGlobalRatio = ( np.min( Y_local_min_scaled ), np.max( Y_local_max_scaled ) )
    if self.GUI.in_3D():
      self.ZRangeGlobalRatio = ( np.min( Z_local_min_scaled ), np.max( Z_local_max_scaled ) )

  def Rescale(self):
    SelectedFrame = self.GUI.GetSelectedFrame()
    TasksPerFrame = self.Data.GetTasksPerFrame()

    if (not self.GUI.in_Trajectory_View()):
      if (not self.GUI.RatioX()):
        self.ScatterPlotAxes.set_xlim( self.XRangeGlobal )
      else:
        self.ScatterPlotAxes.set_xlim( self.XRangeRatio[SelectedFrame] )
      if (not self.GUI.RatioY()):
        self.ScatterPlotAxes.set_ylim( self.YRangeGlobal )
      else:
        self.ScatterPlotAxes.set_ylim( self.YRangeRatio[SelectedFrame] )
      if (self.GUI.in_3D()):
        if (not self.GUI.RatioZ()):
          self.ScatterPlotAxes.set_zlim( self.ZRangeGlobal )
        else:
          self.ScatterPlotAxes.set_zlim( self.ZRangeRatio[SelectedFrame] )
    else:
      if (self.GUI.RatioX()):
        self.ScatterPlotAxes.set_xlim( self.XRangeGlobalRatio )
      else:
        self.ScatterPlotAxes.set_xlim( self.XRangeGlobal )
      if (self.GUI.RatioY()):
        self.ScatterPlotAxes.set_ylim( self.YRangeGlobalRatio )
      else:
        self.ScatterPlotAxes.set_ylim( self.YRangeGlobal )
      if self.GUI.in_3D():
        if (self.GUI.RatioZ()):
          self.ScatterPlotAxes.set_zlim( self.ZRangeGlobalRatio )
        else:
          self.ScatterPlotAxes.set_zlim( self.ZRangeGlobal )

    xlim = self.ScatterPlotAxes.get_xlim()
    ylim = self.ScatterPlotAxes.get_ylim()
    if self.GUI.in_3D():
      zlim = self.ScatterPlotAxes.get_zlim()

    new_xlim = ( max(xlim[0], xlim[0] - (xlim[1] * 0.05)), xlim[1] + (xlim[1] * 0.05) )
    new_ylim = ( max(ylim[0], ylim[0] - (ylim[1] * 0.05)), ylim[1] + (ylim[1] * 0.05) )
    if self.GUI.in_3D():
      new_zlim = ( max(zlim[0], zlim[0] - (zlim[1] * 0.05)), zlim[1] + (zlim[1] * 0.05) )

    self.ScatterPlotAxes.set_xlim( new_xlim )
    self.ScatterPlotAxes.set_ylim( new_ylim )
    if self.GUI.in_3D():
      self.ScatterPlotAxes.set_zlim( new_zlim )

    if (self.GUI.LogX() == True):
      self.ScatterPlotAxes.set_xscale('symlog')
    else:
      self.ScatterPlotAxes.set_xscale('linear')
    if (self.GUI.LogY() == True):
      self.ScatterPlotAxes.set_yscale('symlog')
    else:
      self.ScatterPlotAxes.set_yscale('linear')
    if (self.GUI.in_3D()):
      if (self.GUI.LogZ() == True):
        self.ScatterPlotAxes.set_zscale('symlog')
      else:
        self.ScatterPlotAxes.set_zscale('linear')

    #
    # Configure axes labels
    # 
    self.ScatterPlotAxes.set_xlabel( self.Data.PrettyMetric( self.GUI.GetSelectedMetricX() ) )
    self.ScatterPlotAxes.set_ylabel( self.Data.PrettyMetric( self.GUI.GetSelectedMetricY() ) )
    if (self.GUI.in_3D() == True):
      self.ScatterPlotAxes.set_zlabel( self.Data.PrettyMetric( self.GUI.GetSelectedMetricZ() ) )
#    if not self.GUI.LogX() and not self.GUI.LogY():
#      self.ScatterPlotAxes.ticklabel_format(style='sci', scilimits=(0,0), axis='both')

    self.UpdateCanvas1()
    self.GUI.Timeline.Rescale()

  def ShowGrid1(self, show_or_hide):
    self.ScatterPlotAxes.grid( show_or_hide )
    self.UpdateCanvas1()

  def ShowGrid2(self, show_or_hide):
    self.EvolutionAxes1.grid( show_or_hide )
    self.EvolutionAxes2.grid( show_or_hide )
    self.EvolutionAxes3.grid( show_or_hide )
    self.UpdateCanvas2()
     
 
  def Display_Objects(self):
    self.Show_Points(True)
    self.Show_Centroids(True)
    self.Show_Hulls(True)
    self.Show_Arrows(True)
    self.UpdateCanvas1()
    self.GUI.Timeline.UpdateCanvas()
    return

  def Show_Points(self, show_artists):
    SelectedFrame = self.GUI.GetSelectedFrame()

    for (cluster_id, frame) in self.Scatters:
      scatter = self.Scatters[(cluster_id, frame)]
      draw_this_artist = ((show_artists)                            and
                          (self.GUI.DisplayingObject( cluster_id )) and
                          (self.GUI.ShowingPoints())                and 
                          (self.GUI.in_Trajectory_View() or frame == SelectedFrame))
      if (draw_this_artist):
        scatter.set_visible( True )
      else:
        scatter.set_visible( False ) 

    for (cluster_id, frame) in self.Thumbs:
      scatter = self.Thumbs[(cluster_id, frame)]
      draw_this_artist = (self.GUI.DisplayingObject( cluster_id ))
      if (draw_this_artist):
        scatter.set_visible( True )
      else:
        scatter.set_visible( False )

    return

  def Show_Centroids(self, show_artists):
    SelectedFrame = self.GUI.GetSelectedFrame()

    for (cluster_id, frame) in self.Centroids:
      centroid = self.Centroids[(cluster_id, frame)]
      draw_this_artist = ((show_artists)                            and
                          (self.GUI.DisplayingObject( cluster_id )) and
                          (self.GUI.ShowingCentroids())             and 
                          (self.GUI.in_Trajectory_View() or frame == SelectedFrame))
      if (draw_this_artist):
        centroid.set_visible(True)
      else:
        centroid.set_visible(False)
    return

  def Show_Hulls(self, show_artists):
    SelectedFrame = self.GUI.GetSelectedFrame()

    for (cluster_id, frame) in self.Hulls:
      hull = self.Hulls[(cluster_id, frame)]
      draw_this_artist = ((show_artists)                            and
                          (self.GUI.DisplayingObject( cluster_id )) and
                          (self.GUI.ShowingHulls())                 and 
                          (self.GUI.in_Trajectory_View() or frame == SelectedFrame))
      if (draw_this_artist):
        hull.set_visible(True)
      else:
        hull.set_visible(False)
    return

  def Show_Arrows(self, show_artists):
    FirstObject = self.Data.GetFirstObject()
    LastObject  = self.Data.GetLastObject()

    for i in self.Arrows:
      for arrow in self.Arrows[i]:
        if ((show_artists)                  and 
            (self.GUI.in_Trajectory_View()) and 
            (self.GUI.DisplayingObject( i ))):
          arrow.set_visible(True)
        else:
          arrow.set_visible(False)
    return



class DataCursor(object):
    """A simple data cursor widget that displays the x,y location of a
    matplotlib artist when it is selected."""
    def __init__(self, artists, tolerance=5, offsets=(-20, 20), 
                 template='Cluster %d\n[%0.2f, %0.2f]', display_all=False):
        """Create the data cursor and connect it to the relevant figure.
        "artists" is the matplotlib artist or sequence of artists that will be 
            selected. 
        "tolerance" is the radius (in points) that the mouse click must be
            within to select the artist.
        "offsets" is a tuple of (x,y) offsets in points from the selected
            point to the displayed annotation box
        "template" is the format string to be used. Note: For compatibility
            with older versions of python, this uses the old-style (%) 
            formatting specification.
        "display_all" controls whether more than one annotation box will
            be shown if there are multiple axes.  Only one will be shown
            per-axis, regardless. 
        """
        self.template = template
        self.offsets = offsets
        self.display_all = display_all
        if not cbook.iterable(artists):
            artists = [artists]
        self.artists = artists
        self.axes = tuple(set(art.axes for art in self.artists))
        self.figures = tuple(set(ax.figure for ax in self.axes))

        self.annotations = {}
        for ax in self.axes:
            self.annotations[ax] = self.annotate(ax)

        for artist in self.artists:
            artist.set_picker(tolerance)
        for fig in self.figures:
            fig.canvas.mpl_connect('pick_event', self)

    def annotate(self, ax):
        """Draws and hides the annotation box for the given axis "ax"."""
        annotation = ax.annotate(self.template, xy=(0, 0), ha='center',
                xytext=self.offsets, textcoords='offset points', va='bottom',
                bbox=dict(boxstyle='round,pad=0.5', fc='yellow', alpha=0.5),
                arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0')
                )
        annotation.set_visible(False)
        return annotation

    def hide_all(self):
      for ann in self.annotations.values():
        ann.set_visible(False)
      

    def __call__(self, event):
        """Intended to be called through "mpl_connect"."""
        # Rather than trying to interpolate, just display the clicked coords
        # This will only be called if it's within "tolerance", anyway.
      
        ClickedObject   = event.artist
        ClickedObjectID = ClickedObject.get_gid()

        if not ClickedObject.get_visible():
          return

        x, y = event.mouseevent.xdata, event.mouseevent.ydata
        annotation = self.annotations[event.artist.axes]
        if x is not None:
            if not self.display_all:
                # Hide any other annotation boxes...
                for ann in self.annotations.values():
                    ann.set_visible(False)
            # Update the annotation in the current axis..
            annotation.xy = x, y
            annotation.set_text(self.template % (ClickedObjectID, x, y))
            annotation.set_visible(True)
            event.canvas.draw()

class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        FancyArrowPatch.__init__(self, (0,0), (0,0), *args, **kwargs)
        self._verts3d = xs, ys, zs

    def draw(self, renderer):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
        self.set_positions((xs[0],ys[0]),(xs[1],ys[1]))
        FancyArrowPatch.draw(self, renderer)

