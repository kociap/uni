#include <heuristic.hpp>

#include <queue>
#include <unordered_map>

#include <debug.hpp>

namespace puzzle8 {
  i32 heuristic_manhattan_distance(Configuration_View const configuration) {
    i32 result = 0;
    for(i32 index = 0; Configuration_Entry const value: configuration) {
      i32 const index_x = index % 3;
      i32 const index_y = index / 3;
      i32 const value_x = (value - 1) % 3;
      i32 const value_y = (value - 1) / 3;
      result += abs(index_x - value_x) + abs(index_y - value_y);
    }
    return result;
  }
} // namespace puzzle8

namespace puzzle15 {
  // remap_to_standard
  // Remap the start configuration into a space in which goal configuration is
  // the standard configuration.
  //
  static Configuration remap_to_standard(Configuration_View const start,
                                         Configuration_View const goal) {
    Configuration r_start;
    Configuration map;
    for(i32 index = 1; Configuration_Entry const g: goal) {
      map[g - 1] = index;
      index += 1;
    }

    for(i32 index = 0; Configuration_Entry const s: start) {
      r_start[index] = map[s - 1];
      index += 1;
    }

    return r_start;
  }

  i32 heuristic_manhattan_distance(Configuration_View const start) {
    i32 result = 0;
    for(i32 index = 0; Configuration_Entry const value: start) {
      if(value == 16) {
        continue;
      }

      i32 const index_x = index % 4;
      i32 const index_y = index / 4;
      i32 const value_x = (value - 1) % 4;
      i32 const value_y = (value - 1) / 4;
      result += abs(index_x - value_x) + abs(index_y - value_y);
      index += 1;
    }
    return result;
  }

  i32 heuristic_manhattan_distance_generic(Configuration_View const start,
                                           Configuration_View const goal) {
    Configuration const c = remap_to_standard(start, goal);
    return heuristic_manhattan_distance(c);
  }

  static i32 count_conflicts(Configuration_View const start) {
    i32 sum = 0;
    for(i32 row_offset = 0; row_offset < 16; row_offset += 4) {
      for(i32 i = 0; i < 4; i += 1) {
        Configuration_Entry const e1 = start[row_offset + i] - 1;
        // Entry is the empty square or does not belong in this row.
        if(e1 == 16 || e1 < row_offset || e1 >= row_offset + 4) {
          continue;
        }

        for(i32 j = i + 1; j < 4; j += 1) {
          Configuration_Entry const e2 = start[row_offset + j] - 1;
          // Entry is the empty square or does not belong in this row.
          if(e2 == 16 || e2 < row_offset || e2 >= row_offset + 4) {
            continue;
          }

          if(e1 > e2) {
            sum += 2;
          }
        }
      }
    }
    return sum;
  }

  i32 heuristic_linear_conflict(Configuration_View const start) {
    i32 sum = heuristic_manhattan_distance(start);
    sum += count_conflicts(start);
    // Transpose the configuration to count column conflicts.
    Configuration t = copy_configuration(start);
    for(i32 i = 0; i < 4; i += 1) {
      for(i32 j = i + 1; j < 4; j += 1) {
        std::swap(t[i * 4 + j], t[j * 4 + i]);
      }
    }
    sum += count_conflicts(t);
    return sum;
  }

  i32 heuristic_linear_conflict_generic(Configuration_View const start,
                                        Configuration_View const goal) {
    Configuration const c = remap_to_standard(start, goal);
    return heuristic_linear_conflict(c);
  }

  i32 heuristic_inversions(Configuration_View const start) {
    i32 inversions = 0;
    for(i32 i = 0; i < 16; i += 1) {
      for(i32 j = i + 1; j < 16; j += 1) {
        if(start[j] < start[i]) {
          inversions += 1;
        }
      }
    }
    // Vertical inversions (top-bottom left-right).
    // i32 const area_less_one = dimensions.width * dimensions.height - 1;
    // for(i32 i = 0, i_max = 0; i_max <= area_less_one; i_max += 1) {
    //     for(i32 j = i, j_max = i_max; j_max <= area_less_one; j_max += 1) {
    //         inversions += start[i] > start[j];
    //         j += dimensions.width;
    //         if(j > area_less_one) {
    //             j -= area_less_one;
    //         }
    //     }
    //     i += dimensions.width;
    //     if(i > area_less_one) {
    //         i -= area_less_one;
    //     }
    // }
    return inversions;
  }

  using block_database = std::unordered_map<u64, i32>;

  static void build_block_database(block_database& db,
                                   Configuration block_configuration) {
    struct Pattern {
      Configuration configuration;
      u64 id;
      i32 cost;
    };

    std::queue<Pattern> pattern_queue;
    {
      u64 const hash = hash_configuration(block_configuration);
      db.emplace(hash, 0);
      pattern_queue.push(Pattern{block_configuration, hash, 0});
    }
    while(pattern_queue.size() > 0) {
      Pattern pattern = pattern_queue.front();
      pattern_queue.pop();

      for(i32 index = -1; Configuration_Entry const v: pattern.configuration) {
        index += 1;
        if(v == 0) {
          continue;
        }

        i32 const new_cost = pattern.cost + 1;
        i32 const x = index % 4;
        i32 const y = index / 4;
        // Attempt move left.
        if(x > 0 && pattern.configuration[index - 1] == 0) {
          Configuration configuration(pattern.configuration);
          std::swap(configuration[index], configuration[index - 1]);
          u64 const hash = hash_configuration(configuration);
          if(db.find(hash) == db.end()) {
            db.emplace(hash, new_cost);
            pattern_queue.push(Pattern{configuration, hash, new_cost});
          }
        }
        // Attempt move right.
        if(x < 3 && pattern.configuration[index + 1 == 0]) {
          Configuration configuration(pattern.configuration);
          std::swap(configuration[index], configuration[index + 1]);
          u64 const hash = hash_configuration(configuration);
          if(db.find(hash) == db.end()) {
            db.emplace(hash, new_cost);
            pattern_queue.push(Pattern{configuration, hash, new_cost});
          }
        }
        // Attempt move up.
        if(y > 0 && pattern.configuration[index - 4] == 0) {
          Configuration configuration(pattern.configuration);
          std::swap(configuration[index], configuration[index - 4]);
          u64 const hash = hash_configuration(configuration);
          if(db.find(hash) == db.end()) {
            db.emplace(hash, new_cost);
            pattern_queue.push(Pattern{configuration, hash, new_cost});
          }
        }
        // Attempt move down.
        if(y < 3 && pattern.configuration[index + 4] == 0) {
          Configuration configuration(pattern.configuration);
          std::swap(configuration[index], configuration[index + 4]);
          u64 const hash = hash_configuration(configuration);
          if(db.find(hash) == db.end()) {
            db.emplace(hash, new_cost);
            pattern_queue.push(Pattern{configuration, hash, new_cost});
          }
        }
      }
    }
  }

  // We divide the board into three blocks of 5 tiles.
  //
  //     ###################$$$$$$
  //     #     |     |     #     $
  //     #  1  |  2  |  3  #  4  $
  //     #     |     |     #     $
  //     #-----+-----#######-----$
  //     #     |     #     |     $
  //     #  5  |  6  #  7  |  8  $
  //     #     |     #     |     $
  //     #############-----+-----$
  //     @     |     @     |     $
  //     @  9  | 10  @ 11  | 12  $
  //     @     |     @     |     $
  //     @-----+-----@@@@@@@$$$$$$
  //     @     |     |     @     |
  //     @ 13  | 14  | 15  @ 16  |
  //     @     |     |     @     |
  //     @@@@@@@@@@@@@@@@@@@-----+
  //
  // And generate patterns for each group into a separate set. We will sum the
  // values of the 3 blocks when we calculate the heuristic.

  static block_database block1_database;
  static block_database block2_database;
  static block_database block3_database;

  void build_pattern_database() {
    DEBUG_PRINT("Building block 1\n");
    block1_database.max_load_factor(0.7f);
    Configuration block1_configuration{1, 2, 3, 0, //
                                       5, 6, 0, 0, //
                                       0, 0, 0, 0, //
                                       0, 0, 0, 0};
    build_block_database(block1_database, block1_configuration);
    DEBUG_PRINT("Block database 1 size %llu\n", (i64)block1_database.size());
    DEBUG_PRINT("Building block 2\n");
    block2_database.max_load_factor(0.7f);
    Configuration block2_configuration{0, 0, 0,  4, //
                                       0, 0, 7,  8, //
                                       0, 0, 11, 12, //
                                       0, 0, 0,  0};
    build_block_database(block2_database, block2_configuration);
    DEBUG_PRINT("Block database 2 size %llu\n", (i64)block2_database.size());
    DEBUG_PRINT("Building block 3\n");
    block3_database.max_load_factor(0.7f);
    Configuration block3_configuration{0,  0,  0,  0, //
                                       0,  0,  0,  0, //
                                       9,  10, 0,  0, //
                                       13, 14, 15, 0};
    build_block_database(block3_database, block3_configuration);
    DEBUG_PRINT("Block database 3 size %llu\n", (i64)block3_database.size());
  }

  i32 heuristic_pattern_database(Configuration_View const start) {
    Configuration block1 = {};
    Configuration block2 = {};
    Configuration block3 = {};
    for(i32 index = 0; Configuration_Entry const v: start) {
      switch(v) {
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
          block1[index] = v;
          break;
        case 4:
        case 7:
        case 8:
        case 11:
        case 12:
          block2[index] = v;
          break;
        case 9:
        case 10:
        case 13:
        case 14:
        case 15:
          block3[index] = v;
          break;
        default:
          break;
      }
      index += 1;
    }
    u64 const hash1 = hash_configuration(block1);
    u64 const hash2 = hash_configuration(block2);
    u64 const hash3 = hash_configuration(block3);
    return block1_database[hash1] + block2_database[hash2] +
           block3_database[hash3];
  }

  i32 heuristic_pattern_database_generic(Configuration_View const start,
                                         Configuration_View const goal) {
    Configuration const c = remap_to_standard(start, goal);
    return heuristic_pattern_database(c);
  }
} // namespace puzzle15
