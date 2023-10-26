#pragma once

#include <graph.hpp>

[[nodiscard]] anton::Array<Edge> tsp_cycle_from_mst(Graph& graph);
