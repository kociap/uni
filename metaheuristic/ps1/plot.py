import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas
import math
import sys
import numpy
import networkx as nx

def get_figsize(columnwidth, wf = 1.0, hf = (5.0**0.5 - 1.0) / 2.0):
  """Parameters:
  - wf [float]:  width fraction in columnwidth units
  - hf [float]:  height fraction in columnwidth units.
                  Set by default to golden ratio.
  - columnwidth [float]: width of the column in latex. Get this from LaTeX
                          using \showthe\columnwidth
  Returns:  [fig_width,fig_height]: that should be given to matplotlib
  """
  fig_width_pt = columnwidth * wf
  inches_per_pt = 1.0 / 72.0                # Convert pt to inch
  fig_width = fig_width_pt * inches_per_pt  # Width in inches
  fig_height = fig_width * hf               # Height in inches
  return [fig_width, fig_height]

latex_columnwidth = 443.57848

def create_plot(d1, d2, legend):
  figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
  axes = figure.add_subplot(111)
  axes.scatter(d1.index, d1, c="C1", s=2**2)
  axes.plot(d2.index, d2, c="C2", linewidth=2)
  axes.legend(legend)
  return figure, axes

def create_plot_from_many(legend, *datas):
  figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
  axes = figure.add_subplot(111)
  for index, data in enumerate(datas):
    axes.plot(data.index, data, c=f"C{index + 1}", linewidth=2)
  axes.legend(legend)
  return figure, axes

def entry(args):
  # Enable LaTeX
  plt.rcParams['text.usetex'] = True
  plt.rcParams['font.size'] = 10

  with open(args[0], "r") as file:
    lines = file.read().splitlines()

  G = nx.Graph()

  for line in lines:
    parts = line.split()
    src_x = int(parts[0])
    src_y = int(parts[1])
    dst_x = int(parts[2])
    dst_y = int(parts[3])
    G.add_node(f"{src_x}_{src_y}", pos=[src_x, src_y])
    G.add_node(f"{dst_x}_{dst_y}", pos=[dst_x, dst_y])
    weight = round(math.sqrt((src_x - dst_x)**2 + (src_y - dst_y)**2))
    G.add_edge(f"{src_x}_{src_y}", f"{dst_x}_{dst_y}", weight = weight)

  figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
  axes = figure.add_axes((0, 0, 1, 1))
  pos = nx.get_node_attributes(G, 'pos')
  nx.draw(G, pos = pos, ax = axes, node_size=8, node_color='r', with_labels=False)
  figure.savefig(args[1])

entry(sys.argv[1:])
