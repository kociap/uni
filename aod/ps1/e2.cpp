#include <iostream>
#include <optional>
#include <queue>
#include <span>
#include <stack>
#include <string_view>
#include <vector>

#include <common.hpp>

struct Vertex {
    std::vector<i32> edges;
    i32 index = 0;
    bool visited = false;
    bool pathed = false;

    Vertex() {
        edges.reserve(16);
    }
};

struct Traversal_Data {
    std::vector<i32> order;
    bool cycle = false;
};

[[nodiscard]] Traversal_Data dfs(std::span<Vertex> const vertices) {
    Traversal_Data data;
    struct Frame {
        Vertex* vertex;
        i32 edge;
    };
    std::stack<Frame> path;
    for(Vertex& source: vertices) {
        if(source.visited) {
            continue;
        }

        path.push({&source, 0});
        while(path.size() > 0) {
            auto& [vertex, edge] = path.top();
            vertex->pathed = true;
            vertex->visited = true;

            if(edge >= static_cast<i32>(vertex->edges.size())) {
                data.order.push_back(vertex->index);
                vertex->pathed = false;
                path.pop();
                continue;
            }

            while(edge < static_cast<i32>(vertex->edges.size())) {
                Vertex& n = vertices[vertex->edges[edge]];
                edge += 1;
                if(n.pathed) {
                    // Impossible to sort.
                    data.cycle = true;
                    return data;
                }

                if(!n.visited) {
                    path.push({&n, 0});
                    break;
                }
            }
        }
    }
    return data;
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
    Traversal_Data data = dfs(vertices);
    std::cerr << "graph traversed in " << traverse_timer.end() << "ns\n";
    if(data.cycle) {
        std::cout << "graph has a cycle\n";
    } else {
        for(i32 const index: data.order) {
            std::cout << index << '\n';
        }
    }
    return 0;
}
