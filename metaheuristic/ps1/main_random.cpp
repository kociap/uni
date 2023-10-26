#include "anton/types.hpp"
#include <anton/console.hpp>
#include <anton/format.hpp>
#include <anton/math/math.hpp>
#include <anton/string.hpp>
#include <anton/swap.hpp>

#include <loader.hpp>

#include <random>

using namespace anton::literals;

template<typename T>
void shuffle_fisher_yates(anton::Slice<T> elements)
{
  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<u64> distribution(0, elements.size());
  // Fisher-Yates shuffle. Modulo biases the results as the distribution is
  // not uniform anymore, but should be sufficient for our needs.
  for(i64 i = elements.size() - 1; i > 0; i -= 1) {
    i64 j = distribution(generator) % (i + 1);
    anton::swap(elements[i], elements[j]);
  }
}

[[nodiscard]] i64 compute_cycle_weight(anton::Slice<Vertex const> const path)
{
  i64 weight = 0;
  Edge const starting_edge = calculate_edge_between(*path.end(), path[0]);
  weight += starting_edge.weight;
  auto first = path.begin();
  auto second = first + 1;
  auto end = path.end();
  for(; second != end; first += 1, second += 1) {
    Edge const edge = calculate_edge_between(*first, *second);
    weight += edge.weight;
  }
  return weight;
}

int main(int argc, char** argv)
{
  anton::STDOUT_Stream stdout;
  anton::STDERR_Stream stderr;

  if(argc < 2) {
    stderr.write(anton::format("{} <file>\n"_sv, argv[0]));
    return 1;
  }

  anton::String file(argv[1]);
  Graph graph = load_graph(file);

  i64 avg_100x10 = 0;
  for(i64 i = 0; i < 100; i += 1) {
    i64 minimum = anton::limits::maximum_i64;
    for(i64 j = 0; j < 10; j += 1) {
      shuffle_fisher_yates<Vertex>(graph.vertices);
      i64 const result = compute_cycle_weight(graph.vertices);
      minimum = anton::math::min(minimum, result);
    }
    avg_100x10 += minimum;
  }
  avg_100x10 /= 100;
  stdout.write(anton::format("average 100x10 {}\n", avg_100x10));

  i64 avg_20x50 = 0;
  for(i64 i = 0; i < 20; i += 1) {
    i64 minimum = anton::limits::maximum_i64;
    for(i64 j = 0; j < 50; j += 1) {
      shuffle_fisher_yates<Vertex>(graph.vertices);
      i64 const result = compute_cycle_weight(graph.vertices);
      minimum = anton::math::min(minimum, result);
    }
    avg_20x50 += minimum;
  }
  avg_20x50 /= 20;
  stdout.write(anton::format("average 20x50 {}\n", avg_20x50));

  i64 minimum_1000 = anton::limits::maximum_i64;
  for(i64 i = 0; i < 1000; i += 1) {
    shuffle_fisher_yates<Vertex>(graph.vertices);
    i64 const result = compute_cycle_weight(graph.vertices);
    minimum_1000 = anton::math::min(minimum_1000, result);
  }
  stdout.write(anton::format("average 1000 {}\n", minimum_1000));

  return 0;
}
