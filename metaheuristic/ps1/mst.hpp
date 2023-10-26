#pragma once

#include <anton/array.hpp>

#include <graph.hpp>
#include <types.hpp>

[[nodiscard]] anton::Array<Edge> compute_mst_kruskal(Graph& graph);
