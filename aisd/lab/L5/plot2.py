import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas
import math

def read_data_file(path):
    means = pandas.DataFrame(columns = ['comparisons', 'swaps'])
    for i in range(1, 11):
        csv = pandas.read_csv(path.format(10000 * i), header = None, names = ['comparisons', 'swaps'])
        csv_means = csv.mean().rename(10000 * i).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

def create_plot(data):
    figure = mpl.figure.Figure()
    axes = figure.add_subplot(111)
    # Plot scatter for statistics.
    col0 = data.iloc[:, 0]
    axes.plot(col0.index, col0, c="C1")
    col1 = data.iloc[:, 1]
    axes.plot(col1.index, col1, c="C2")
    # Add legend from column names.
    axes.legend(data.columns, markerscale=4)
    return figure, axes

data_heap = read_data_file("./o/out_heap_{}.txt")
figure, axes = create_plot(data_heap)
figure.savefig(f"./heap.png")
