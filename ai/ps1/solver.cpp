#include <solver.hpp>

#include <algorithm>
#include <map>
#include <queue>
#include <stack>

#include <debug.hpp>
#include <heap.hpp>

constexpr i32 FOUND = -1;

namespace puzzle15 {
  bool is_solvable(Configuration_View const c) {
    i32 inversions = 0;
    for(i32 i = 1; i < 16; i += 1) {
      for(i32 j = i + 1; j < 16; j += 1) {
        if(c[i] < c[j]) {
          inversions += 1;
        }
      }
    }

    i32 empty_row = 1;
    // No need to check the first row. Check 2nd row immediately.
    if(c[4] == 16 || c[5] == 16 || c[6] == 16 || c[7] == 16) {
      empty_row = 2;
    }
    // Check 3rd row.
    if(c[8] == 16 || c[9] == 16 || c[10] == 16 || c[11] == 16) {
      empty_row = 3;
    }
    // Check 4th row.
    if(c[12] == 16 || c[13] == 16 || c[14] == 16 || c[15] == 16) {
      empty_row = 4;
    }

    return (inversions + empty_row) % 2 == 0;
  }

  constexpr Configuration goal_configuration = {
    1,  2,  3,  4, //
    5,  6,  7,  8, //
    9,  10, 11, 12, //
    13, 14, 15, 16 //
  };

  static constexpr u64 goal_configuration_hash =
    hash_configuration(goal_configuration);

  [[nodiscard]] static std::optional<Configuration>
  generate_successor(Configuration_View const configuration,
                     i32 const successor) {
    i32 empty_index = 0;
    while(configuration[empty_index] != 16) {
      empty_index += 1;
    }

    switch(successor) {
      case 0: {
        // Check top row.
        if(empty_index >= 4) {
          Configuration node = copy_configuration(configuration);
          std::swap(node[empty_index], node[empty_index - 4]);
          return node;
        }
        return std::nullopt;
      }

      case 1: {
        i32 const empty_x = empty_index % 4;
        // Check rightmost row.
        if(empty_x < 3) {
          Configuration node = copy_configuration(configuration);
          std::swap(node[empty_index], node[empty_index + 1]);
          return node;
        }
        return std::nullopt;
      }

      case 2: {
        // Check bottom row.
        if(empty_index < 12) {
          Configuration node = copy_configuration(configuration);
          std::swap(node[empty_index], node[empty_index + 4]);
          return node;
        }
        return std::nullopt;
      }

      case 3: {
        i32 const empty_x = empty_index % 4;
        // Check leftmost row.
        if(empty_x > 0) {
          Configuration node = copy_configuration(configuration);
          std::swap(node[empty_index], node[empty_index - 1]);
          return node;
        }
        return std::nullopt;
      }

      default:
        return std::nullopt;
    }
  }

  struct IDAstar_Result {
    std::vector<Configuration> path;
    i64 depth = 0;
    i64 explored = 0;
    i32 result = 0;
  };

  [[nodiscard]] static IDAstar_Result
  IDAstar_search(Configuration const& starting_configuration,
                 heuristic_t const heuristic, i32 const f_cutoff) {
#define RETURN_VALUE(value)                  \
  {                                          \
    path.pop_back();                         \
    i32 const local_value = value;           \
    stack.pop();                             \
    Frame& parent_frame = stack.top();       \
    parent_frame.return_value = local_value; \
    parent_frame.returned = true;            \
    continue;                                \
  }

#define CALL(conf, hash, cost)             \
  {                                        \
    path.push_back(Path_Node{conf, hash}); \
    Frame frame;                           \
    frame.path_cost = cost;                \
    frame.created = true;                  \
    stack.push(frame);                     \
    continue;                              \
  }

    struct Path_Node {
      Configuration configuration;
      u64 id = 0;
    };

    struct Frame {
      i32 path_cost = 0;
      i32 successor = 0;
      i32 min = i32_largest_value;
      i32 return_value = 0;
      bool returned = false;
      bool created = false;
    };

    IDAstar_Result statistics;
    std::stack<Frame> stack;
    std::vector<Path_Node> path;
    {
      Frame frame;
      stack.push(frame);
      u64 const hash = hash_configuration(starting_configuration);
      path.push_back(Path_Node{starting_configuration, hash});
    }
    while(stack.size() > 0) {
      Frame& frame = stack.top();
      i32& path_cost = frame.path_cost;
      i32& successor = frame.successor;
      i32& min = frame.min;
      i32& return_value = frame.return_value;
      bool& returned = frame.returned;
      bool& created = frame.created;
      auto const& [configuration, configuration_hash] = path.back();

      if((i64)path.size() > statistics.depth) {
        statistics.depth = path.size();
        DEBUG_PRINT("Reached depth %lld\n", statistics.depth);
      }

      // We have returned from a "recursive call".
      if(returned) {
        if(return_value < min) {
          min = return_value;
        }
        returned = false;
      }

      // New configuration is being explored.
      if(created) {
        created = false;
        statistics.explored += 1;
        // Goal check should appear after f cutoff check, however, this way
        // we save a few iterations of the search.
        if(configuration_hash == goal_configuration_hash) {
          statistics.result = FOUND;
          for(auto const& [c, _]: path) {
            statistics.path.push_back(c);
          }
          return statistics;
        }

        i32 const f_value = path_cost + heuristic(configuration);
        if(f_value > f_cutoff) {
          RETURN_VALUE(f_value);
        }
      }

      // Search through successors.
      if(successor < 4) {
        std::optional<Configuration> successor_node =
          generate_successor(configuration, successor);
        successor += 1;
        if(!successor_node) {
          continue;
        }
        // Search through the path to verify that we have not been in
        // this configuration yet.
        u64 const successor_id = hash_configuration(successor_node.value());
        bool contains = false;
        for(auto const& [configuration, id]: path) {
          if(id == successor_id) {
            contains = true;
            break;
          }
        }

        if(!contains) {
          CALL(std::move(successor_node.value()), successor_id, path_cost + 1);
        } else {
          continue;
        }
      }

      // We do not want to pop the last frame with RETURN_VALUE since that
      // would also remove the last node from the path and we would be unable
      // to return the minimum.
      if(stack.size() > 1) {
        RETURN_VALUE(min);
      } else {
        statistics.result = min;
        return statistics;
      }
    }

    __builtin_unreachable();
  }

  Solution IDAstar_solver(IDAstar_Parameters const p) {
    i32 f_cutoff = p.initial_f_cutoff;
    if(f_cutoff <= 0) {
      f_cutoff = p.heuristic(p.starting_configuration);
    }
    // Cap max iterations at 1 million if not provided.
    i32 const max_iterations =
      p.max_iterations > 0 ? p.max_iterations : (1 << 20);
    Solution solution;
    solution.iterations = max_iterations;
    Configuration const configuration =
      copy_configuration(p.starting_configuration);
    for(i32 i = 0; i < max_iterations; i += 1) {
      IDAstar_Result const statistics =
        IDAstar_search(configuration, p.heuristic, f_cutoff);
      DEBUG_PRINT(
        "IDA* i %d; f_cutoff %d; result %d; depth %lld; explored "
        "%lld\n",
        i, f_cutoff, statistics.result, statistics.depth, statistics.explored);
      if(statistics.result == FOUND) {
        solution.found = true;
        solution.iterations = i;
        solution.explored = statistics.explored;
        solution.depth = statistics.depth;
        solution.path = std::move(statistics.path);
        break;
      } else {
        f_cutoff = statistics.result;
      }
    }
    return solution;
  }

  Solution Astar_solver(Astar_Parameters const p) {
    struct Node {
      Configuration configuration;
      Node* parent = nullptr;
      u64 id = 0;
      // Value of the path cost function.
      i32 g = 0;
      // Value of the heuristic function.
      i32 f = 0;
    };

    auto compare_priority = [](Node const* lhs, Node const* rhs) {
      bool const less = (lhs->g + lhs->f) < (rhs->g + rhs->f);
      bool const equal = (lhs->g + lhs->f) == (rhs->g + rhs->f);
      return less || (equal && lhs->f < rhs->f);
    };

    struct Compare_ID {
      [[nodiscard]] bool operator()(Node const* lhs, Node const* rhs) {
        return lhs->id < rhs->id;
      }

      [[nodiscard]] bool operator()(Node const* lhs, u64 const rhs) {
        return lhs->id < rhs;
      }

      [[nodiscard]] bool operator()(u64 const lhs, Node const* rhs) {
        return lhs < rhs->id;
      }
    };

    using heap_t = Heap<Node*, decltype(compare_priority), Compare_ID>;
    using heap_iterator = heap_t::iterator;
    heap_t frontier(compare_priority, Compare_ID{});
    // Populate frontier with the starting node.
    {
      Node* const node = new Node();
      node->configuration = copy_configuration(p.starting_configuration);
      node->id = hash_configuration(node->configuration);
      // There's no need to calculate the f value since the starting node is
      // the only one in the frontier and will be removed from it in the first
      // iteration.
      frontier.insert(node);
    }

    // Random number of buckets.
    std::map<u64, Node*> expanded;
    // expanded.max_load_factor(0.7f);
    Solution solution;
    while(frontier.size() > 0) {
      Node* const node = frontier.extract();
      if(node->g > solution.depth) {
        solution.depth = node->g;
        DEBUG_PRINT("A* depth %lld, explored %lld, %lld frontier\n",
                    solution.depth, (i64)expanded.size(), (i64)frontier.size());
      }

      if(node->id == goal_configuration_hash) {
        solution.found = true;
        Node* path_node = node;
        while(path_node != nullptr) {
          solution.path.push_back(std::move(path_node->configuration));
          path_node = path_node->parent;
        }
        std::reverse(solution.path.begin(), solution.path.end());
        break;
      }

      for(i32 i = 0; i < 4; i += 1) {
        std::optional<Configuration> result =
          generate_successor(node->configuration, i);
        if(!result) {
          continue;
        }

        Configuration& configuration = result.value();
        u64 const new_id = hash_configuration(configuration);
        i32 const new_g = node->g + 1;

        {
          heap_iterator const i = frontier.find(new_id);
          if(i != frontier.end()) {
            if(new_g < i->g) {
              i->parent = node;
              i->g = new_g;
              frontier.decrease(i);
            }
            continue;
          }
        }

        auto const iterator = expanded.find(new_id);
        bool const in_expanded = iterator != expanded.end();
        if(!in_expanded) {
          Node* const successor = new Node();
          successor->parent = node;
          successor->configuration = std::move(configuration);
          successor->id = new_id;
          successor->g = new_g;
          successor->f = p.heuristic(successor->configuration);
          expanded.emplace(successor->id, successor);
          frontier.insert(successor);
        }
      }
    }

    solution.explored = expanded.size();

    // TODO: Free all nodes. Use a bump allocator and scratch all memory.
    for(auto [_, node]: expanded) {
      delete node;
    }

    return solution;
  }

  Solution Bidirectional_Astar_solver(Bidirectional_Astar_Parameters const p) {
    struct Node {
      Configuration configuration;
      Node* parent = nullptr;
      u64 id = 0;
      // Value of the path cost function.
      i32 g = 0;
      // Value of the heuristic function.
      i32 f = 0;
    };

    auto compare_priority = [](Node const* lhs, Node const* rhs) {
      bool const less = (lhs->g + lhs->f) < (rhs->g + rhs->f);
      bool const equal = (lhs->g + lhs->f) == (rhs->g + rhs->f);
      return less || (equal && lhs->f < rhs->f);
    };

    struct Compare_ID {
      [[nodiscard]] bool operator()(Node const* lhs, Node const* rhs) {
        return lhs->id < rhs->id;
      }

      [[nodiscard]] bool operator()(Node const* lhs, u64 const rhs) {
        return lhs->id < rhs;
      }

      [[nodiscard]] bool operator()(u64 const lhs, Node const* rhs) {
        return lhs < rhs->id;
      }
    };

    Configuration const start_configuration =
      copy_configuration(p.starting_configuration);
    u64 const start_configuration_hash =
      hash_configuration(p.starting_configuration);

    using heap_t = Heap<Node*, decltype(compare_priority), Compare_ID>;
    using heap_iterator = heap_t::iterator;
    heap_t forward_frontier(compare_priority, Compare_ID{});
    heap_t backward_frontier(compare_priority, Compare_ID{});
    // Populate forward_frontier with the starting node.
    {
      Node* const node = new Node();
      node->configuration = start_configuration;
      node->id = start_configuration_hash;
      // There's no need to calculate the f value since the starting node is the
      // only one in the forward_frontier and will be removed from it in the
      // first iteration.
      forward_frontier.insert(node);
    }
    // Populate backward_frontier with the goal (starting) node.
    {
      Node* const node = new Node();
      node->configuration = goal_configuration;
      node->id = goal_configuration_hash;
      // There's no need to calculate the f value since the starting node is the
      // only one in the backward_frontier and will be removed from it in the
      // first iteration.
      backward_frontier.insert(node);
    }

    // Random number of buckets.
    std::map<u64, Node*> forward_expanded;
    std::map<u64, Node*> backward_expanded;
    Solution solution;
    bool forward = false;
    i64 counter = 0;
    i32 backward_depth = 0;
    i32 forward_depth = 0;
    while(forward_frontier.size() > 0 && backward_frontier.size() > 0) {
      if(counter == 0xFFFF) {
        forward = !forward;
        counter = 0;
      }

      counter += 1;

      if(forward) {
        Node* const node = forward_frontier.extract();
        // printf("Exploring forward configuration, g = %d, f = %d\n", node->g,
        //        node->f);
        // print(node->configuration);
        if(node->g > forward_depth) {
          forward_depth = node->g;
          DEBUG_PRINT(
            "A* forward depth %d, explored %lld, %lld forward_frontier\n",
            forward_depth, (i64)forward_expanded.size(),
            (i64)forward_frontier.size());
        }

        if(node->id == goal_configuration_hash) {
          solution.found = true;
          Node* path_node = node;
          while(path_node != nullptr) {
            solution.path.push_back(std::move(path_node->configuration));
            path_node = path_node->parent;
          }
          std::reverse(solution.path.begin(), solution.path.end());
          break;
        }

        for(i32 i = 0; i < 4; i += 1) {
          std::optional<Configuration> result =
            generate_successor(node->configuration, i);
          if(!result) {
            continue;
          }

          Configuration& configuration = result.value();
          u64 const new_id = hash_configuration(configuration);
          i32 const new_g = node->g + 1;

          {
            heap_iterator const i = forward_frontier.find(new_id);
            if(i != forward_frontier.end()) {
              if(new_g < i->g) {
                i->parent = node;
                i->g = new_g;
                forward_frontier.decrease(i);
              }
              continue;
            }
          }

          auto const forward_iterator = forward_expanded.find(new_id);
          bool const in_expanded = forward_iterator != forward_expanded.end();
          if(!in_expanded) {
            auto const iterator = backward_expanded.find(new_id);
            // Check whether we have met the opposide side search.
            if(iterator != backward_expanded.end()) {
              solution.found = true;
              // Reconstruct the solution by joining the forward and backward
              // parts. The forward part is reconstructed in reverse, while the
              // backward part in the correct order.
              Node* forward_node = node;
              while(forward_node != nullptr) {
                solution.path.push_back(std::move(forward_node->configuration));
                forward_node = forward_node->parent;
              }
              std::reverse(solution.path.begin(), solution.path.end());

              Node* backward_node = iterator->second;
              while(backward_node != nullptr) {
                solution.path.push_back(
                  std::move(backward_node->configuration));
                backward_node = backward_node->parent;
              }
              break;
            }

            Node* const successor = new Node();
            successor->parent = node;
            successor->configuration = std::move(configuration);
            successor->id = new_id;
            successor->g = new_g;
            successor->f = p.forward_heuristic(successor->configuration);
            forward_expanded.emplace(successor->id, successor);
            forward_frontier.insert(successor);
          }
        }

        if(solution.found) {
          break;
        }
      } else {
        Node* const node = backward_frontier.extract();
        // printf("Exploring backward configuration, g = %d, f = %d\n", node->g,
        //        node->f);
        // print(node->configuration);
        if(node->g > backward_depth) {
          backward_depth = node->g;
          DEBUG_PRINT(
            "A* backward depth %d, explored %lld, %lld backward_frontier\n",
            backward_depth, (i64)backward_expanded.size(),
            (i64)backward_frontier.size());
        }

        if(node->id == start_configuration_hash) {
          solution.found = true;
          Node* path_node = node;
          while(path_node != nullptr) {
            solution.path.push_back(std::move(path_node->configuration));
            path_node = path_node->parent;
          }
          break;
        }

        for(i32 i = 0; i < 4; i += 1) {
          std::optional<Configuration> result =
            generate_successor(node->configuration, i);
          if(!result) {
            continue;
          }

          Configuration& configuration = result.value();
          u64 const new_id = hash_configuration(configuration);
          i32 const new_g = node->g + 1;

          {
            heap_iterator const i = backward_frontier.find(new_id);
            if(i != backward_frontier.end()) {
              if(new_g < i->g) {
                i->parent = node;
                i->g = new_g;
                backward_frontier.decrease(i);
              }
              continue;
            }
          }

          auto const backward_iterator = backward_expanded.find(new_id);
          bool const in_expanded = backward_iterator != backward_expanded.end();
          if(!in_expanded) {
            auto const iterator = forward_expanded.find(new_id);
            // Check whether we have met the opposide side search.
            if(iterator != forward_expanded.end()) {
              solution.found = true;
              // Reconstruct the solution by joining the forward and backward
              // parts. The forward part is reconstructed in reverse, while the
              // backward part in the correct order.
              Node* forward_node = iterator->second;
              while(forward_node != nullptr) {
                solution.path.push_back(std::move(forward_node->configuration));
                forward_node = forward_node->parent;
              }
              std::reverse(solution.path.begin(), solution.path.end());

              Node* backward_node = node;
              while(backward_node != nullptr) {
                solution.path.push_back(
                  std::move(backward_node->configuration));
                backward_node = backward_node->parent;
              }
              break;
            }

            Node* const successor = new Node();
            successor->parent = node;
            successor->configuration = std::move(configuration);
            successor->id = new_id;
            successor->g = new_g;
            successor->f = p.backward_heuristic(successor->configuration,
                                                start_configuration);
            backward_expanded.emplace(successor->id, successor);
            backward_frontier.insert(successor);
          }
        }

        if(solution.found) {
          break;
        }
      }
    }

    solution.explored = forward_expanded.size() + backward_expanded.size();
    solution.depth =
      forward_depth > backward_depth ? forward_depth : backward_depth;

    // TODO: Free all nodes. Use a bump allocator and scratch all memory.
    for(auto [_, node]: forward_expanded) {
      delete node;
    }

    return solution;
  }
} // namespace puzzle15
