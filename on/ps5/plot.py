import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas
import math
import sys
import numpy

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

latex_columnwidth = 426.79135

def entry(args):
  # Enable LaTeX
  plt.rcParams['text.usetex'] = True
  plt.rcParams['font.family'] = "Computer Modern"
  plt.rcParams['font.size'] = 10

  series = ["Gauss", "Gauss select", "LU", "LU select"]
  csv = pandas.read_csv(f"runtime_trimmed.csv", header = None,
    names = ["size", *series, "memory"])
  csv.sort_values(by = "size", axis = "index", ignore_index = True, inplace = True)

  figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
  axes = figure.add_subplot()
  axes.set_xticks(csv["size"], labels = csv["size"], rotation = 25)
  axes.set_ylabel("Time [seconds]")
  axes.set_xlabel("n (input size)")
  for s in series:
    axes.plot(csv["size"], csv[s])
  axes.legend(series)

  figure.tight_layout()
  figure.savefig(f"runtime.png")

  del figure

  figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
  axes = figure.add_subplot()
  axes.set_xticks(csv["size"], labels = csv["size"], rotation = 25)
  axes.set_ylabel("Memory [MiB]")
  axes.set_xlabel("n (input size)")
  baseline = csv["memory"][0]
  axes.plot(csv["size"], csv["memory"].apply(lambda x: (x - baseline + 2) / 1024))
  figure.tight_layout()
  figure.savefig(f"memory.png")

entry(sys.argv[1:])
