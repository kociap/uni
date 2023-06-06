#pragma once

#include <graph.hpp>

i32 edmonds_karp(Graph& graph, i32 const source, i32 const sink,
                 i32& augmenting_paths);
