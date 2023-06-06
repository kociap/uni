
#include <algorithm>
#include <types.hpp>

#include <map>
#include <optional>
#include <random>
#include <stdio.h>
#include <vector>

static i32 generate_random() {
  static std::random_device rd;
  static std::mt19937 g(rd());
  static std::uniform_int_distribution<i32> d(8, 64);
  return d(g);
}

constexpr u32 signal_jam = 0xF0000000;
constexpr u32 signal_noise = 0x0FFFFFFF;

class Medium {
private:
  using wire_t = std::vector<u32>;

  std::map<i32, wire_t> wires;
  i32 size;

public:
  Medium(i32 size): size(size) {}

  void connect(i32 const id) {
    wires[id] = wire_t(size);
  }

  void transmit(i32 const at, u32 const byte) {
    wires[at][at] = byte;
  }

  u32 read(i32 const at) {
    u32 value = 0;
    for(auto const& [id, wire]: wires) {
      value |= wire[at];
    }
    return value;
  }

  void tick() {
    for(auto& [id, wire]: wires) {
      propagate_left_from(wire, id);
      propagate_right_from(wire, id);
      wire[id] = 0;
    }

    auto count_signals = [*this](i32 const index) {
      i32 count = 0;
      for(auto const& [id, wire]: wires) {
        if(wire[index] != 0) {
          count += 1;
        }
      }
      return count;
    };

    bool collision = false;
    for(i32 i = 0; i < size; i += 1) {
      i32 const signals = count_signals(i);
      if(signals > 1) {
        collision = true;
        break;
      }
    }

    if(!collision) {
      return;
    }

    // Change overlappping bytes to noise.
    for(i32 index = 0; index < size; index += 1) {
      i32 const signals = count_signals(index);
      if(signals <= 1) {
        continue;
      }

      for(auto& [id, wire]: wires) {
        if(wire[index] != 0) {
          wire[index] |= signal_noise;
        }
      }
    }
  }

  void print() {
    // for(auto& [id, wire]: wires) {
    //   printf("wire %d: ", id);
    //   for(u32 v: wire) {
    //     printf("%X ", v);
    //   }
    //   printf("\n");
    // }

    for(i32 index = 0; index < size; index += 1) {
      u32 value = 0;
      for(auto& [id, wire]: wires) {
        u32 const signal = wire[index];
        if(signal != 0) {
          value |= signal;
        }
      }
      printf("%X ", value);
    }
    printf("\n");
  }

private:
  void propagate_left_from(wire_t& wire, i32 const at) {
    i32 carry = wire[at];
    for(i32 i = at; i > 0; i -= 1) {
      i32 next = wire[i - 1];
      wire[i - 1] = carry;
      carry = next;
    }
  }

  void propagate_right_from(wire_t& wire, i32 const at) {
    i32 carry = wire[at];
    for(i32 i = at; i < wire.size() - 1; i += 1) {
      i32 next = wire[i + 1];
      wire[i + 1] = carry;
      carry = next;
    }
  }
};

class Station {
private:
  static constexpr i32 max_attempts = 10;

  Medium* medium = nullptr;
  u32 id = -1;
  i32 wait_time = 0;
  i32 attempts = 0;
  i32 length = 0;

  struct Transmission {
    i32 len = 0;
    i32 dst = -1;
  };

  Transmission transmission;
  bool began_transmission = false;

public:
  Station(i32 id, i32 length): id(id), length(length) {}

  void connect(Medium* const medium) {
    this->medium = medium;
    medium->connect(id);
  }

  void transmit(u32 const destination) {
    transmission.len = 2 * length;
    transmission.dst = destination;
  }

  void tick() {
    if(transmission.dst == -1) {
      u32 const value = medium->read(id);
      if(value == id) {
        printf("station %d receiving transmission\n", id);
      }
    } else {
      if(wait_time > 0) {
        wait_time -= 1;
        return;
      }

      if(!began_transmission) {
        attempt_transmit();
        return;
      }

      u32 const value = medium->read(id);
      if((value & signal_noise) == signal_noise) {
        backoff();
        medium->transmit(id, signal_jam);
      } else {
        medium->transmit(id, transmission.dst);
        transmission.len -= 1;
      }

      if(transmission.len == 0) {
        reset();
      }
    }
  }

  void backoff() {
    wait_time = generate_random();
    attempts += 1;
    began_transmission = false;
    transmission.len = length * 2;
    medium->transmit(id, signal_jam);
    if(attempts >= max_attempts) {
      reset();
    }
  }

private:
  void reset() {
    transmission.dst = -1;
    began_transmission = false;
    attempts = 0;
    wait_time = 0;
  }

  void attempt_transmit() {
    if(medium->read(id) == 0) {
      began_transmission = true;
      medium->transmit(id, transmission.dst);
      return;
    } else {
      wait_time = generate_random();
      attempts += 1;
      began_transmission = false;
      if(attempts >= max_attempts) {
        reset();
      }
    }
  }
};

int main() {
  constexpr i32 wire_length = 50;
  Medium medium(wire_length);
  std::vector<Station> stations;
  for(i32 i = 1; i < wire_length; i += 1) {
    Station& station = stations.emplace_back(i, wire_length);
    station.connect(&medium);
  }

  stations[10].transmit(5);
  stations[26].transmit(3);
  stations[27].transmit(1);
  stations[31].transmit(8);
  stations[4].transmit(30);
  stations[40].transmit(41);
  stations[41].transmit(1);
  stations[7].transmit(2);

  for(i32 tick = 0; tick < 512; tick += 1) {
    medium.tick();

    for(Station& station: stations) {
      station.tick();
    }

    medium.print();
  }

  return 0;
}
