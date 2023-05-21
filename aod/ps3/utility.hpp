#pragma once

#include <chrono>
#include <stdio.h>

struct File_Guard {
private:
  FILE* file = nullptr;

public:
  File_Guard(FILE* file): file(file) {}

  ~File_Guard() {
    if(file != nullptr) {
      fclose(file);
    }
  }

  void release() {
    file = nullptr;
  }
};

struct Timer {
private:
  std::chrono::high_resolution_clock::time_point start_time;

public:
  void start() {
    start_time = std::chrono::high_resolution_clock::now();
  }

  [[nodiscard]] i64 end_ns() {
    auto const end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time -
                                                                start_time)
      .count();
  }

  [[nodiscard]] i64 end_us() {
    auto const end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                 start_time)
      .count();
  }

  [[nodiscard]] i64 end_ms() {
    auto const end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                 start_time)
      .count();
  }
};
