import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.backends.backend_agg as mplbackend
import pandas
import math

def read_data_file(path):
    means = pandas.DataFrame(columns = ['compares', 'swaps'])
    for i in range(1, 11):
        csv = pandas.read_csv(path.format(100 * i), header = None, names = ['compares', 'swaps'])
        csv_means = csv.mean().rename(100 * i).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

def create_plot(data):
    figure = mpl.figure.Figure()
    axes = figure.add_subplot(111)
    # Plot scatter for statistics.
    col0 = data.iloc[:, 0]
    axes.plot(col0.index, col0, c="C0")
    col1 = data.iloc[:, 1]
    axes.plot(col1.index, col1, c="C1")
    col2 = data.iloc[:, 2]
    axes.plot(col2.index, col2, c="C2")
    col3 = data.iloc[:, 3]
    axes.plot(col3.index, col3, c="C3")
    # Add legend from column names.
    axes.legend(data.columns, markerscale=4)
    return figure, axes

data_heap = read_data_file("./o/out_sort_heap_{}.txt")
data_qsort = read_data_file("./o/out_sort_qsort_{}.txt")
data_msort = read_data_file("./o/out_sort_msort_{}.txt")
data_dpqsort = read_data_file("./o/out_sort_dpqsort_{}.txt")
comparisons = pandas.concat([data_heap.iloc[:, 0], data_qsort.iloc[:, 0], data_msort.iloc[:, 0], data_dpqsort.iloc[:, 0]], 
                            axis='columns')
comparisons.columns = ["heapsort", "qsort", "msort", "dpqsort"]
figure, axes = create_plot(comparisons)
axes.set_ylabel("comparisons")
figure.savefig(f"./sorts_comparisons.png")

swaps = pandas.concat([data_heap.iloc[:, 1], data_qsort.iloc[:, 1], data_msort.iloc[:, 1], data_dpqsort.iloc[:, 1]], 
                            axis='columns')
swaps.columns = ["heapsort", "qsort", "msort", "dpqsort"]
figure, axes = create_plot(swaps)
axes.set_ylabel("swaps")
figure.savefig(f"./sorts_swaps.png")
