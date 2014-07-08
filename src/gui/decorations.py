# -*- coding: utf-8 -*-

import math

ARROW_LEFT  = "◄"
ARROW_RIGHT = "►"
ARROW_UP    = "▲"
ARROW_DOWN  = "▼"

#
# PRV color palette
#
ColorPalette = [ ( 117, 195, 255 ), (   0,   0, 255 ), ( 255, 255, 255 ), ( 255,   0,   0 ), ( 255,   0, 174 ), ( 179,   0,   0 ), ( 0,   255,   0 ), ( 255, 255,   0 ), ( 235,   0,   0 ), (   0, 162,   0 ), ( 255,   0, 255 ), ( 100, 100,  177 ), ( 172, 174,  41 ), ( 255, 144,  26 ), (   2, 255, 177 ), ( 192, 224,   0 ), (  66,  66,  66 ), ( 189, 168, 100 ), (  95, 200,   0 ), ( 203,  60,  69 ), (   0, 109, 255 ), ( 200,  61,  68 ), ( 200,  66,   0 ), (   0,  41,   0 ), ( 139, 121, 177 ), ( 116, 116, 116 ), ( 200,  50,  89 ), ( 255, 171,  98 ), (   0,  68, 189 ), (  52,  43,   0 ), ( 255,  46,   0 ), ( 100, 216,  32 ), (   0,   0, 112 ), ( 105, 105,   0 ), ( 132,  75, 255 ), ( 184, 232,   0 ), (   0, 109, 112 ), ( 189, 168, 100 ), ( 132,  75,  75 ), ( 255,  75,  75 ), ( 255,  20,   0 ), (  52,   0,   0 ), (   0,  66,   0 ), ( 184, 132,   0 ), ( 100,  16,  32 ), ( 146, 255, 255 ), (   0,  23,  37 ), ( 146,   0, 255 ), (   0, 138, 119 ) ]

def RGBColor0_1(cluster_id):
    idx = cluster_id
    if (idx > len(ColorPalette)): idx = len(ColorPalette) - 1
    r = float(float(ColorPalette[idx][0]) / 255)
    g = float(float(ColorPalette[idx][1]) / 255)
    b = float(float(ColorPalette[idx][2]) / 255)
    return (r, g, b)

def RGBColor0_255(cluster_id):
    idx = cluster_id
    if (idx > len(ColorPalette)): idx = len(ColorPalette) - 1
    r = float(ColorPalette[idx][0])
    g = float(ColorPalette[idx][1])
    b = float(ColorPalette[idx][2])
    return (r, g, b)

def FontColor(background_rgb):
  r = background_rgb[0]
  g = background_rgb[1]
  b = background_rgb[2]
  
  if (math.sqrt (r * r * .299 + g * g * .587 + b * b * .114) > 130):
    return (0, 0, 0)
  else:
    return (255, 255, 255)

Markers = ['o', '+', '*', 's', 'd', '8', 'p', 'D', 'h', 'H', '4' ]
#, '_', '3', '0', '1', '2', '1', '3', '4', '2', 'v', '<', '>', '^', '|', 'x', ',', '.'] 

def ChooseMarker(id):
  return Markers[id % len(Markers)]

def MetricColor(id, range=1):
    if (range == 1):
      return RGBColor0_1( ( (id + 20) % len(ColorPalette) ) )
    else:
      return RGBColor0_255( ( (id + 20) % len(ColorPalette) ) )


