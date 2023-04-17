#include <iostream>
#include <span>
#include <stack>
#include <vector>

#include <common.hpp>

using Color = bool;
constexpr Color COLOR_BLACK = false;
constexpr Color COLOR_RED = true;

struct Vertex {
    std::vector<i32> edges;
    Color color = COLOR_BLACK;
    bool colored = false;

    Vertex() {
        edges.reserve(16);
    }
};

[[nodiscard]] bool twocolor_graph(std::span<Vertex> const vertices) {
    struct Vertex_Info {
        Vertex* vertex;
        Color last_color;
    };

    std::stack<Vertex_Info> stack;
    for(Vertex& source: vertices) {
        if(source.colored) {
            continue;
        }

        stack.push({&source, COLOR_BLACK});
        while(stack.size() > 0) {
            auto const [vertex, last_color] = stack.top();
            stack.pop();
            if(vertex->colored) {
                if(vertex->color == last_color) {
                    return false;
                }
            } else {
                vertex->color = !last_color;
                vertex->colored = true;
                for(i32 const index: vertex->edges) {
                    Vertex* const neighbor = vertices.data() + index;
                    stack.push({neighbor, !last_color});
                }
            }
        }
    }
    return true;
}

int main() {
    std::optional<std::vector<Vertex>> graph_read_result = read_graph<Vertex>();
    if(!graph_read_result) {
        return 1;
    }

    std::vector<Vertex>& vertices = graph_read_result.value();

    Timer coloring_timer;
    coloring_timer.start();
    bool const result = twocolor_graph(vertices);
    std::cerr << "graph colored in " << coloring_timer.end() << "ns\n";
    if(!result) {
        std::cout << "0\n";
    } else {
        std::cout << "1\n";
        if(vertices.size() <= 200) {
            std::cout << "black:\n";
            for(i32 index = 1; Vertex const& vertex: vertices) {
                if(vertex.color == COLOR_BLACK) {
                    std::cout << index << '\n';
                }
                index += 1;
            }

            std::cout << "red:\n";
            for(i32 index = 1; Vertex const& vertex: vertices) {
                if(vertex.color == COLOR_RED) {
                    std::cout << index << '\n';
                }
                index += 1;
            }
        }
    }

    return 0;
}
