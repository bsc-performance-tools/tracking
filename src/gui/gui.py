#!/usr/bin/env python

#
# GUI
#
import wxversion
wxversion.ensureMinimal('2.8')
import wx
import wx.aui
import wx.lib.buttons as buttons

import matplotlib
matplotlib.use('WXAgg')

import matplotlib.mlab as mlab

from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
from collections import defaultdict
from matplotlib.patches import Polygon
from matplotlib.patches import FancyArrowPatch
import matplotlib.animation as animation
from scipy.interpolate import interp1d
from matplotlib.lines import Line2D
import matplotlib.pyplot as plt

import sys
import numpy as np
import convex_hull as ch

import time

INSTALLATION_PATH='/home/gllort/Work/tests/matplotlib/gui-clean/'

matplotlib.rcParams.update({'font.size': 10})

metrics_list = ['ipc', 'pm_inst_cmpl'] # XXX Read this from the CSV
selected_color     = (0.20, 0.75, 0.80)
non_selected_color = (1, 1, 1)

#
# Defines
#
FirstCluster = 5
LastCluster  = FirstCluster
DIM_X = 'ipc'
DIM_Y = 'pm_inst_cmpl'
CLUSTER_ID = 'cluster_id'
VIEW_FRAMES = "frames"
VIEW_TRAJECTORY = "trajectory"

#
# Global variables
#
Data = { }
Callers = { }
NumberOfFrames = 0
NumberOfClusters = 0
X_min_global = Y_min_global = X_max_global = Y_max_global = -1

Callers = mlab.csv2rec('3D') ### XXX We have to load the callers for all experiments



#
# PRV color palette
#
ColorPalette = [ ( 117, 195, 255 ), (   0,   0, 255 ), ( 255, 255, 255 ), ( 255,   0,   0 ), ( 255,   0, 174 ), ( 179,   0,   0 ), ( 0,   255,   0 ), ( 255, 255,   0 ), ( 235,   0,   0 ), (   0, 162,   0 ), ( 255,   0, 255 ), ( 100, 100,  177 ), ( 172, 174,  41 ), ( 255, 144,  26 ), (   2, 255, 177 ), ( 192, 224,   0 ), (  66,  66,  66 ), ( 189, 168, 100 ), (  95, 200,   0 ), ( 203,  60,  69 ), (   0, 109, 255 ), ( 200,  61,  68 ), ( 200,  66,   0 ), (   0,  41,   0 ), ( 139, 121, 177 ), ( 116, 116, 116 ), ( 200,  50,  89 ), ( 255, 171,  98 ), (   0,  68, 189 ), (  52,  43,   0 ), ( 255,  46,   0 ), ( 100, 216,  32 ), (   0,   0, 112 ), ( 105, 105,   0 ), ( 132,  75, 255 ), ( 184, 232,   0 ), (   0, 109, 112 ), ( 189, 168, 100 ), ( 132,  75,  75 ), ( 255,  75,  75 ), ( 255,  20,   0 ), (  52,   0,   0 ), (   0,  66,   0 ), ( 184, 132,   0 ), ( 100,  16,  32 ), ( 146, 255, 255 ), (   0,  23,  37 ), ( 146,   0, 255 ), (   0, 138, 119 ) ]

def PRVColor(cluster_id):
    idx = cluster_id + 1
    if (idx > len(ColorPalette)): idx = len(ColorPalette) - 1
    r = float(float(ColorPalette[idx][0]) / 255)
    g = float(float(ColorPalette[idx][1]) / 255)
    b = float(float(ColorPalette[idx][2]) / 255)
    rgb = (r, g, b)
    return rgb

def MetricColor(metric):
    global metrics_list
    return PRVColor( metrics_list.index(metric) + 25 )

def ClusterOffset(cluster_id):
    return cluster_id - FirstCluster + 1


class ToolbarL(NavigationToolbar):
  #
  # Extend the default wx toolbar with your own event handlers
  #  
  ON_SCATTER_CHECK = wx.NewId()
  ON_HULL_CHECK = wx.NewId()
  ON_CENTROID_CHECK = wx.NewId()
  ON_LOGARITHM_CHECK = wx.NewId()
  def __init__(self, canvas):
    NavigationToolbar.__init__(self, canvas)

    self.AddSeparator()

    self.AddCheckTool(
      self.ON_SCATTER_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/scatter.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/scatter.png'),
      'Draw the points of the cluster',
      'Draw the points of the cluster')

    self.AddCheckTool(
      self.ON_HULL_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/hull.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/hull.png'),
      'Draw the perimeter of the cluster',
      'Draw the perimeter of the cluster')

    self.AddCheckTool(
      self.ON_CENTROID_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/centroid.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/centroid.png'),
      'Draw the centroid of the cluster',
      'Draw the centroid of the cluster')

    self.AddCheckTool(
      self.ON_LOGARITHM_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/logscale.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/logscale.png'),
      'Draw logarithmic scale',
      'Draw logarithmic scale')

    self.ToggleTool(self.ON_SCATTER_CHECK, True)

    wx.EVT_TOOL(self, self.ON_SCATTER_CHECK, self.on_Scatter_Check)
    wx.EVT_TOOL(self, self.ON_HULL_CHECK, self.on_Hull_Check)
    wx.EVT_TOOL(self, self.ON_CENTROID_CHECK, self.on_Centroid_Check)
    wx.EVT_TOOL(self, self.ON_LOGARITHM_CHECK, self.on_Logarithm_Check)
   
  def on_Scatter_Check(self, event):
      app.main.PlotL()

  def on_Hull_Check(self, event):
      app.main.PlotL()

  def on_Centroid_Check(self, event):
      app.main.PlotL()

  def on_Logarithm_Check(self, event):
      app.main.PlotL()

class ToolbarR(NavigationToolbar):
  #
  # Extend the default wx toolbar with your own event handlers
  #  
  ON_BOXPLOT_CHECK = wx.NewId()
  ON_INTERPOLATE_CHECK = wx.NewId()
  ON_LEGEND_CHECK = wx.NewId()

  def __init__(self, canvas):
    NavigationToolbar.__init__(self, canvas)

    self.AddSeparator()

    self.AddCheckTool(
      self.ON_BOXPLOT_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/boxplot.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/boxplot.png'),
      'Switch between boxplots and trend lines',
      'Switch between boxplots and trend lines')

    self.AddCheckTool(
      self.ON_INTERPOLATE_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/heatmap.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/heatmap.png'),
      'Interpolate the heat map',
      'Interpolate the heat map')

    self.AddCheckTool(
      self.ON_LEGEND_CHECK,
      wx.Bitmap(INSTALLATION_PATH+'icons/legend.png'),
      wx.Bitmap(INSTALLATION_PATH+'icons/legend.png'),
      'Show plots legends',
      'Show plots legends')

    self.ToggleTool(self.ON_LEGEND_CHECK, True)

    wx.EVT_TOOL(self, self.ON_BOXPLOT_CHECK, self.on_Boxplot_Check)
    wx.EVT_TOOL(self, self.ON_INTERPOLATE_CHECK, self.on_Interpolate_Check)
    wx.EVT_TOOL(self, self.ON_LEGEND_CHECK, self.on_Legend_Check)

  def on_Boxplot_Check(self, event):
      app.main.PlotR()

  def on_Interpolate_Check(self, event):
      app.main.PlotR()

  def on_Legend_Check(self, event):
      app.main.PlotR()


class MainFrame(wx.Frame, animation.TimedAnimation):
  #
  # The main frame of the application
  #

  def __init__(self, parent, title):
    #
    # A "-1" in the size parameter instructs wxWidgets to use the default size.
    #
    wx.Frame.__init__(self, parent, title=title)

    self.FocusOnFrame = 1
    self.FocusOnCluster    = FirstCluster
    self.FocusOnMetric     = DIM_X
    self.CurrentView = VIEW_FRAMES
    self.Scatters = defaultdict()
    self.Hulls = defaultdict()
    self.Centroids = defaultdict()
    self.Trajectories = defaultdict()
    self.Arrows = defaultdict(list)
    self.Paths = defaultdict()
#    self.Ti = defaultdict()
    self.Xi = defaultdict()
    self.Yi = defaultdict()
    self.DisplayingClusters = defaultdict()
    self.DisplayingMetrics = defaultdict()
    self.PointsWithAnnotation = []
    self.Bins = 25

    self.bmp_display   = wx.Bitmap(INSTALLATION_PATH+"icons/display.png")
    self.bmp_nodisplay = wx.Bitmap(INSTALLATION_PATH+"icons/nodisplay.png")
    self.bmp_play      = wx.Bitmap(INSTALLATION_PATH+"icons/play.png")
    self.bmp_prev      = wx.Bitmap(INSTALLATION_PATH+"icons/prev.png")
    self.bmp_next      = wx.Bitmap(INSTALLATION_PATH+"icons/next.png")

    self.LoadData()
    self.Create_Main_Panel()
    self.SetupView()

    self.ComputePlots()

    self.PlotAll()
    self.FigR.subplots_adjust(left=0.06, bottom=0.05, right=0.99, top=0.95, wspace=0.11, hspace=0.19)

  def Create_Main_Panel(self):
    self.Panel = wx.Panel(self)
        
    #
    # Create the mpl Figure and FigCanvas objects. 
    # 5x4 inches, 100 dots-per-inch
    #
    self.DPI = 100
    self.Fig = Figure((5.0, 4.0), dpi=self.DPI)
    self.Fig.set_facecolor("#FFFFFF")
    self.Canvas = FigCanvas(self.Panel, -1, self.Fig)

    self.FigR = Figure((5.0, 4.0), dpi=self.DPI)
    self.FigR.set_facecolor("#FFFFFF")
    self.ColorBar = ()
    self.CanvasR = FigCanvas(self.Panel, -1, self.FigR)

    self.AxesL  = self.Fig.add_subplot(111)
 

    #
    # Create the navigation toolbar, tied to the canvas
    #
    self.Toolbar = ToolbarL(self.Canvas)
    self.ToolbarR = ToolbarR(self.CanvasR)


    #
    # Create the buttons
    #
    self.ViewModeBox = wx.StaticBox(self.Panel, -1, "View mode", style=wx.EXPAND)
    self.ViewModeBoxSizer = wx.StaticBoxSizer(self.ViewModeBox, wx.HORIZONTAL)
    self.ViewButton = wx.ToggleButton(self.Panel, -1, "Switch to "+VIEW_TRAJECTORY+" view")
    self.ViewModeBoxSizer.Add( self.ViewButton, 1, wx.EXPAND )

    self.FrameBox = wx.StaticBox(self.Panel, -1, "Select frame", style=wx.EXPAND)
    self.FrameBoxSizer = wx.StaticBoxSizer(self.FrameBox, wx.HORIZONTAL)


    self.Bind(wx.EVT_TOGGLEBUTTON, self.on_View_Button, self.ViewButton)
    self.PrevButton = wx.BitmapButton(self.Panel, -1, bitmap=self.bmp_prev)
    self.Bind(wx.EVT_BUTTON, self.on_Prev_Button, self.PrevButton)
    self.NextButton = wx.BitmapButton(self.Panel, -1, bitmap=self.bmp_next)
    self.Bind(wx.EVT_BUTTON, self.on_Next_Button, self.NextButton)
    self.PlayButton = wx.BitmapButton(self.Panel, -1, bitmap=self.bmp_play)
    self.Bind(wx.EVT_BUTTON, self.on_Play_Button, self.PlayButton)
    self.FrameSlider = wx.Slider(self.Panel, -1,
      value=self.FocusOnFrame, 
      minValue=1,
      maxValue=NumberOfFrames,
      style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS | wx.SL_LABELS)
    self.FrameSlider.SetTickFreq(1, 1)
    self.Bind(wx.EVT_COMMAND_SCROLL_CHANGED, self.on_Frame_Change, self.FrameSlider)

    self.FrameBoxSizer.Add( self.PlayButton, 0, wx.ALIGN_LEFT  | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.FrameBoxSizer.Add( self.PrevButton, 0, wx.ALIGN_LEFT  | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.FrameBoxSizer.Add( self.FrameSlider,     1, wx.EXPAND      | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.FrameBoxSizer.Add( self.NextButton, 0, wx.ALIGN_RIGHT | wx.ALL | wx.ALIGN_CENTER_VERTICAL)

    #
    # 
    #
    self.HeatMapConfigBox = wx.StaticBox(self.Panel, -1, "Heat map resolution", style=wx.EXPAND)
    self.HeatMapConfigBoxSizer = wx.StaticBoxSizer(self.HeatMapConfigBox, wx.HORIZONTAL)
    self.ResolutionSlider = wx.Slider(self.Panel, -1,
      value=25,
      minValue=5,
      maxValue=50,
      style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS | wx.SL_LABELS)
    self.FrameSlider.SetTickFreq(1, 1)
    self.Bind(wx.EVT_COMMAND_SCROLL_CHANGED, self.on_Resolution_Change, self.ResolutionSlider)
    self.HeatMapConfigBoxSizer.Add( self.ResolutionSlider, 1, wx.EXPAND )

    self.PredictionBox = wx.StaticBox(self.Panel, -1, "Predict", style=wx.EXPAND)
    self.PredictionBoxSizer = wx.StaticBoxSizer(self.PredictionBox, wx.HORIZONTAL )
    self.PredictExperiments = wx.TextCtrl(self.Panel, -1, size=(100, 26), style=wx.TE_PROCESS_ENTER)
    self.PredictExperiments.SetValue("0")

    self.Bind(wx.EVT_TEXT_ENTER, self.on_Prediction_Change, self.PredictExperiments)

    RegressionTypeList = ['Linear', 'Curve-2', 'Curve-3']
    self.RegressionTypeCombo = wx.ComboBox( self.Panel, -1, value=RegressionTypeList[0], size=(100, 26), choices=RegressionTypeList, style=wx.CB_DROPDOWN)
    self.Bind(wx.EVT_COMBOBOX, self.on_RegressionType_Change, self.RegressionTypeCombo)

    self.PredictionBoxSizer.Add( self.PredictExperiments, 1, wx.ALIGN_CENTER_VERTICAL)
    self.PredictionBoxSizer.Add( self.RegressionTypeCombo, 1, wx.ALIGN_CENTER_VERTICAL)

    #
    # Create timers
    #
    self.PlayTimer = wx.Timer(self)
    self.Bind(wx.EVT_TIMER, self.on_Playing_Timer, self.PlayTimer)

    #
    # Clusters scroll
    #
    self.ClustersScroll     = wx.ScrolledWindow(self.Panel, -1, style=wx.VSCROLL | wx.HSCROLL | wx.ALWAYS_SHOW_SB)
    self.ClustersScrollList = []
    for cluster_id in range(FirstCluster, LastCluster+1):
      ClustersDisplayToggle = wx.BitmapButton(self.ClustersScroll, -1, bitmap=self.bmp_display, size=(26, 26), name=str(cluster_id))   
      ClustersFocusToggle   = wx.ToggleButton(self.ClustersScroll, -1, "Region "+str(cluster_id-FirstCluster+1), size=(-1,26), name=str(cluster_id))
      self.Bind(wx.EVT_BUTTON, self.on_Clusters_Display, ClustersDisplayToggle)
      self.Bind(wx.EVT_TOGGLEBUTTON, self.on_Clusters_Focus, ClustersFocusToggle)
      c = PRVColor(cluster_id)
      ClustersDisplayToggle.SetBackgroundColour(wx.Colour(c[0]*255, c[1]*255, c[2]*255))
      self.ClustersScrollList.append((ClustersDisplayToggle, ClustersFocusToggle))
      self.DisplayingClusters[cluster_id] = True

    ClustersScrollSizer = wx.BoxSizer(wx.VERTICAL)
    for controls in self.ClustersScrollList:
      ClustersScrollSizerH = wx.BoxSizer(wx.HORIZONTAL)
      ClustersScrollSizerH.Add(controls[0], 0, wx.ALIGN_LEFT)
      ClustersScrollSizerH.Add(controls[1], 0, wx.ALIGN_LEFT)
      ClustersScrollSizer.Add(ClustersScrollSizerH)

    self.ClustersLegendBox = wx.StaticBox(self.Panel, -1, "Display regions", style=wx.EXPAND)
    self.ClustersLegendBoxSizer = wx.StaticBoxSizer(self.ClustersLegendBox, wx.VERTICAL)

    self.ClustersScroll.SetSizer(ClustersScrollSizer)
    self.ClustersScroll.SetScrollbars(1, 1, 1, 1)
    self.ClustersLegendBoxSizer.Add( self.ClustersScroll, 1, wx.EXPAND )
    self.set_Cluster_Focus(FirstCluster)

    #
    # Metrics scroll
    #

    self.MetricsScroll     = wx.ScrolledWindow(self.Panel, -1, style=wx.VSCROLL | wx.HSCROLL | wx.ALWAYS_SHOW_SB)
    self.MetricsScrollList = []
    for metric in metrics_list:
      MetricsDisplayToggle = wx.BitmapButton(self.MetricsScroll, -1, bitmap=self.bmp_display, size=(26, 26), name=metric)
      MetricsFocusToggle   = wx.ToggleButton(self.MetricsScroll, -1, metric.upper(), size=(-1,26), name=metric)
      self.Bind(wx.EVT_BUTTON, self.on_Metrics_Display, MetricsDisplayToggle)
      self.Bind(wx.EVT_TOGGLEBUTTON, self.on_Metrics_Focus, MetricsFocusToggle)
      c = MetricColor(metric)
      MetricsDisplayToggle.SetBackgroundColour(wx.Colour(c[0]*255, c[1]*255, c[2]*255))
      self.MetricsScrollList.append((MetricsDisplayToggle, MetricsFocusToggle))
      self.DisplayingMetrics[metric] = True

    MetricsScrollSizer = wx.BoxSizer(wx.VERTICAL)
    for controls in self.MetricsScrollList:
      MetricsScrollSizerH = wx.BoxSizer(wx.HORIZONTAL)
      MetricsScrollSizerH.Add(controls[0], 0, wx.ALIGN_LEFT)
      MetricsScrollSizerH.Add(controls[1], 0, wx.ALIGN_LEFT)
      MetricsScrollSizer.Add(MetricsScrollSizerH)

    self.MetricsLegendBox = wx.StaticBox(self.Panel, -1, "Display metrics", style=wx.EXPAND)
    self.MetricsLegendBoxSizer = wx.StaticBoxSizer(self.MetricsLegendBox, wx.VERTICAL)

    self.MetricsScroll.SetSizer(MetricsScrollSizer)
    self.MetricsScroll.SetScrollbars(1, 1, 1, 1)
    self.MetricsLegendBoxSizer.Add( self.MetricsScroll, 1, wx.EXPAND )
    self.set_Metric_Focus(metrics_list[0])

    #
    # Layout with box sizers
    #
    self.PlotBoxL = wx.BoxSizer(wx.VERTICAL)
    self.PlotBoxL.Add(self.Canvas, 1, wx.LEFT | wx.TOP | wx.GROW)
    self.PlotBoxL.Add(self.Toolbar, 0)
    self.PlotBoxL.AddSpacer(10)

    self.PlotBoxR = wx.BoxSizer(wx.VERTICAL)
    self.PlotBoxR.Add(self.CanvasR, 1, wx.LEFT | wx.TOP | wx.GROW)
    self.PlotBoxR.Add(self.ToolbarR, 0)

    self.PlotControlsBox1 = wx.BoxSizer(wx.HORIZONTAL)
    self.PlotControlsBox1.Add(self.ViewModeBoxSizer, 1, wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL)

    self.PlotControlsBox2 = wx.BoxSizer(wx.HORIZONTAL)
    self.PlotControlsBox2.Add( self.FrameBoxSizer, 1, wx.EXPAND )

    self.PlotControlsBox3 = wx.BoxSizer(wx.HORIZONTAL)
    self.PlotControlsBox3.Add( self.HeatMapConfigBoxSizer, 1, wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.PlotControlsBox3.Add( self.PredictionBoxSizer, 1, wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
  
    self.PlotBoxL.Add(self.PlotControlsBox1, 0, flag = wx.TOP | wx.EXPAND)
    self.PlotBoxL.Add(self.PlotControlsBox2, 0, flag = wx.TOP | wx.EXPAND)
    self.PlotBoxL.Add(self.PlotControlsBox3, 0, flag = wx.TOP | wx.EXPAND)

    self.LegendBox = wx.BoxSizer(wx.VERTICAL)
    self.LegendBox.Add(self.ClustersLegendBoxSizer, 1, wx.EXPAND)
    self.LegendBox.Add(self.MetricsLegendBoxSizer, 1, wx.EXPAND)

    self.WindowBox = wx.BoxSizer(wx.HORIZONTAL)
    self.WindowBox.Add(self.PlotBoxL, 3, wx.EXPAND)
    self.WindowBox.Add(self.LegendBox, 1, wx.EXPAND)
    self.WindowBox.Add(self.PlotBoxR, 5, wx.EXPAND)

    self.Panel.SetSizer(self.WindowBox)
    self.WindowBox.Fit(self)

    self.SetMinSize(self.GetSize())

  def LoadData(self):
    global X_min_global
    global Y_min_global
    global X_max_global
    global Y_max_global
    global LastCluster
    global NumberOfClusters

    for frame in range(1, NumberOfFrames+1):
      csv = sys.argv[frame]
      Data[frame] = mlab.csv2rec(csv)

      X_min = np.min(Data[frame][DIM_X]) 
      Y_min = np.min(Data[frame][DIM_Y])
      X_max = np.max(Data[frame][DIM_X])
      Y_max = np.max(Data[frame][DIM_Y])
      if (X_min_global == -1) or (X_min < X_min_global):
        X_min_global = X_min
      if (Y_min_global == -1) or (Y_min < Y_min_global):
        Y_min_global = Y_min
      if (X_max_global == -1) or (X_max > X_max_global):
        X_max_global = X_max
      if (Y_max_global == -1) or (Y_max > Y_max_global):
        Y_max_global = Y_max

      LastCluster = np.max(Data[frame][CLUSTER_ID])
      NumberOfClusters = LastCluster - FirstCluster + 1

  def ComputePlots(self):
    for frame in range(1, NumberOfFrames+1):
      for cluster_id in range(FirstCluster, LastCluster+1):
        #
        # Compute the scatter plot for this cluster
        #
        self.ClusterData = Data[frame][Data[frame][CLUSTER_ID] == cluster_id]
        cluster_scatter = self.AxesL.scatter(self.ClusterData[DIM_X], self.ClusterData[DIM_Y], picker=True, color=PRVColor(cluster_id), marker=".", zorder=2)
        for i in range(self.ClusterData.size):
          point = (self.ClusterData[DIM_X][i], self.ClusterData[DIM_Y][i])
          annotation = self.AxesL.annotate("Mouseover point %s" % i,
            xy=point, xycoords='data',
            xytext=(i + 1, i), textcoords='data',
            horizontalalignment="left",
            arrowprops=dict(arrowstyle="simple", connectionstyle="arc3,rad=-0.2"),
            bbox=dict(boxstyle="round", facecolor="w", edgecolor="0.5")
          )
          annotation.set_visible(True)

          self.PointsWithAnnotation.append([point, annotation]) 

        #
        # Compute the convex hull for this cluster
        #
        cluster_hull = ch.convexHull( self.ClusterData[[ DIM_X, DIM_Y ]] )
        vertices = []
        for v in cluster_hull:
          vertices.append([v[0], v[1]])
        cluster_hull = Polygon(vertices, True, color=PRVColor(cluster_id), zorder=1)

        #
        # Compute the centroid for this cluster
        #
        centroid_x = np.average(self.ClusterData[DIM_X])
        centroid_y = np.average(self.ClusterData[DIM_Y])
        cluster_centroid = self.AxesL.scatter(centroid_x, centroid_y, s=50, color=PRVColor(cluster_id), edgecolor="black", marker="o", zorder=3)
        
        # Save the trajectory for each cluster
        self.Trajectories[(cluster_id, frame)] = (centroid_x, centroid_y)

        #
        # Store the plots in arrays 
        #
#        self.Scatters[frame,cluster_id].append( cluster_scatter )
#        self.Hulls[frame].append( cluster_hull )
#        self.Centroids[frame].append (cluster_centroid)
        self.Scatters[(cluster_id,frame)] = cluster_scatter 
        self.Hulls[(cluster_id,frame)] = cluster_hull
        self.Centroids[(cluster_id,frame)] = cluster_centroid


    # Compute the arrows for the trajectories
    for cluster_id in range(FirstCluster, LastCluster+1):
      for frame in range(1, NumberOfFrames):
        from_x = self.Trajectories[(cluster_id, frame)][0]
        from_y = self.Trajectories[(cluster_id, frame)][1]
        to_x = self.Trajectories[(cluster_id, frame+1)][0]
        to_y = self.Trajectories[(cluster_id, frame+1)][1]

        arrow = FancyArrowPatch((from_x,from_y), (to_x,to_y), arrowstyle='-|>', mutation_scale=20, color=PRVColor(cluster_id), linewidth=1)
        self.Arrows[cluster_id].append(arrow)
    # Compute the interpolated trajectory
    self.AnimationSeconds = 5.0
    self.AnimationSamples = 100
    self.AnimationPeriod  = (self.AnimationSeconds / self.AnimationSamples)
    self.T = np.linspace(0, NumberOfFrames, self.AnimationSamples)

    for cluster_id in range(FirstCluster, LastCluster+1):
      Xs = []
      Ys = []
      for frame in range(1, NumberOfFrames+1):
        # Sequence of X and Y coords for this object 
        Xs.append( self.Trajectories[(cluster_id, frame)][0] )
        Ys.append( self.Trajectories[(cluster_id, frame)][1] )

      Ts = np.linspace(0, len(Xs), len(Xs))
  
      if (NumberOfFrames < 4):
        kind="linear"
      else:
        kind="linear"

      fx = interp1d(Ts, Xs, kind=kind)
      fy = interp1d(Ts, Ys, kind=kind)

      # Generate the interpolated array
      self.Xi[cluster_id] = fx(self.T)
      self.Yi[cluster_id] = fy(self.T)
       
      # Artists for the interpolated path
      Path = Line2D([], [], color='black')
      Tail = Line2D([], [], color='red', linewidth=3)
      Head = Line2D([], [], color='red', marker='o', markeredgecolor='black')
      self.Paths[cluster_id] = (Path, Tail, Head)
      self.AxesL.add_line(Path)
      self.AxesL.add_line(Tail)
      self.AxesL.add_line(Head)

  def DrawSingleFrame(self, frame, clear=True):
     if (clear == True):
       self.AxesL.clear()

     self.AxesL.set_xlim(X_min_global, X_max_global)
     self.AxesL.set_ylim(Y_min_global, Y_max_global)
     if (self.Toolbar.GetToolState(self.Toolbar.ON_LOGARITHM_CHECK) == True):
#       self.AxesL.set_xscale('log', basey=10)
       self.AxesL.set_yscale('log', basey=10)

     if (self.Toolbar.GetToolState(self.Toolbar.ON_CENTROID_CHECK) == True):
       for cluster_id, f in self.Centroids:
         if (self.DisplayingClusters[cluster_id] == True) and (frame == f):
           centroid = self.Centroids[(cluster_id,frame)]
           self.AxesL.add_artist(centroid)

     if (self.Toolbar.GetToolState(self.Toolbar.ON_SCATTER_CHECK) == True):
       for cluster_id, f in self.Scatters:
         if (self.DisplayingClusters[cluster_id] == True) and (frame == f):
           scatter = self.Scatters[(cluster_id,frame)]
           self.AxesL.add_artist(scatter)

     if (self.Toolbar.GetToolState(self.Toolbar.ON_HULL_CHECK) == True):
       for cluster_id, f in self.Hulls:
         if (self.DisplayingClusters[cluster_id] == True) and (frame == f):
           hull = self.Hulls[(cluster_id,frame)]
           self.AxesL.add_artist(hull)

     self.AxesL.set_xlabel("IPC")
     self.AxesL.set_ylabel("Instructions")
#     self.Canvas.draw()

  
  def DrawAllFrames(self):
    global NumberOfFrames

    for frame in range(1, NumberOfFrames+1):
      self.DrawSingleFrame(frame, False)

  def DrawTrajectories(self):
    for cluster_id in range(FirstCluster, LastCluster+1):
      if (self.DisplayingClusters[cluster_id] == True):
        for arrow in self.Arrows[cluster_id]:
          self.AxesL.add_patch(arrow) 
#    self.Canvas.draw()


  def PlotAll(self):
    self.PlotL()
    self.PlotR()

  def PlotL(self):
    self.AxesL.clear()

    if (self.CurrentView == VIEW_FRAMES):
      self.DrawSingleFrame(self.FocusOnFrame)
    elif (self.CurrentView == VIEW_TRAJECTORY):
      self.AxesL.set_title('Clusters trajectories')
      self.DrawTrajectories()
      self.DrawAllFrames()

    self.Fig.tight_layout()
    self.DrawL()

  def PlotR(self):
    self.CleanR()
    self.AxesR1 = self.FigR.add_subplot(221)
    self.AxesR2 = self.FigR.add_subplot(222)
    self.AxesR3 = self.FigR.add_subplot(223)
    self.AxesR4 = self.FigR.add_subplot(224)

    self.PlotCorrelation()
    self.PlotDispersion()
    self.PlotHeatMap()
    self.PlotCallers()
    self.FigR.tight_layout()

    self.DrawR()
 
  def PlotCorrelation(self):
    global metrics_list

    for metric in metrics_list:
      if (self.DisplayingMetrics[metric] == True):
        metric_min = -1
        metric_max = -1
        metric_y   = [ ]
        for frame in range(1, NumberOfFrames+1):
          frame_min = np.min( Data[frame][metric][Data[frame]['cluster_id'] == self.FocusOnCluster] )
          if ((frame_min < metric_min) or (metric_min == -1)):
            metric_min = frame_min
    
          frame_max = np.max( Data[frame][metric][Data[frame]['cluster_id'] == self.FocusOnCluster] )
          if ((frame_max > metric_max) or (metric_max == -1)):
            metric_max = frame_max

        for frame in range(1, NumberOfFrames+1):
          frame_avg = np.average(Data[frame][metric][Data[frame]['cluster_id'] == self.FocusOnCluster] )
          normalized_frame_avg = (frame_avg - metric_min) / (metric_max - metric_min)
          metric_y.append( normalized_frame_avg )

        if (metric.lower() == self.FocusOnMetric.lower()):
          line_size = 3
        else:
          line_size = 1
        self.AxesR2.plot (range(1, NumberOfFrames+1), metric_y, label=metric, lw=line_size, color=MetricColor(metric) )

    self.AxesR2.set_title('Metrics correlation for Region '+str(ClusterOffset(self.FocusOnCluster)))
    self.AxesR2.set_xticks(range(1, NumberOfFrames+1))
    if (self.ToolbarR.GetToolState(self.ToolbarR.ON_LEGEND_CHECK) == True):
      self.AxesR2.legend(map(lambda x:x.upper(), metrics_list), loc=(0,0), prop={'size':7})

    
  def PlotHeatMap(self):
    heatm_precision = self.Bins
    heatm_num_ticks = 5

    heatm_x = Data[self.FocusOnFrame][DIM_X][Data[self.FocusOnFrame]['cluster_id'] == self.FocusOnCluster]
    heatm_y = Data[self.FocusOnFrame][DIM_Y][Data[self.FocusOnFrame]['cluster_id'] == self.FocusOnCluster]

    self.AxesR1.set_title('Support for Region '+str(ClusterOffset(self.FocusOnCluster)))

    heatmap, heatm_yedges, heatm_xedges = np.histogram2d(heatm_y, heatm_x, bins=heatm_precision)

    if (self.ToolbarR.GetToolState(self.ToolbarR.ON_INTERPOLATE_CHECK) == True):
      cax = self.AxesR1.imshow(heatmap, origin='lower', aspect='auto', extent=(heatm_xedges.min(), heatm_xedges.max(), heatm_yedges.min(), heatm_yedges.max()))
    else:
      cax = self.AxesR1.imshow(heatmap, interpolation='nearest', origin='lower', aspect='auto', extent=(heatm_xedges.min(), heatm_xedges.max(), heatm_yedges.min(), heatm_yedges.max()))

    self.ColorBar = self.FigR.colorbar(cax, ax=self.AxesR1, orientation='vertical', use_gridspec=True, shrink=0.8)
    

  def PlotDispersion(self):
    metric_dispersion = [ ]
    metric_average    = [ ]
    metric_min        = [ ]
    metric_max        = [ ]
    for frame in range(1, NumberOfFrames+1):
      frame_dispersion = Data[frame][self.FocusOnMetric][Data[frame]['cluster_id'] == self.FocusOnCluster]
      metric_dispersion.append( frame_dispersion )
      metric_average.append( np.average(Data[frame][self.FocusOnMetric][Data[frame]['cluster_id'] == self.FocusOnCluster]) )
      metric_min.append( np.min(Data[frame][self.FocusOnMetric][Data[frame]['cluster_id'] == self.FocusOnCluster]) )
      metric_max.append( np.max(Data[frame][self.FocusOnMetric][Data[frame]['cluster_id'] == self.FocusOnCluster]) )
  
    self.AxesR4.set_title(self.FocusOnMetric.upper()+' for Region '+str(ClusterOffset(self.FocusOnCluster)))
    if (self.ToolbarR.GetToolState(self.ToolbarR.ON_BOXPLOT_CHECK) == True):
      self.AxesR4.boxplot(metric_dispersion)
      self.AxesR4.plot( range(1, NumberOfFrames+1), metric_average, lw=2, color=MetricColor(self.FocusOnMetric) )
    else:
      x = range(1, NumberOfFrames+1)
      self.AxesR4.plot(x, metric_min, ls='--', color=MetricColor(self.FocusOnMetric))
      self.AxesR4.plot(x, metric_average, marker='o', ms=2, color=MetricColor(self.FocusOnMetric))
      self.AxesR4.plot(x, metric_max, ls='--', color=MetricColor(self.FocusOnMetric))
      self.AxesR4.fill_between(x, metric_min, metric_max, alpha=0.25, color=MetricColor(self.FocusOnMetric) )
      self.AxesR4.set_xticks(range(1, NumberOfFrames+1))
      self.AxesR4.set_xlim(1, NumberOfFrames)
      self.AxesR4.set_xbound(1, NumberOfFrames)

    if (self.GetFittingSamples() > 0):
      fit_x, fit_y = self.Predict(metric_average)
      self.AxesR4.plot(fit_x, fit_y, ls=':', color='black', marker='o', ms=4)
      self.AxesR4.set_xticks(range(1, len(fit_x)+1))
      self.AxesR4.set_xlim(1, len(fit_x))
      self.AxesR4.set_xbound(1, len(fit_x))

  def Predict(self, y):
    x = range(1, len(y)+1)
   
    fit = np.polyfit(x, y, self.GetFittingDegree())
    fit_fn = np.poly1d(fit) # fit_fn is now a function which takes in x and returns an estimate for y

    fit_x = range(1, self.GetFittingSamples() + 1)
    fit_y = [ ]
    for x in fit_x:
      fit_y.append( fit_fn(x) )

    return fit_x, fit_y

  def GetFittingSamples(self):
    return int(self.PredictExperiments.GetValue())

  def GetFittingDegree(self):
    return self.RegressionTypeCombo.GetSelection() + 1

  def PlotCallers(self):
    callers_labels = Callers['caller'][Callers['cluster_id'] == self.FocusOnCluster]
    callers_fracts = Callers['pct'][Callers['cluster_id'] == self.FocusOnCluster]
    self.AxesR3.set_title('Callers distribution for Region '+str(ClusterOffset(self.FocusOnCluster)))
    self.AxesR3.pie(callers_fracts, autopct='%1.1f%%', shadow=True) #labels=callers_labels
    self.AxesR3.set_aspect('equal')
    if (self.ToolbarR.GetToolState(self.ToolbarR.ON_LEGEND_CHECK) == True):
      self.AxesR3.legend( callers_labels, loc=(0,0), prop={'size':7} )

  def on_Frame_Change(self, event):
    self.FocusOnFrame = self.FrameSlider.GetValue()
    self.PlotL()
    self.PlotR()

  def on_Resolution_Change(self, event):
    self.Bins = self.ResolutionSlider.GetValue()
    self.PlotR()

  def SetupView(self):
    if (self.CurrentView == VIEW_FRAMES):
      self.ViewButton.SetLabel("Switch to "+VIEW_TRAJECTORY+" view")
      self.PrevButton.Enable()
      self.NextButton.Enable()
      self.FrameSlider.Enable()
      self.ResolutionSlider.Enable()
      self.PredictExperiments.Enable()
      self.RegressionTypeCombo.Enable()
      self.Toolbar.ToggleTool(self.Toolbar.ON_SCATTER_CHECK,  True)
      self.Toolbar.ToggleTool(self.Toolbar.ON_HULL_CHECK,     False)
      self.Toolbar.ToggleTool(self.Toolbar.ON_CENTROID_CHECK, False)
    elif (self.CurrentView == VIEW_TRAJECTORY):
      self.ViewButton.SetLabel("Switch to "+VIEW_FRAMES+" view")
      self.PrevButton.Disable()
      self.NextButton.Disable()
      self.FrameSlider.Disable()
      self.ResolutionSlider.Disable()
      self.PredictExperiments.Disable()
      self.RegressionTypeCombo.Disable()
      self.Toolbar.ToggleTool(self.Toolbar.ON_SCATTER_CHECK,  False)
      self.Toolbar.ToggleTool(self.Toolbar.ON_HULL_CHECK,     False)
      self.Toolbar.ToggleTool(self.Toolbar.ON_CENTROID_CHECK, True)
    self.ViewButton.Enable()
    self.PlayButton.Enable()
    self.ClustersScroll.Enable()
    self.MetricsScroll.Enable()

  def DisableControls(self):
    self.ViewButton.Disable()
    self.PrevButton.Disable()
    self.NextButton.Disable()
    self.PlayButton.Disable()
    self.FrameSlider.Disable()
    self.ClustersScroll.Disable()
    self.MetricsScroll.Disable()

  def on_View_Button(self, event):
    if (self.ViewButton.GetValue() == True):
      self.CurrentView = VIEW_TRAJECTORY
    else:
      self.CurrentView = VIEW_FRAMES
    self.SetupView()
    self.PlotL()

  def on_Playing_Timer(self, event):
    global NumberOfFrames

    self.FocusOnFrame = self.PlayingCurrentFrame
    self.FrameSlider.SetValue(self.PlayingCurrentFrame)
    self.PlotL()
    self.PlotR()
    self.PlayingCurrentFrame = self.PlayingCurrentFrame + 1
    if (self.PlayingCurrentFrame > NumberOfFrames):
      self.PlayTimer.Stop()
      self.SetupView()

  def on_Play_Button(self, event):
    self.DisableControls()
    if (self.CurrentView == VIEW_FRAMES):
      self.PlayingCurrentFrame = 1
      self.PlayTimer.Start(1000)
    elif (self.CurrentView == VIEW_TRAJECTORY):
      animation.TimedAnimation.__init__(self, self.Fig, interval=self.AnimationPeriod, repeat=False, blit=True)

  def on_Prev_Button(self, event):
    slider_old_value = self.FrameSlider.GetValue()
    slider_new_value = slider_old_value - 1
    if (slider_new_value >= 1):
      self.FrameSlider.SetValue(slider_new_value)
      self.FocusOnFrame = slider_new_value
      self.PlotL()
      self.PlotR()

  def on_Next_Button(self, event):
    global NumberOfFrames

    slider_old_value = self.FrameSlider.GetValue()
    slider_new_value = slider_old_value + 1
    if (slider_new_value <= NumberOfFrames):
      self.FrameSlider.SetValue(slider_new_value)
      self.FocusOnFrame = slider_new_value
      
      self.PlotL()
      self.PlotR()

  def on_Metrics_Display(self, event):
    btn = event.GetEventObject()
    metric = btn.GetName()
    if (self.DisplayingMetrics[ metric ] == True):
      self.DisplayingMetrics[ metric ] = False
      bmp = self.bmp_nodisplay
    else:
      self.DisplayingMetrics[ metric ] = True
      bmp = self.bmp_display

    for controls in self.MetricsScrollList:
      if (controls[0].GetName() == metric):
        controls[0].SetBitmapLabel(bmp)
        break
    self.PlotR()

  def on_Metrics_Focus(self, event):

    btn = event.GetEventObject()
    self.set_Metric_Focus( btn.GetName() )
    self.PlotR()

  def set_Metric_Focus(self, metric):
    for controls in self.MetricsScrollList:
      if (controls[1].GetName() == metric):
        controls[1].SetValue(1)
        controls[1].SetBackgroundColour(wx.Colour(selected_color[0]*255, selected_color[1]*255, selected_color[2]*255, 0))
        self.FocusOnMetric = metric
      else:
        controls[1].SetValue(0)
        controls[1].SetBackgroundColour(wx.Colour(non_selected_color[0]*255, non_selected_color[1]*255, non_selected_color[2]*255, 0))

  def on_Clusters_Display(self, event):
    btn = event.GetEventObject()
    cluster_id = int(btn.GetName())
    if (self.DisplayingClusters[ cluster_id ] == True):
      self.DisplayingClusters[ cluster_id ] = False
      bmp = self.bmp_nodisplay
    else:
      self.DisplayingClusters[ cluster_id ] = True
      bmp = self.bmp_display

    for controls in self.ClustersScrollList:
      if (int(controls[0].GetName()) == cluster_id):
        controls[0].SetBitmapLabel(bmp)
        break
    self.PlotL()

  def on_Clusters_Focus(self, event):
    btn = event.GetEventObject()
    self.set_Cluster_Focus(btn.GetName())
    self.PlotR()

  def set_Cluster_Focus(self, cluster_id):
    for controls in self.ClustersScrollList:
      if (controls[1].GetName() == str(cluster_id)):
        controls[1].SetValue(1)
        controls[1].SetBackgroundColour(wx.Colour(selected_color[0]*255, selected_color[1]*255, selected_color[2]*255, 0))
        self.FocusOnCluster = int(cluster_id)
      else:
        controls[1].SetValue(0)
        controls[1].SetBackgroundColour(wx.Colour(non_selected_color[0]*255, non_selected_color[1]*255, non_selected_color[2]*255, 0))

  def CleanL(self):
    self.Fig.clf()

  def CleanR(self):
    self.FigR.clf()

  def CleanAll(self):
    self.CleanL()
    self.CleanR()

  def DrawL(self):
    self.Canvas.draw()
  
  def DrawR(self):
    self.CanvasR.draw()
  
  def DrawAll(self):
    self.DrawL()
    self.DrawR()

  def on_Prediction_Change(self, event):
    self.PlotR()

  def on_RegressionType_Change(self, event):
    if (self.GetFittingSamples() > 0):
      self.PlotR()

  def _draw_frame(self, current_frame):
    i = current_frame
    head = i - 1
    head_len = 0.5

    for cluster_id in self.Paths:
      if (self.DisplayingClusters[cluster_id] == True):
        x = self.Xi[cluster_id]
        y = self.Yi[cluster_id]

        Path = self.Paths[cluster_id][0]
        Tail = self.Paths[cluster_id][1]
        Head = self.Paths[cluster_id][2]

        # Status of this frame for each object
        head_slice = (self.T > self.T[i] - head_len) & (self.T < self.T[i])
        Path.set_data(x[:i], y[:i])
        Tail.set_data(x[head_slice], y[head_slice])
        Head.set_data(x[head], y[head])

        # Display the trajectory animation
        self._drawn_artists.append(Path)
        self._drawn_artists.append(Tail)
        self._drawn_artists.append(Head)
    
    if (current_frame+1 == self.AnimationSamples):
      self.SetupView()

  def new_frame_seq(self):
    return iter(range(self.AnimationSamples))

  def _init_draw(self):
    for cluster_id in self.Paths:
      lines =  [self.Paths[cluster_id][0], self.Paths[cluster_id][1], self.Paths[cluster_id][2]]
      for l in lines:
          l.set_data([], [])


def Usage():
  print sys.argv[0], "<csv-1> <csv-2> ... <csv-N>"

#
# Parse arguments
#
argc = len(sys.argv)
NumberOfFrames = argc - 1
if NumberOfFrames < 2:
  Usage()
  sys.exit()

app = wx.PySimpleApp(False)
app.main = MainFrame(None, "Tracking")
app.main.Show()
app.MainLoop()
