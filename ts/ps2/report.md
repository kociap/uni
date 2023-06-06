---
title: \vspace{8cm} Technologie Sieciowe 2022/2023
subtitle: Laboratorium 2
author: Piotr Kocia
date: 17th April 2023

numbersections: true

geometry: "left=2cm,right=2cm,top=2cm,bottom=2cm"
linestretch: 1.5
fontsize: 12pt

header-includes: |
    \usepackage{graphicx}
    \usepackage{wrapfig}
    \usepackage{float}
---

\thispagestyle{empty}

\newpage

\setcounter{page}{1}

\tableofcontents

# Introduction and Code

We use a python package called `networkx` to handle most of the networking specific bits.

```py
import random
import networkx as nx
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
```

The graph is a dodecahedron defined as an adjacency list

```py
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

def create_graph():
    return nx.from_numpy_array(adjacency)
```

We use a procedure to generate a random intensity matrix.

```py
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
```

To calculate the flow in a graph, we use all-to-all shortest path algorithm from networkx to find all shortest paths in the graph. Then we iterate over all paths from $i$ to $j$ and add to the constituent edges the corresponding flow between nodes $i$ and $j$. The capacity is a constant multiplied with a coefficient.

```py
def calculate_flow(graph, N):
    nx.set_edge_attributes(graph, 0, "a")
    paths = nx.all_pairs_shortest_path(graph)
    for i, targets in paths:
        for j, path in targets.items():
            for n in range(len(path) - 1):
                graph.edges[path[n], path[n + 1]]["a"] += N[i][j]

def calculate_capacity(graph, c):
    nx.set_edge_attributes(graph, 96 * 1024 * c, "c")
```

The test function tests every edge whether it should be damaged according to the given probability parameter, then tests whether the graph remains connected. Further tests on latency and capacity are performed afterwards. The capacity test simply verifies that every edge in the graph satisfies the condition $a < c$ (flow less than capacity). The latency test calculates the mean latency of packets in the network
$$
T = \frac{\sum_e \left( \frac{e_a}{e_c  m - e_a} \right)}{\sum_{i,  j} N_{i, j}}
$$
where $m$ is the average packet size in bits (in our case 56), and then checks $T < T_{max}$, where $T_{max}$ is a parameter.

```py
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
```

# Results
Three separate tests have been conducted, each one with a different parameter being varied. The probability of an edge being damaged is 20%.

## Varying Intensity
\begin{figure}[h]
\centering
\includegraphics[]{intensity.png}
\caption{Success rate with increasing intensity matrix.}
\label{fig:intensity}
\end{figure}
We change the intensity matrix by multiplying every entry by a constant factor which gradually increases starting at 0.3 up to 1.7 in increments of 0.04. The succcess rate begins to fall around 1.0 which naturally comes from the fact that higher flow is blocked by the capacity of the network.

## Varying Capacity
\begin{figure}[h]
\centering
\includegraphics{capacity.png}
\caption{Success rate with increasing capacity.}
\label{fig:capacity}
\end{figure}
We vary the capacity using a factor to scale the capacity at every egde. The factor increases from 0.3 up to 1.7 in increments of 0.04. The success rate increases from nearly 0% at 0.3 up to 80% at 1.0 and stabilises at that value meaning that at that point the network failures are caused by the graph becoming disconnected.

## Varying Edges
\begin{figure}[h]
\centering
\includegraphics{edges.png}
\caption{Success rate with gradually more edges being added.}
\label{fig:edges}
\end{figure}
We add random edges to the graph with creating additional pathways between nodes. The success rate increases from 80% up to 99% at 30 additional edges. Thanks to many additional edges, the degree of each node increases and the network has a much higher chance of remaining connected. Precisely, the probability of a node $n$ to become disconnected is equal
$$
p^{d(n)}
$$
In the case of an undirected dodecahedral graph with 30 undirected edges and degree 3 at every node, adding 30 undirected edges approaches the maximum number of 95 edges. This, assuming uniform distribution of newly created edges, raises the degree of each node to approximately 6 resulting in the probability of a node becoming disconnected plummeting from $0.2^3 = 0.008$ to a mere $0.2^6 = 0.000064$!
