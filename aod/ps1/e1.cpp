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

    Vertex() {
        edges.reserve(16);
    }
};

struct Edge {
    i32 src;
    i32 dst;
};

struct Traversal_Data {
    std::vector<i32> order;
    std::vector<Edge> tree;
};

[[nodiscard]] Traversal_Data dfs(std::span<Vertex> const vertices) {
    struct Vertex_Info {
        Vertex* vertex;
        Vertex* parent;
    };

    Traversal_Data data;
    std::stack<Vertex_Info> stack;
    for(Vertex& source: vertices) {
        if(source.visited) {
            continue;
        }

        stack.push({&source, nullptr});
        while(stack.size() > 0) {
            auto const [vertex, parent] = stack.top();
            stack.pop();
            if(!vertex->visited) {
                data.order.push_back(vertex->index);
                vertex->visited = true;
                if(parent != nullptr) {
                    data.tree.push_back(Edge{parent->index, vertex->index});
                }

                for(i32 const index: vertex->edges) {
                    Vertex* const neighbor = vertices.data() + index;
                    stack.push({neighbor, vertex});
                }
            }
        }
    }
    return data;
}

[[nodiscard]] Traversal_Data bfs(std::span<Vertex> const vertices) {
    struct Vertex_Info {
        Vertex* vertex;
        Vertex* parent;
    };

    Traversal_Data data;
    std::queue<Vertex_Info> queue;
    for(Vertex& source: vertices) {
        if(source.visited) {
            continue;
        }

        queue.push({&source, nullptr});
        while(queue.size() > 0) {
            auto const [vertex, parent] = queue.front();
            queue.pop();
            if(!vertex->visited) {
                data.order.push_back(vertex->index);
                vertex->visited = true;
                if(parent != nullptr) {
                    data.tree.push_back(Edge{parent->index, vertex->index});
                }

                for(i32 const index: vertex->edges) {
                    Vertex* const neighbor = vertices.data() + index;
                    queue.push({neighbor, vertex});
                }
            }
        }
    }
    return data;
}

enum struct Algorithm_Kind {
    dfs,
    bfs,
};

struct Options {
    Algorithm_Kind algorithm;
    bool tree = false;
};

[[nodiscard]] std::optional<Options> parse_options(int argc, char** argv) {
    Options options;
    bool algorithm_selected = false;
    for(i32 i = 1; i < argc; i += 1) {
        std::string_view arg(argv[i]);
        if(arg == "--dfs") {
            options.algorithm = Algorithm_Kind::dfs;
            algorithm_selected = true;
        } else if(arg == "--bfs") {
            options.algorithm = Algorithm_Kind::bfs;
            algorithm_selected = true;
        } else if(arg == "--tree") {
            options.tree = true;
        }
    }

    if(!algorithm_selected) {
        std::cerr << "error: missing algorithm type\n";
        return std::nullopt;
    }

    return options;
}

int main(int argc, char** argv) {
    std::optional<Options> option_parsing_result = parse_options(argc, argv);
    if(!option_parsing_result) {
        return 1;
    }

    Options const options = option_parsing_result.value();

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
    Traversal_Data data;
    switch(options.algorithm) {
        case Algorithm_Kind::dfs:
            data = dfs(vertices);
            break;
        case Algorithm_Kind::bfs:
            data = bfs(vertices);
            break;
    }
    std::cerr << "graph traversed in " << traverse_timer.end() << "ns\n";

    std::cout << "order\n";
    for(i32 const index: data.order) {
        std::cout << index << '\n';
    }

    if(options.tree) {
        std::cout << "tree\n";
        for(Edge const edge: data.tree) {
            std::cout << edge.src << " " << edge.dst << '\n';
        }
    }

    return 0;
}
