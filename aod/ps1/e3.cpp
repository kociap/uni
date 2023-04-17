#include <iostream>
#include <optional>
#include <queue>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include <common.hpp>

static i32 min(i32 const a, i32 const b) {
    return a < b ? a : b;
}

struct Vertex {
    std::vector<i32> edges;
    i32 index = 0;
    i32 preorder = -1;
    // "lowest" index reachable from this vertex.
    i32 low = -1;
    bool pathed = false;

    Vertex() {
        edges.reserve(16);
    }
};

std::vector<std::vector<Vertex*>> scc(std::span<Vertex> const vertices) {
    i32 preorder = 0;
    struct Frame {
        Vertex* vertex;
        i32 edge;
        Vertex* visited_vertex = nullptr;
    };
    std::stack<Frame> stack;
    std::stack<Vertex*> scc_stack;
    std::vector<std::vector<Vertex*>> sccs;
    for(Vertex& source: vertices) {
        if(source.preorder != -1) {
            continue;
        }

        stack.push({&source, 0, nullptr});
        while(stack.size() > 0) {
            Frame& frame = stack.top();
            Vertex*& vertex = frame.vertex;
            i32& edge = frame.edge;
            Vertex*& visited_vertex = frame.visited_vertex;
            // auto& [vertex, edge, visited_vertex] = stack.top();
            if(vertex->preorder == -1) {
                vertex->preorder = preorder;
                vertex->low = preorder;
                preorder += 1;
                vertex->pathed = true;
                scc_stack.push(vertex);
            }

            // Code from after the recursive call.
            if(visited_vertex != nullptr) {
                vertex->low = min(vertex->low, visited_vertex->low);
                visited_vertex = nullptr;
            }

            i32 const edges = vertex->edges.size();
            bool recurse = false;
            while(edge < edges) {
                Vertex* v = vertices.data() + vertex->edges[edge];
                edge += 1;
                if(v->preorder == -1) {
                    // Store v on our callstack.
                    visited_vertex = v;
                    stack.push({v, 0, nullptr});
                    // Break the flow for a "recursive call".
                    recurse = true;
                    break;
                } else if(v->pathed) {
                    vertex->low = min(vertex->low, v->low);
                }
            }

            if(recurse) {
                continue;
            }

            // We get here only if the loop above ends, hence no vertices to
            // check, hence we wrap up for this vertex.
            if(vertex->low == vertex->preorder) {
                std::vector<Vertex*>& scc = sccs.emplace_back();
                while(true) {
                    Vertex* const v = scc_stack.top();
                    scc_stack.pop();
                    v->pathed = false;
                    scc.push_back(v);
                    if(v == vertex) {
                        break;
                    }
                }
            }
            stack.pop();
        }
    }
    return sccs;
}

int main() {
    std::optional<std::vector<Vertex>> graph_read_result = read_graph<Vertex>();
    if(!graph_read_result) {
        return 1;
    }

    std::vector<Vertex>& vertices = graph_read_result.value();
    for(i32 index = 1; Vertex & vertex: vertices) {
        vertex.index = index;
        index += 1;
    }

    Timer traverse_timer;
    traverse_timer.start();
    std::vector<std::vector<Vertex*>> sccs = scc(vertices);
    std::cerr << "scc found in " << traverse_timer.end() << "ns\n";
    for(i32 index = 1; std::vector<Vertex*> const& scc: sccs) {
        std::cout << "scc " << index << '\n';
        for(Vertex* const vertex: scc) {
            std::cout << vertex->index << '\n';
        }
        index += 1;
    }
    return 0;
}
