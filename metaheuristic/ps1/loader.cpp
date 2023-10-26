#include "graph.hpp"
#include <loader.hpp>

#include <anton/filesystem.hpp>
#include <anton/math/math.hpp>
#include <anton/optional.hpp>

[[nodiscard]] static anton::Optional<anton::String>
read_line(anton::fs::Input_File_Stream& stream, char8 const delimiter)
{
  bool const good = !stream.eof() && !stream.error();
  if(!good) {
    return anton::null_optional;
  }

  anton::String result;
  while(true) {
    char8 const c = stream.get();
    if(c == anton::eof_char8 || c == delimiter) {
      break;
    }

    result.append(c);
  }
  return result;
}

[[nodiscard]] static bool is_digit(char8 const c)
{
  return c >= '0' && c <= '9';
}

struct Parse_Result {
  i64 value;
  char8 const* end;
};

[[nodiscard]] static Parse_Result parse_i64(char8 const* begin,
                                            char8 const* const end)
{
  while(begin != end && !is_digit(*begin)) {
    ++begin;
  }

  i64 value = 0;
  while(begin != end && is_digit(*begin)) {
    value = value * 10 + (*begin - '0');
    ++begin;
  }

  return {value, begin};
}

Graph load_graph(anton::String const& path)
{
  Graph graph;
  anton::Array<Vertex>& vertices = graph.vertices;
  anton::fs::Input_File_Stream file(path);
  i64 vertex_index = 0;
  while(true) {
    anton::Optional result = read_line(file, '\n');
    if(!result) {
      break;
    }

    anton::String& line = result.value();
    if(line.size_bytes() == 0) {
      continue;
    }

    char8 const c = *line.data();
    if(!is_digit(c)) {
      continue;
    }

    char8 const* begin = line.data();
    char8 const* const end = line.data() + line.size_bytes();
    Parse_Result result1 = parse_i64(begin, end);
    Parse_Result result2 = parse_i64(result1.end, end);
    Parse_Result result3 = parse_i64(result2.end, end);
    vertices.push_back(
      Vertex{{}, nullptr, vertex_index, result2.value, result3.value});
    vertex_index += 1;
  }

  for(i64 src = 0; src < vertices.size(); src += 1) {
    for(i64 dst = src + 1; dst < vertices.size(); dst += 1) {
      Edge edge = calculate_edge_between(vertices[src], vertices[dst]);
      graph.edges.push_back(edge);
    }
  }

  return graph;
}
