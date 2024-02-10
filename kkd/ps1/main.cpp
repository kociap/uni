#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/math/math.hpp>

using anton::char8;
using anton::f32;
using anton::i64;
using anton::u64;
using anton::u8;

using namespace anton::literals;

struct Counter {
  anton::Array<i64> occurrences;
  anton::Array<i64> conditional_occurrences;
  i64 total;

  [[nodiscard]] i64 get_occurrences(u8 symbol) const
  {
    return occurrences[symbol];
  }

  [[nodiscard]] i64 get_conditional_occurrences(u8 symbol, u8 prev) const
  {
    return conditional_occurrences[static_cast<i64>(symbol) + 256 * static_cast<i64>(prev)];
  }
};

[[nodiscard]] f32 calculate_entropy(Counter const& counter)
{
  f32 entropy = 0.0f;
  for(i64 const occurrence: counter.occurrences) {
    if(occurrence <= 0) {
      continue;
    }
    f32 const p = static_cast<f32>(occurrence) / static_cast<f32>(counter.total);
    entropy -= p * anton::math::log2(p);
  }
  return entropy;
}

[[nodiscard]] f32 calculate_conditional_entropy(Counter const& counter)
{
  f32 entropy = 0.0f;
  for(i64 x = 0; x < 256; x += 1) {
    f32 subsum = 0.0f;
    f32 const x_occurrences = static_cast<f32>(counter.occurrences[x]);
    for(i64 y = 0; y < 256; y += 1) {
      f32 const occurences = static_cast<f32>(counter.get_conditional_occurrences(y, x));
      if(occurences <= 0) {
        continue;
      }
      f32 const pyx =  occurences / x_occurrences;
      subsum += pyx * anton::math::log2(pyx);
    }
    f32 const px = x_occurrences / counter.total;
    entropy -= px * subsum;
  }
  return entropy;
}

int main(int argc, char** argv)
{
  anton::STDOUT_Stream stdout_stream;
  anton::STDERR_Stream stderr_stream;
  if(argc < 2) {
    stderr_stream.write(anton::format("{} <file>"_sv, argv[0]));
    return 1;
  }

  anton::String const filename(argv[1]);
  anton::fs::Input_File_Stream file(filename);
  if(file.error()) {
    stderr_stream.write(anton::format("error: could not open file '{}'"_sv, filename));
    return 1;
  }

  constexpr i64 page_size = 4096;
  anton::Array<u8> pagebuf_store(anton::reserve, page_size);
  u8* const pagebuf = pagebuf_store.data();
  Counter counter{.occurrences = anton::Array<i64>{256, 0},
                  .conditional_occurrences = anton::Array<i64>{256 * 256, 0}};
  while(!file.eof()) {
    i64 const read = file.read(pagebuf, page_size);
    counter.total += read;
    u8 const* beg = pagebuf;
    u8 const* const end = pagebuf + read;
    i64 last = 0;
    for(; beg != end; ++beg) {
      i64 value = *beg;
      counter.occurrences[value] += 1;
      counter.conditional_occurrences[last * 256 + value] += 1;
      last = value;
    }
  }

  f32 const entropy = calculate_entropy(counter);
  counter.occurrences[0] += 1;
  counter.total += 1;
  f32 const conditional_entropy = calculate_conditional_entropy(counter);
  stdout_stream.write(
    anton::format("{},{},{}\n"_sv, entropy, conditional_entropy, entropy - conditional_entropy));

  return 0;
}
