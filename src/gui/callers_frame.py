import wxversion
wxversion.ensureMinimal('2.8')
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import numpy as np
import wx

from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigCanvas

PieColors = ['yellowgreen', 'gold', 'lightskyblue', 'sienna', 'darkorchid', 'darkslateblue', 'orangered']

class Caller(object):
    def __init__(self, caller_file, caller_line, caller_pct):
        self.File = caller_file
        self.Line = caller_line
        self.Pct  = caller_pct

class CallersFrame( wx.Frame ):
  def __init__(self, parent, title, plot_data, plot_labels):
    wx.Frame.__init__(self, parent, title="Callers distribution for "+title)
    self.DrawFrame(plot_data, plot_labels)

  def GetPieColor(self, index):
    global PieColors
    if (index < len(PieColors)):
      return PieColors[index]
    else:
      return 'dimgray'

  def GetFontColor(self, index):
    colors=['black', 'black', 'black', 'white', 'white', 'white', 'black']
    if (index < len(colors)):
      return colors[index]
    else:
      return 'white'

  def DrawFrame(self, plot_data, plot_labels):
    global PieColors

    self.Panel  = wx.Panel(self)
    self.DPI    = 100
    self.Figure = Figure((4.0, 4.0), dpi=self.DPI)
    self.Canvas = FigCanvas(self.Panel, -1, self.Figure)
    self.Axes   = self.Figure.add_subplot(111)
    self.Figure.set_facecolor("#FFFFFF")

    self.CallersList = wx.ListCtrl(self.Panel, -1, style = wx.LC_REPORT | wx.EXPAND)
    self.CallersList.InsertColumn(0, "%", width=70)
    self.CallersList.InsertColumn(1, "File", width=250)
    self.CallersList.InsertColumn(2, "Line", width=80)
    self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.on_CallersList_Click, self.CallersList)


    self.SizerPlot = wx.BoxSizer(wx.HORIZONTAL)
    self.SizerPlot.Add( self.Canvas, 1, wx.EXPAND )
    self.SizerCallersList = wx.BoxSizer(wx.HORIZONTAL)
    self.SizerCallersList.Add( self.CallersList, 1, wx.EXPAND )

    print plot_data
    print "max: ",max(plot_data)
    index = np.searchsorted( plot_data, max(plot_data) )
    print "index: ",index
    explode = []
    for i in range(0, len(plot_data)):
      if (plot_data[i] == np.max(plot_data)):
        explode.append( 0.1 ) 
      else:
        explode.append(0)
    print index
    print explode

    self.Wedges, self.PlotLabels = self.Axes.pie( plot_data, labels=plot_labels, explode=explode, colors=PieColors, shadow=False, startangle=90 )
    wedge_id = 0
    for Wedge in self.Wedges:
      Wedge.set_picker(True)
      Wedge.set_edgecolor('white')
      Wedge.set_gid(wedge_id)
      wedge_id = wedge_id + 1 
      
    for Label in self.PlotLabels:
      Label.set_visible(False)

    self.Canvas.mpl_connect('pick_event', self.on_Pie_Click)

    data_array   = plot_data.tolist()
    labels_array = plot_labels.tolist()
    for i in range(0, len(data_array)):
      caller_file, caller_line = self.ParseLabel(labels_array[i])
 
      new_item = Caller(data_array[i], caller_file, caller_line)
      self.CallersList.Append((new_item.File, new_item.Line, new_item.Pct))
      self.CallersList.SetItemBackgroundColour(i, self.GetPieColor(i))
      self.CallersList.SetItemTextColour(i, self.GetFontColor(i))

    self.SizerWindow = wx.BoxSizer( wx.VERTICAL )
    self.SizerWindow.Add( self.SizerPlot, 2, wx.EXPAND )
    self.SizerWindow.Add( self.SizerCallersList, 1, wx.EXPAND )
    self.Panel.SetSizer( self.SizerWindow )
    self.SizerWindow.Fit(self)
    self.SetMinSize( self.GetSize() )

  def ParseLabel(self, label):
    caller_file = label[label.find("(")+1:label.find(")")]
    caller_line = label[0:label.find("(")]
    return caller_file, caller_line

  def on_Pie_Click(self, event):
    for Wedge in self.Wedges:
      Wedge.set_edgecolor('white')
      Wedge.set_zorder(0)

    Wedge = event.artist
    Label = Wedge.get_label()
    Wedge.set_edgecolor('red')
    Wedge.set_zorder(1)

    self.CallersList.Select( Wedge.get_gid(), True )
    self.Canvas.draw()

  def on_CallersList_Click(self, event):
    wedge_id = event.GetIndex()
    for Wedge in self.Wedges:
      if (Wedge.get_gid() == wedge_id):
        Wedge.set_edgecolor('red')
        Wedge.set_zorder(1)
      else:
        Wedge.set_edgecolor('white')
        Wedge.set_zorder(0)
    self.Canvas.draw()

