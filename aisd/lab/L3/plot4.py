import matplotlib.pyplot as plt
import pandas
import math

def read_data_file(path_function, index_function, count):
    means = pandas.DataFrame(columns=["time", "comparisons"])
    for i in range(2, count + 1):
        csv = pandas.read_csv(path_function(i), header=None, names=["time", "comparisons"])
        index = index_function(i)
        csv_means = csv.mean().rename(index).to_frame().T
        means = pandas.concat([means, csv_means])
    return means

def generate_master_data(index_function, count):
    data = pandas.DataFrame(columns=["time", "comparisons"])
    for i in range(1, count + 1):
        index = index_function(i)
        value = math.log2(index)
        row = pandas.Series(data=[50 * value, 1.5 * value], index=["time", "comparisons"], name=index).to_frame().T
        data = pandas.concat([data, row])
    return data

data_info = [
    { "name": "beg", "path": lambda i: f"./o/p4/beg{5000 * i}.txt"},
    { "name": "end", "path": lambda i: f"./o/p4/end{5000 * i}.txt"},
    { "name": "mid", "path": lambda i: f"./o/p4/mid{5000 * i}.txt"},
    { "name": "out", "path": lambda i: f"./o/p4/out{5000 * i}.txt"},
    { "name": "rand", "path": lambda i: f"./o/p4/rand{5000 * i}.txt"},
]

times = pandas.DataFrame();
comparisons = pandas.DataFrame();
for info in data_info:
    means = read_data_file(info["path"], lambda i: 5000 * i, 20)
    times.insert(len(times.columns), info["name"], means.iloc[:, 0])
    comparisons.insert(len(comparisons.columns), info["name"], means.iloc[:, 1])

master_data = generate_master_data(lambda i: 5000 * i, 20)
times.insert(len(times.columns), "master", master_data.iloc[:, 0])
comparisons.insert(len(comparisons.columns), "master", master_data.iloc[:, 1])

def compute_constant_factors(dataframe):
    constant_factors = pandas.DataFrame()
    for row in dataframe.iterrows():
        lg = math.log2(row[0])
        frow = row[1].apply(lambda v: v / lg).to_frame().T
        constant_factors = pandas.concat([constant_factors, frow])
    return constant_factors

time_constant_factors = compute_constant_factors(times).mean().to_frame().T.iloc[:, 0:5]
comparisons_constant_factors = compute_constant_factors(comparisons).mean().to_frame().T.iloc[:, 0:5]

times_axes = times.plot()
times_axes.set_ylabel("time (ns)")
times_axes.legend(loc = "upper left")
times_axes.figure.savefig("./binsearch_time.png")
comparisons_axes = comparisons.plot()
comparisons_axes.set_ylabel("comparisons")
comparisons_axes.legend(loc = "upper left")
comparisons_axes.figure.savefig("./binsearch_compares.png")

time_cf = time_constant_factors.plot.bar(xticks=[], linewidth=5)
time_cf.figure.savefig("./binsearch_time_cf.png")
comparisons_cf = comparisons_constant_factors.plot.bar(xticks=[])
comparisons_cf.figure.savefig("./binsearch_compares_cf.png")
