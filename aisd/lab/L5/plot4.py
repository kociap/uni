import matplotlib.pyplot as plt
import matplotlib as mpl
import matplotlib.backends.backend_agg as mplbackend
import pandas
import math

def read_data_file(path):
    means = pandas.DataFrame(columns = ['time'])
    for i in range(1, 11):
        csv = pandas.read_csv(path.format(1000 * i), header = None, names = ['time'])
        csv_means = csv.mean().rename(1000 * i).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

def create_plot(data):
    figure = mpl.figure.Figure()
    axes = figure.add_subplot(111)
    # Plot scatter for statistics.
    col0 = data.iloc[:, 0]
    axes.plot(col0.index, col0, c="C1")
    # Add legend from column names.
    # axes.legend(data.columns, markerscale=4)
    return figure, axes

data_lcs = read_data_file("./o/out_lcs_{}.txt")
data_lcs.columns = ["time"]
figure, axes = create_plot(data_lcs)
axes.set_ylabel("time (ns)")
figure.savefig(f"./lcs_time.png")
