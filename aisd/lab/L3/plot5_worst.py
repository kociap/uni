import matplotlib.pyplot as plt
import pandas

def read_data_file(path_function, index_function, count):
    means = pandas.DataFrame(columns=["time", "comparisons", "swaps"])
    for i in range(1, count + 1):
        csv = pandas.read_csv(path_function(i), header=None, names=["time", "comparisons", "swaps"])
        index = index_function(i)
        csv_means = csv.mean().rename(index).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

data_info = [
    { "name": "qsort", "path": lambda i: f"./o/p5_worst/qsort_nonsel{100 * i}.txt", "count": 100},
    { "name": "qsort select", "path": lambda i: f"./o/p5_worst/qsort{100 * i}.txt", "count": 49},
]

times = pandas.DataFrame();
comparisons = pandas.DataFrame();
swaps = pandas.DataFrame();
for info in data_info:
    means = read_data_file(info["path"], lambda i: 100 * i, info["count"])
    times.insert(len(times.columns), f"{info['name']}", means.iloc[:, 0])
    comparisons.insert(len(comparisons.columns), f"{info['name']}", means.iloc[:, 1])
    swaps.insert(len(swaps.columns), f"{info['name']}", means.iloc[:, 2])

times_axes = times.plot()
times_axes.set_ylabel("time (ns)")
times_axes.figure.savefig("./sort_worst_time.png")
print(times)
comparisons_axes = comparisons.plot()
comparisons_axes.set_ylabel("comparisons")
comparisons_axes.figure.savefig("./sort_worst_compares.png")
print(comparisons)
swaps_axes = swaps.plot()
swaps_axes.set_ylabel("swaps")
swaps_axes.figure.savefig("./sort_worst_swaps.png")
print(comparisons)
