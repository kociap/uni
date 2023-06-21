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

latex_columnwidth = 390.0

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

def plot_edmonds(filename):
    data = pandas.read_csv(filename, header = None, names = ["size", "flow", "time", "paths"])
    data = data.groupby("size").mean()

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    axes.plot(data.index, numpy.log2(data["time"] / 1000.0))
    axes.set_ylabel("time (log$_{2}$ ms)")
    axes.set_xlabel("k")
    figure.savefig("edmonds_time.png", bbox_inches = "tight")

    del figure

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    axes.plot(data.index, numpy.log2(data["flow"]))
    axes.set_ylabel("maximum flow (log$_{2}$ units)")
    axes.set_xlabel("k")
    figure.savefig("edmonds_flow.png", bbox_inches = "tight")

    del figure

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    axes.plot(data.index, numpy.log2(data["paths"]))
    axes.set_ylabel("augmenting paths (log$_{2}$ units)")
    axes.set_xlabel("k")
    figure.savefig("edmonds_paths.png", bbox_inches = "tight")

    del figure

def plot_bipartite(filename):
    figure_time = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes_time = figure_time.add_subplot(111)
    figure_matchings = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes_matchings = figure_matchings.add_subplot(111)
    data = pandas.read_csv(filename, header = None, names = ["size", "degree", "matchings", "time"])
    transformed = pandas.DataFrame()
    for name, g in data.groupby("size"):
        g = g.groupby("degree").mean()
        # axes_time.plot(g.index, numpy.log10(g["time"]), label = f"size {name}")
        # axes_matchings.plot(g.index, numpy.log10(g["matchings"]), label = f"size {name}")
        axes_time.plot(g.index, g["time"], label = f"size {name}")
        axes_matchings.plot(g.index, g["matchings"], label = f"size {name}")
    axes_time.legend()
    axes_time.set_xlabel("degree")
    axes_time.set_ylabel("time (µs)")
    figure_time.savefig("bipartite_time.png", bbox_inches = "tight")
    axes_matchings.legend()
    axes_matchings.set_xlabel("degree")
    axes_matchings.set_ylabel("matchings (units)")
    figure_matchings.savefig("bipartite_matchings.png", bbox_inches = "tight")

def entry(args):
    # Enable LaTeX
    plt.rcParams['text.usetex'] = True
    plt.rcParams['font.size'] = 10

    plot_edmonds("edmonds_result.txt")
    plot_bipartite("bipartite_result.txt")

entry(sys.argv[1:])
