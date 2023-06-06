#pragma once

#include <types.hpp>

#include <chrono>

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

template<typename T>
T max(T a, T b) {
  return a > b ? a : b;
}

template<typename T>
T min(T a, T b) {
  return a < b ? a : b;
}
