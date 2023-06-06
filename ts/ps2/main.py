import random
import networkx as nx
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

# Dodecahedron
adjacency = np.array((
    (0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    (1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0),
    (0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0),
    (0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0),
    (0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0),
    (0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0),
    (0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
    (0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0)
), ndmin = 2)

def generate_N(lower, upper):
    N = []
    for i in range(0, 20):
        col = []
        for j in range(0, 20):
            if i == j:
                col.append(0)
            else:
                col.append(random.randint(lower, upper))
        N.append(col);
    return N

def create_graph():
    return nx.from_numpy_array(adjacency)

def calculate_flow(graph, N):
    nx.set_edge_attributes(graph, 0, "a")
    paths = nx.all_pairs_shortest_path(graph)
    for i, targets in paths:
        for j, path in targets.items():
            for n in range(len(path) - 1):
                graph.edges[path[n], path[n + 1]]["a"] += N[i][j]

def calculate_capacity(graph, c):
    nx.set_edge_attributes(graph, 96 * 1024 * c, "c")

def run_test(N, p, t_max, c = 1, new_edges = 0):
    def test_throughput(graph, N):
        for u, v, d in graph.edges(data = True):
            if (d["c"] / 56) <= d["a"]:
                return False
        return True

    def test_latency(graph, t_max, N):
        g = 0
        for u in range(20):
            for v in range(20):
                g += N[u][v]

        sum_e = 0
        for u, v, d in graph.edges(data = True):
            a = d["a"]
            c = d["c"]
            sum_e += a / (c / 56 - a)

        t = sum_e / g
        return t <= t_max

    graph = create_graph()
    # Add edges
    for i in range(new_edges):
        nodes = list(graph.nodes)
        while(True):
            u = random.choice(nodes)
            v = random.choice(nodes)
            # Verify the randomised nodes are not the same node and there is
            # no edge between them.
            if not u == v and u not in graph.neighbors(v):
                graph.add_edge(u, v)
                break

    for u, v in graph.edges:
        if random.random() <= p:
            graph.remove_edge(u, v)

    if not nx.is_connected(graph):
        return False

    calculate_flow(graph, N)
    calculate_capacity(graph, c)

    if not test_throughput(graph, N):
        return False
    if not test_latency(graph, t_max, N):
        return False

    return True

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

def main():
    N = generate_N(3, 15)
    p = 0.8
    failure_probability = 1 - p
    t_max = 0.07
    run_count = 750

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    edges_x_axis = np.arange(0, 30, 1)
    edges_data = []
    for new_edges in edges_x_axis:
        print(f"test {new_edges}", flush = True)
        successes = 0
        for i in range(0, run_count):
            result = run_test(N = N, p = failure_probability, t_max = t_max, new_edges = new_edges)
            if result == True:
                successes += 1
        edges_data.append(successes / run_count)
    axes.plot(edges_x_axis, edges_data, c = "C2")
    axes.set_xlabel("Edges added")
    axes.set_ylabel("Success rate")
    figure.savefig("edges.png")

    del figure

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    capacity_x_axis = np.linspace(0.3, 1.7, 36, endpoint=True)
    capacity_data = []
    for capacity_coeff in capacity_x_axis:
        print(f"test {capacity_coeff}", flush = True)
        successes = 0
        for i in range(0, run_count):
            result = run_test(N = N, p = failure_probability, t_max = t_max, c = capacity_coeff)
            if result == True:
                successes += 1
        capacity_data.append(successes / run_count)
    axes.plot(capacity_x_axis, capacity_data, c = "C2")
    axes.set_xlabel("Capacity coefficient")
    axes.set_ylabel("Success rate")
    figure.savefig("capacity.png")

    del figure

    figure = mpl.figure.Figure(figsize = get_figsize(latex_columnwidth), dpi = 144)
    axes = figure.add_subplot(111)
    intensity_x_axis = np.linspace(0.3, 1.7, 36, endpoint=True)
    intensity_data = []
    for coeff in intensity_x_axis:
        print(f"test {coeff}", flush = True)
        successes = 0
        for i in range(0, run_count):
            result = run_test(N = np.multiply(N, coeff), p = failure_probability, t_max = t_max)
            if result == True:
                successes += 1
        intensity_data.append(successes / run_count)
    axes.plot(intensity_x_axis, intensity_data, c = "C2")
    axes.set_xlabel("Intensity coefficient")
    axes.set_ylabel("Success rate")
    figure.savefig("intensity.png")

main()
