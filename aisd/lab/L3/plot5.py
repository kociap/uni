import matplotlib.pyplot as plt
import pandas
import math

def read_data_file(path_function, index_function, count):
    means = pandas.DataFrame(columns=["time", "comparisons", "swaps"])
    for i in range(1, count + 1):
        csv = pandas.read_csv(path_function(i), header=None, names=["time", "comparisons", "swaps"])
        index = index_function(i)
        csv_means = csv.mean().rename(index).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

data_info = [
    { "name": "qsort", "path": lambda i: f"./o/p5/qsort_{100 * i}.txt"},
    { "name": "dpqsort", "path": lambda i: f"./o/p5/dpqsort_{100 * i}.txt"},
    { "name": "qsort select", "path": lambda i: f"./o/p5/qsort_select_{100 * i}.txt"},
    { "name": "dpqsort select", "path": lambda i: f"./o/p5/dpqsort_select_{100 * i}.txt"},
]

times = pandas.DataFrame();
comparisons = pandas.DataFrame();
swaps = pandas.DataFrame();
for info in data_info:
    means = read_data_file(info["path"], lambda i: 100 * i, 100)
    times.insert(len(times.columns), f"{info['name']}", means.iloc[:, 0])
    comparisons.insert(len(comparisons.columns), f"{info['name']}", means.iloc[:, 1])
    swaps.insert(len(swaps.columns), f"{info['name']}", means.iloc[:, 2])

def compute_constant_factors(dataframe):
    constant_factors = pandas.DataFrame()
    for row in dataframe.iterrows():
        fac = row[0] * math.log2(row[0])
        frow = row[1].apply(lambda v: v / fac).to_frame().T
        constant_factors = pandas.concat([constant_factors, frow])
    return constant_factors

comparisons_cf = compute_constant_factors(comparisons).mean().to_frame().T

# Time
times_axes = times.plot()
times_axes.set_ylabel("time (ns)")
times_axes.figure.savefig("./sort_time.png")
# Comparisons
comparisons_axes = comparisons.plot()
comparisons_axes.set_ylabel("comparisons")
comparisons_axes.figure.savefig("./sort_compares.png")
# Swaps
swaps_axes = swaps.plot()
swaps_axes.set_ylabel("swaps")
swaps_axes.figure.savefig("./sort_swaps.png")
# Comparisons constant factor
ccf_axes = comparisons_cf.plot.bar(xticks=[])
ccf_axes.figure.savefig("./sort_compares_cf.png")

