import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.backends.backend_agg as mplbackend
import pandas
import math

def read_data_file(path):
    csv = pandas.read_csv(path, header = None, names = ["comparisons", "accesses", "assigns", "height"])
    return csv

def create_plot(data):
    figure = mpl.figure.Figure()
    axes = figure.add_subplot(111)
    # Plot scatter for statistics.
    comparisons = data.iloc[:, 0]
    axes.scatter(comparisons.index, comparisons, c="C0", s=1)
    accesses = data.iloc[:, 1]
    axes.scatter(accesses.index, accesses, c="C1", s=1)
    assigns = data.iloc[:, 2]
    axes.scatter(assigns.index, assigns, c="C2", s=1)
    # Plot line for height.
    height = data.iloc[:, 3]
    axes.plot(height.index, height, color="C3")
    # Add legend from column names.
    axes.legend(data.columns, markerscale=4)
    return figure, axes

for index in range(1, 11):
    data = read_data_file(f"./o/out_rbt_rand_{10000 * index}.txt")
    figure, axes = create_plot(data)
    axes.set_xticks(ticks = [], labels = [])
    figure.savefig(f"./rbt_rand_{10000 * index}.png")

for index in range(1, 11):
    data = read_data_file(f"./o/out_rbt_lin_{10000 * index}.txt")
    figure, axes = create_plot(data)
    axes.set_xticks(ticks = [], labels = [])
    figure.savefig(f"./rbt_lin_{10000 * index}.png")