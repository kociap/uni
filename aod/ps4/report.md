---
title: \vspace{8cm} Algorytmy Optymalizacji Dyskretnej 2022/2023
author: Piotr Kocia
date: 6th June 2023

numbersections: true

geometry: "top=2cm,bottom=2cm"
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

# Maximum flow
We construct a graph such that each vertex has directed edges connecting it to vertices whose index has popcount greater by exactly one. We randomly assign capacities of at least 1 to the edges. The total number of vertices in the graph is $2^k$ where $k$ is a parameter to the program. We gather information for $k \in \left\{ 1, .., 16 \right\}$ performing several runs for each value of $k$.
\begin{figure}[h]
\centering
\includegraphics{edmonds_time.png}
\caption{Average computation time across several runs.}
\label{fig:edmonds_time}
\end{figure}
\begin{figure}[h]
\centering
\includegraphics{edmonds_flow.png}
\caption{Average maximum flow across several runs.}
\label{fig:edmonds_flow}
\end{figure}
\begin{figure}[h]
\centering
\includegraphics{edmonds_paths.png}
\caption{Average number of augmenting paths across several runs.}
\label{fig:edmonds_paths}
\end{figure}
The plots are linearised with log$_{2}$ to better present the results across the vast range.

It is clearly visible that all metrics (time in Fig. \ref{fig:edmonds_time}, flow in Fig. \ref{fig:edmonds_flow} and augmenting paths in Fig. \ref{fig:edmonds_paths}) increase exponentially with $k$, the function being approximately $2^{k}$, hence proportionally to the number of vertices in the graph.

# Perfect matchings in bipartite graphs
We use the reduction of the maximal matching problem to the network flow problem in the bipartite graphs and use the Edmonds-Karp algorithm. We construct a bipartite graph consisting of two partitions of size $2^k$ such that the vertices in the first partition have exactly $d < k$ outgoing edges each. Every edge is assigned the capacity 1. We additionally add a source and a sink which are connected via directed edges to ensure the graph is connected. We then proceed to run Edmonds-Karp algorithm from the source to the sink in order to find the maximal matching.
\begin{figure}[h]
\centering
\includegraphics{bipartite_time.png}
\caption{Average computation time with respect to the degree across several runs.}
\label{fig:capacity}
\end{figure}
\begin{figure}[h]
\centering
\includegraphics{bipartite_matchings.png}
\caption{Average number of matchings with respect to the degree across several runs.}
\label{fig:capacity}
\end{figure}
The degree clearly affects the computation time since it increases the number of edges in the graph, yet its effect appears to diminish with increase in $d$. However, the degree does not seem to have much of an effect past a certain point in the case of edges in a maximal matching.
