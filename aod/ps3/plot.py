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

    data = pandas.DataFrame()
    for line in sys.stdin:
        filename = line.rstrip()
        components = filename.split("/")
        algorithm = components[2]
        properties = components[4].split(".")
        test_set = properties[0]
        test_subset = properties[1]
        csv = pandas.read_csv(filename, sep = " ", header = None, names = ['t', algorithm])
        csv = csv.iloc[:, 1:]
        csv.insert(0, "subset", test_subset)
        data = pandas.concat((data, csv))
    data = data.groupby("subset").aggregate({"dijkstra": "first", "radix": "first"})


    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    width = 0.8
    x = numpy.array(range(len(data.index))) * width * 4 - width
    offset = 0
    for column in data:
        rects = axes.bar(x + offset, data[column], width, label = column)
        # axes.bar_label(rects)
        offset += width

    # axes.legend(data.headers)
    axes.set_xticks(x + width, data.index)
    axes.legend()
    axes.set_ylabel("time (ms)")
    figure.savefig("random-n.png")

entry(sys.argv[1:])
