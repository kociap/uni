#pragma once

#include <chrono>
#include <optional>
#include <stdio.h>
#include <vector>

#ifndef ENABLE_HOTSPOT_ERRORS
    #define ENABLE_HOTSPOT_ERRORS 0
#endif

using u8 = unsigned char;
using i32 = int;
using i64 = long long;

#define INPUT_BUFFER_SIZE 4096

struct String_View {
    char* begin;
    char* end;
};

struct Input_Buffer {
    u8 buf[INPUT_BUFFER_SIZE];
    u8* i;
    u8* end;

    Input_Buffer(): i(buf), end(buf) {
        buf[0] = EOF;
    }

    [[nodiscard]] bool read_byte(u8& v) {
        ensure_input(1);
        if(i != end) {
            v = *i;
            i += 1;
            return true;
        } else {
            return false;
        }
    }

    bool read_i32(i32& v) {
        ensure_input(10);
        while((*i < '0' || *i > '9') & (*i != (u8)EOF)) {
            ++i;
        }

        if(i == end) {
            return false;
        }

        v = 0;
        while((*i >= '0') & (*i <= '9')) {
            v = 10 * v + *i - '0';
            i += 1;
        }

        return true;
    }

    bool read_i64(i64& v) {
        ensure_input(20);
        while((*i < '0' || *i > '9') & (*i != (u8)EOF)) {
            ++i;
        }

        if(i == end) {
            return false;
        }

        v = 0;
        while((*i >= '0') & (*i <= '9')) {
            v = 10 * v + *i - '0';
            i += 1;
        }

        return true;
    }

private:
    void ensure_input(i64 const threshold) {
        i64 const remaining = end - i;
        if(remaining < threshold) {
            u8* b = i;
            i = buf;
            while(b != end) {
                *i = *b;
                ++i;
                ++b;
            }

            i64 const count =
                fread(i, 1, INPUT_BUFFER_SIZE - 1 - remaining, stdin);
            i = buf;
            end = buf + remaining + count;
            *end = EOF;
        }
    }
};

struct Output_Buffer {
public:
    char buf[65536];
    char* i;
    char* end;

    Output_Buffer(): i(buf), end(buf + 65536) {}

    ~Output_Buffer() {
        fwrite(buf, 1, i - buf, stdout);
    }

    void write_i64(i64 v) {
        if(end - i < 30) {
            fwrite(buf, 1, i - buf, stdout);
            i = buf;
        }

        char buffer[21];
        String_View number = to_string(v, buffer);
        for(; number.begin != number.end; ++number.begin, ++i) {
            *i = *number.begin;
        }
    }

    void write_newline() {
        if(end - i < 1) {
            fwrite(buf, 1, i - buf, stdout);
            i = buf;
        }

        *i = '\n';
        ++i;
    }

private:
    // buffer must be a char array of 21 elements.
    String_View to_string(i64 v, char* buffer) {
        // We don't need null-terminator or initialized elements.
        bool has_sign = false;
        if(v < 0) {
            has_sign = true;
            v = -v;
        }

        i32 i = 20;
        do {
            buffer[i] = (v % 10) | '0';
            --i;
            v /= 10;
        } while(v > 0);
        buffer[i] = '-';
        return {buffer + i + !has_sign, buffer + 21};
    }
};

struct Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;

public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] i64 end() {
        auto const end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                                    start_time)
            .count();
    }
};

template<typename Vertex>
[[nodiscard]] inline std::optional<std::vector<Vertex>> read_graph() {
    Input_Buffer in;

    u8 graph_kind;
    if(!in.read_byte(graph_kind)) {
        std::cerr << "error: could not read graph kind\n";
        return std::nullopt;
    }

    bool const directed = graph_kind == 'D';

    i32 num_vertices;
    if(!in.read_i32(num_vertices)) {
        std::cerr << "error: could not read number of vertices\n";
        return std::nullopt;
    }

    std::vector<Vertex> vertices;
    vertices.resize(num_vertices);

    i32 num_edges;
    if(!in.read_i32(num_edges)) {
        std::cerr << "error: could not read number of edges\n";
        return std::nullopt;
    }

    Timer edges_read_timer;
    edges_read_timer.start();
    for(i32 i = 0; i < num_edges; i += 1) {
        i32 src;
        if constexpr(!ENABLE_HOTSPOT_ERRORS) {
            in.read_i32(src);
        } else {
            if(!in.read_i32(src)) {
                std::cerr << "error: could not read edge source\n";
                return std::nullopt;
            }
        }

        i32 dst;
        if constexpr(!ENABLE_HOTSPOT_ERRORS) {
            in.read_i32(dst);
        } else {
            if(!in.read_i32(dst)) {
                std::cerr << "error: could not read edge destination\n";
                return std::nullopt;
            }
        }

        vertices[src - 1].edges.push_back(dst - 1);
        if(!directed) {
            vertices[dst - 1].edges.push_back(src - 1);
        }
    }
    std::cerr << "edges read in " << edges_read_timer.end() << "ns\n";
    return vertices;
}
