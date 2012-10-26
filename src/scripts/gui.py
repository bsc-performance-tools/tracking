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
NumberOfFrames = 0
X_min_global = Y_min_global = X_max_global = Y_max_global = -1

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


class MyNavigationToolbar(NavigationToolbar):
  #
  # Extend the default wx toolbar with your own event handlers
  #  
  ON_SCATTER_CHECK = wx.NewId()
  ON_HULL_CHECK = wx.NewId()
  ON_CENTROID_CHECK = wx.NewId()
  def __init__(self, canvas):
    NavigationToolbar.__init__(self, canvas)

    self.AddSeparator()

    self.AddCheckTool(
      self.ON_SCATTER_CHECK,
      wx.Bitmap('icon_scatter.png'),
      wx.Bitmap('icon_scatter.png'),
      'Draw the points of the cluster',
      'Draw the points of the cluster')

    self.AddCheckTool(
      self.ON_HULL_CHECK,
      wx.Bitmap('icon_hull.png'),
      wx.Bitmap('icon_hull.png'),
      'Draw the perimeter of the cluster',
      'Draw the perimeter of the cluster')

    self.AddCheckTool(
      self.ON_CENTROID_CHECK,
      wx.Bitmap('icon_centroid.png'),
      wx.Bitmap('icon_centroid.png'),
      'Draw the centroid of the cluster',
      'Draw the centroid of the cluster')

    self.ToggleTool(self.ON_SCATTER_CHECK, True)

    wx.EVT_TOOL(self, self.ON_SCATTER_CHECK, self.on_Scatter_Check)
    wx.EVT_TOOL(self, self.ON_HULL_CHECK, self.on_Hull_Check)
    wx.EVT_TOOL(self, self.ON_CENTROID_CHECK, self.on_Centroid_Check)
   
  def on_Scatter_Check(self, event):
      app.main.Plot()

  def on_Hull_Check(self, event):
      app.main.Plot()

  def on_Centroid_Check(self, event):
      app.main.Plot()


class MainFrame(wx.Frame, animation.TimedAnimation):
  #
  # The main frame of the application
  #

  def __init__(self, parent, title):
    #
    # A "-1" in the size parameter instructs wxWidgets to use the default size.
    #
    wx.Frame.__init__(self, parent, title=title)

    self.CurrentFrame = 1
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
    self.Displaying = defaultdict()

    self.LoadData()
    self.Create_Main_Panel()
    self.SetupView()

    self.ComputePlots()

    self.Plot()

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
 
    #
    # Since we have only one plot, we can use add_axes instead 
    # of add_subplot, but then the subplot configuration tool 
    # in the navigation toolbar wouldn't work.
    #
    self.Axes = self.Fig.add_subplot(111)

    #
    # Create the navigation toolbar, tied to the canvas
    #
    self.Toolbar = MyNavigationToolbar(self.Canvas)

    #
    # Create the buttons
    #
    self.ViewButton = wx.ToggleButton(self.Panel, -1, "Switch to "+VIEW_TRAJECTORY+" view")
    self.Bind(wx.EVT_TOGGLEBUTTON, self.on_View_Button, self.ViewButton)
    self.PrevButton = wx.Button(self.Panel, -1, "\n<<\n")
    self.Bind(wx.EVT_BUTTON, self.on_Prev_Button, self.PrevButton)
    self.NextButton = wx.Button(self.Panel, -1, "\n>>\n")
    self.Bind(wx.EVT_BUTTON, self.on_Next_Button, self.NextButton)
    self.PlayButton = wx.Button(self.Panel, -1, "\nPlay\n")
    self.Bind(wx.EVT_BUTTON, self.on_Play_Button, self.PlayButton)
    self.Slider = wx.Slider(self.Panel, -1,
      value=self.CurrentFrame, 
      minValue=1,
      maxValue=NumberOfFrames,
      style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS | wx.SL_LABELS)
    self.Slider.SetTickFreq(1, 1)
    self.Bind(wx.EVT_COMMAND_SCROLL_CHANGED, self.on_Slider_Change, self.Slider)

    #
    # Create timers
    #
    self.PlayTimer = wx.Timer(self)
    self.Bind(wx.EVT_TIMER, self.on_Playing_Timer, self.PlayTimer)

    #
    # Create legend
    #
    self.LegendScroll = wx.ScrolledWindow(self.Panel, -1, style=wx.VSCROLL | wx.HSCROLL | wx.ALWAYS_SHOW_SB)
    self.LegendLine = []
    for cluster_id in range(FirstCluster, LastCluster+1):
      LegendLineTxt = wx.TextCtrl(self.LegendScroll, -1, "Cluster "+str(cluster_id-FirstCluster+1), size=(100,-1), style=wx.TE_READONLY)
      LegendLineBtn = wx.ToggleButton(self.LegendScroll, -1, size=(25,25), name=str(cluster_id), style=wx.RAISED_BORDER)
      self.Bind(wx.EVT_TOGGLEBUTTON, self.on_Legend_Button, LegendLineBtn)
      c = PRVColor(cluster_id)
      LegendLineBtn.SetBackgroundColour(wx.Colour(c[0]*255, c[1]*255, c[2]*255))
      self.LegendLine.append((LegendLineBtn, LegendLineTxt))

#    LegendLineSizer = wx.FlexGridSizer(rows=len(self.LegendLine)+1, cols=2, vgap=3, hgap=3)
#    for ctrls in self.LegendLine:
#      LegendLineSizer.AddMany([(ctrls[0], 0, wx.ALIGN_LEFT),
#                               (ctrls[1], 0, wx.ALIGN_LEFT)])
#    LegendLineSizer.AddGrowableCol(0)
    
    LegendLineSizer = wx.BoxSizer(wx.VERTICAL)
    for ctrls in self.LegendLine:
      LegendLineSizerH = wx.BoxSizer(wx.HORIZONTAL)
      LegendLineSizerH.Add(ctrls[0], 0, wx.ALIGN_LEFT)
      LegendLineSizerH.Add(ctrls[1], 0, wx.ALIGN_LEFT)
      LegendLineSizer.Add(LegendLineSizerH)

    self.LegendBox = wx.StaticBox(self.Panel, -1, "Display clusters", style=wx.EXPAND)
    self.LegendBoxSizer = wx.StaticBoxSizer(self.LegendBox, wx.VERTICAL)
    
    self.LegendScroll.SetSizer(LegendLineSizer)
    self.LegendScroll.SetScrollbars(1, 1, 1, 1)

#    self.LegendBoxSizer.Add(self.LegendScroll, 1, wx.EXPAND)
    self.LegendBoxSizer.Add( self.LegendScroll, 1, wx.EXPAND )

#    self.Legend = wx.BoxSizer(wx.VERTICAL)
#    self.Legend.Add(self.LegendBoxSizer, 1, wx.EXPAND | wx.GROW)



    #
    # Layout with box sizers
    #
    self.PlotBox = wx.BoxSizer(wx.VERTICAL)
    self.PlotBox.Add(self.Canvas, 1, wx.LEFT | wx.TOP | wx.GROW)
    self.PlotBox.Add(self.Toolbar, 0)
    self.PlotBox.AddSpacer(10)
        
    self.PlotControlsBox1 = wx.BoxSizer(wx.HORIZONTAL)
    self.PlotControlsBox1.Add(self.ViewButton, 1, wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL)

    self.PlotControlsBox2 = wx.BoxSizer(wx.HORIZONTAL)
    self.PlotControlsBox2.Add(self.PlayButton, 0, wx.ALIGN_LEFT | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.PlotControlsBox2.Add(self.PrevButton, 0, wx.ALIGN_LEFT | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.PlotControlsBox2.Add(self.Slider, 1, wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL)
    self.PlotControlsBox2.Add(self.NextButton, 0, wx.ALIGN_RIGHT | wx.ALL | wx.ALIGN_CENTER_VERTICAL)

    self.PlotBox.Add(self.PlotControlsBox1, 0, flag = wx.TOP | wx.EXPAND)
    self.PlotBox.Add(self.PlotControlsBox2, 0, flag = wx.TOP | wx.EXPAND)

    self.FrameBox = wx.BoxSizer(wx.VERTICAL)
    self.FrameBox.Add(self.LegendBoxSizer, 1, wx.EXPAND)

    self.WindowBox = wx.BoxSizer(wx.HORIZONTAL)
    self.WindowBox.Add(self.PlotBox, 3, wx.EXPAND)
    self.WindowBox.Add(self.FrameBox, 1, wx.EXPAND)

    self.Panel.SetSizer(self.WindowBox)
    self.WindowBox.Fit(self)

    self.SetMinSize(self.GetSize())

  def LoadData(self):
    global X_min_global
    global Y_min_global
    global X_max_global
    global Y_max_global
    global LastCluster

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

    for cluster_id in range(FirstCluster, LastCluster+1):
      self.Displaying[cluster_id] = True

  def ComputePlots(self):
    for frame in range(1, NumberOfFrames+1):
      for cluster_id in range(FirstCluster, LastCluster+1):
        #
        # Compute the scatter plot for this cluster
        #
        ClusterData = Data[frame][Data[frame][CLUSTER_ID] == cluster_id]
        cluster_scatter = self.Axes.scatter(ClusterData[DIM_X], ClusterData[DIM_Y], color=PRVColor(cluster_id), marker=".", zorder=2)

        #
        # Compute the convex hull for this cluster
        #
        cluster_hull = ch.convexHull( ClusterData[[ DIM_X, DIM_Y ]] )
        vertices = []
        for v in cluster_hull:
          vertices.append([v[0], v[1]])
        cluster_hull = Polygon(vertices, True, color=PRVColor(cluster_id), zorder=1)

        #
        # Compute the centroid for this cluster
        #
        centroid_x = np.average(ClusterData[DIM_X])
        centroid_y = np.average(ClusterData[DIM_Y])
        cluster_centroid = self.Axes.scatter(centroid_x, centroid_y, s=50, color=PRVColor(cluster_id), edgecolor="black", marker="o", zorder=3)
        
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

        arrow = FancyArrowPatch((from_x,from_y), (to_x,to_y), arrowstyle='->', mutation_scale=30, color=PRVColor(cluster_id), linewidth=2)
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
      fx = interp1d(Ts, Xs, kind="cubic")
      fy = interp1d(Ts, Ys, kind="cubic")

      # Generate the interpolated array
      self.Xi[cluster_id] = fx(self.T)
      self.Yi[cluster_id] = fy(self.T)
       
      # Artists for the interpolated path
      Path = Line2D([], [], color='black')
      Tail = Line2D([], [], color='red', linewidth=3)
      Head = Line2D([], [], color='red', marker='o', markeredgecolor='black')
      self.Paths[cluster_id] = (Path, Tail, Head)
      self.Axes.add_line(Path)
      self.Axes.add_line(Tail)
      self.Axes.add_line(Head)

  def DrawSingleFrame(self, frame, clear=True):
     if (clear == True):
       self.Axes.clear()

     self.Axes.set_xlim(X_min_global, X_max_global)
     self.Axes.set_ylim(Y_min_global, Y_max_global)
#     self.Axes.set_xscale('log')
#     self.Axes.set_yscale('log')

     if (self.Toolbar.GetToolState(self.Toolbar.ON_CENTROID_CHECK) == True):
       for cluster_id, f in self.Centroids:
         if (self.Displaying[cluster_id] == True) and (frame == f):
           centroid = self.Centroids[(cluster_id,frame)]
           self.Axes.add_artist(centroid)

     if (self.Toolbar.GetToolState(self.Toolbar.ON_SCATTER_CHECK) == True):
       for cluster_id, f in self.Scatters:
         if (self.Displaying[cluster_id] == True) and (frame == f):
           scatter = self.Scatters[(cluster_id,frame)]
           self.Axes.add_artist(scatter)

     if (self.Toolbar.GetToolState(self.Toolbar.ON_HULL_CHECK) == True):
       for cluster_id, f in self.Hulls:
         if (self.Displaying[cluster_id] == True) and (frame == f):
           hull = self.Hulls[(cluster_id,frame)]
           self.Axes.add_artist(hull)

     self.Axes.set_xlabel("IPC")
     self.Axes.set_ylabel("Instructions")
     self.Canvas.draw()

  
  def DrawAllFrames(self):
    global NumberOfFrames

    for frame in range(1, NumberOfFrames+1):
      self.DrawSingleFrame(frame, False)

  def DrawTrajectories(self):
    for cluster_id in range(FirstCluster, LastCluster+1):
      if (self.Displaying[cluster_id] == True):
        for arrow in self.Arrows[cluster_id]:
          self.Axes.add_patch(arrow) 
    self.Canvas.draw()

  def Plot(self):
    self.Axes.clear()

    if (self.CurrentView == VIEW_FRAMES):
      self.DrawSingleFrame(self.CurrentFrame)
    elif (self.CurrentView == VIEW_TRAJECTORY):
      self.DrawAllFrames()
      self.DrawTrajectories()

  def on_Slider_Change(self, event):
    self.CurrentFrame = self.Slider.GetValue()
    self.Plot()
    

  def SetupView(self):
    if (self.CurrentView == VIEW_FRAMES):
      self.ViewButton.SetLabel("Switch to "+VIEW_TRAJECTORY+" view")
      self.PrevButton.Enable()
      self.NextButton.Enable()
      self.Slider.Enable()
    elif (self.CurrentView == VIEW_TRAJECTORY):
      self.ViewButton.SetLabel("Switch to "+VIEW_FRAMES+" view")
      self.PrevButton.Disable()
      self.NextButton.Disable()
      self.Slider.Disable()
    self.ViewButton.Enable()
    self.PlayButton.Enable()
    self.LegendScroll.Enable()

  def DisableControls(self):
    self.ViewButton.Disable()
    self.PrevButton.Disable()
    self.NextButton.Disable()
    self.PlayButton.Disable()
    self.Slider.Disable()
    self.LegendScroll.Disable()

  def on_View_Button(self, event):
    if (self.ViewButton.GetValue() == True):
      self.CurrentView = VIEW_TRAJECTORY
    else:
      self.CurrentView = VIEW_FRAMES
    self.SetupView()
    self.Plot()

  def on_Playing_Timer(self, event):
    global NumberOfFrames

    self.Slider.SetValue(self.PlayingCurrentFrame)
    self.DrawSingleFrame(self.PlayingCurrentFrame)
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
    slider_old_value = self.Slider.GetValue()
    slider_new_value = slider_old_value - 1
    if (slider_new_value >= 1):
      self.Slider.SetValue(slider_new_value)
      self.CurrentFrame = slider_new_value
      self.Plot()

  def on_Next_Button(self, event):
    global NumberOfFrames

    slider_old_value = self.Slider.GetValue()
    slider_new_value = slider_old_value + 1
    if (slider_new_value <= NumberOfFrames):
      self.Slider.SetValue(slider_new_value)
      self.CurrentFrame = slider_new_value
      self.Plot()

  def on_Legend_Button(self, event):
    btn = event.GetEventObject()
    cluster_id = int(btn.GetName())
    idx = cluster_id - FirstCluster
    txt = self.LegendLine[idx][1]
    c = PRVColor(cluster_id)
    if (btn.GetValue() == True):
      self.Displaying[cluster_id] = False
      btn.SetBackgroundColour(wx.Colour(c[0]*255, c[1]*255, c[2]*255, alpha=0.1))
      txt.Disable()
    else:
      self.Displaying[cluster_id] = True
      btn.SetBackgroundColour(wx.Colour(c[0]*255, c[1]*255, c[2]*255, 0))
      txt.Enable()
    self.Plot()
    

  def _draw_frame(self, current_frame):
    i = current_frame
    head = i - 1
    head_len = 0.5

    for cluster_id in self.Paths:
      if (self.Displaying[cluster_id] == True):
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
