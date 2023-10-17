#include <AAC.hpp>

#include <anton/intrinsics.hpp>
#include <anton/optional.hpp>
#include <anton/pair.hpp>

#include <bitio.hpp>

constexpr u32 code_bits = 33;
constexpr u32 frequency_bits = 31;
constexpr u64 max_code = 0x1FFFFFFFF;
constexpr u64 max_frequency = static_cast<u64>(1) << (frequency_bits - 1);
constexpr u64 half_range = static_cast<u64>(1) << (code_bits - 1);
constexpr u64 one_fourth_range = half_range >> 1;
constexpr u64 three_fourths_range =
  one_fourth_range + one_fourth_range + one_fourth_range;
constexpr u32 symbol_count = 257;
constexpr u32 EOF_symbol = 256;

struct Interval {
  u64 low;
  u64 high;
  // denominator
  u64 denom;
};

struct Model {
public:
  void initialise()
  {
    // Initialise the model.
    cumulative_frequencies[0] = 0;
    for(u32 i = 0; i < symbol_count; i += 1) {
      frequencies[i] = 1;
      cumulative_frequencies[i + 1] = cumulative_frequencies[i] + 1;
    }

    frequencies[symbol_count] = 0;
  }

  void update(u32 const symbol)
  {
    // if(cumulative_frequencies[symbol_count] >= max_frequency) { return;
    // }

    frequencies[symbol] += 1;
    for(u32 i = symbol + 1; i < symbol_count + 1; i += 1) {
      cumulative_frequencies[i] += 1;
    }

    if(cumulative_frequencies[symbol_count] >= max_frequency) {
      // Renormalise the frequencies.
      cumulative_frequencies[0] = 0;
      for(u32 i = 0; i < symbol_count; i += 1) {
        frequencies[i] = (frequencies[i] + 1) / 2;
        cumulative_frequencies[i + 1] =
          cumulative_frequencies[i] + frequencies[i];
      }
    }
  }

  [[nodiscard]] Interval get_probability_interval(u32 const symbol) const
  {
    return {.low = cumulative_frequencies[symbol],
            .high = cumulative_frequencies[symbol + 1],
            .denom = cumulative_frequencies[symbol_count]};
  }

  [[nodiscard]] u64 get_denom() const
  {
    return cumulative_frequencies[symbol_count];
  }

  [[nodiscard]] anton::Pair<u32, Interval> get_symbol(u64 const scaled_value)
  {
    for(u32 i = 0; i < symbol_count; i += 1) {
      if(scaled_value < cumulative_frequencies[i + 1]) {
        u32 const symbol = i;
        return {symbol, get_probability_interval(symbol)};
      }
    }

    ANTON_UNREACHABLE();
  }

private:
  u64 cumulative_frequencies[symbol_count + 1];
  u64 frequencies[symbol_count + 1];
};

struct Encode_Context {
public:
  void start_encoding(anton::Output_Stream* stream)
  {
    writer.reset(stream);

    follow_bits = 0;
    low = 0;
    high = max_code;

    model.initialise();
  }

  void finish_encoding()
  {
    encode(EOF_symbol);
    // We write additional bits to ensure the last symbol will be properly
    // decoded.
    follow_bits += 1;
    if(low < one_fourth_range) {
      output_bit(0);
    } else {
      output_bit(1);
    }

    writer.flush();
  }

  void encode(u32 const symbol)
  {
    u64 const range = high - low + 1;
    Interval const interval = model.get_probability_interval(symbol);
    high = low + (range * interval.high) / interval.denom - 1;
    low = low + (range * interval.low) / interval.denom;

    // Renormalise the range.
    while(true) {
      if(high < half_range) {
        output_bit(0);
      } else if(low >= half_range) {
        output_bit(1);
      } else if(low >= one_fourth_range && high < three_fourths_range) {
        follow_bits += 1;
        low -= one_fourth_range;
        high -= one_fourth_range;
      } else {
        break;
      }
      high <<= 1;
      high += 1;
      low <<= 1;
      high &= max_code;
      low &= max_code;
    }

    model.update(symbol);
  }

  [[nodiscard]] i64 get_total_written()
  {
    return writer.get_written();
  }

private:
  Model model;
  Bit_Writer writer;
  u64 follow_bits = 0;
  u64 low = 0;
  u64 high = 0;

  void output_bit(u8 const bit)
  {
    writer.write(bit);
    u8 const flipped = 1 - bit;
    while(follow_bits > 0) {
      follow_bits -= 1;
      writer.write(flipped);
    }
  }
};

struct Decode_Context {
  void start_decoding(anton::Input_Stream* input, anton::Output_Stream* output)
  {
    this->output = output;
    reader.reset(input);
    model.initialise();
  }

  void decode()
  {
    u64 high = max_code;
    u64 low = 0;
    u64 value = 0;

    // Prime value with the first code_bits bits of input. TODO: If there are
    // fewer than code_bits bits in the input stream, report an error or sth.
    for(u32 i = 0; i < code_bits; i += 1) {
      value <<= 1;
      value += reader.read();
    }

    while(true) {
      u64 const range = high - low + 1;
      u64 const denom = model.get_denom();
      u64 const scaled_value = ((value - low + 1) * denom - 1) / range;

      auto [symbol, interval] = model.get_symbol(scaled_value);
      if(symbol == EOF_symbol) {
        return;
      }

      output->put(symbol);

      high = low + (range * interval.high) / interval.denom - 1;
      low = low + (range * interval.low) / interval.denom;

      // Renormalise the range.
      while(true) {
        if(high < half_range) {
          // Nothing.
        } else if(low >= half_range) {
          value -= half_range;
          low -= half_range;
          high -= half_range;
        } else if(low >= one_fourth_range && high < three_fourths_range) {
          value -= one_fourth_range;
          low -= one_fourth_range;
          high -= one_fourth_range;
        } else {
          break;
        }
        high <<= 1;
        high += 1;
        low <<= 1;
        value <<= 1;
        value += reader.read();
      }

      model.update(symbol);
    }
  }

private:
  Model model;
  Bit_Reader reader;
  anton::Output_Stream* output = nullptr;
};

Encoding_Statistics encode_AAC(anton::fs::Input_File_Stream& input,
                               anton::Output_Stream& output)
{
  Encoding_Statistics statistics = {};
  Encode_Context ctx;
  ctx.start_encoding(&output);
  while(true) {
    u8 const symbol = input.get();
    if(input.eof()) {
      break;
    }

    ctx.encode(symbol);
    statistics.frequencies[symbol] += 1;
    statistics.read += 1;
  }
  ctx.finish_encoding();
  statistics.written += ctx.get_total_written();
  return statistics;
}

void decode_AAC(anton::fs::Input_File_Stream& input,
                anton::Output_Stream& output)
{
  Decode_Context ctx;
  ctx.start_decoding(&input, &output);
  ctx.decode();
}
