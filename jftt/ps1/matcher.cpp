#include <anton/array.hpp>
#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/math/math.hpp>
#include <anton/optional.hpp>
#include <anton/string.hpp>
#include <anton/string_view.hpp>
#include <anton/swap.hpp>

using anton::char8;
using anton::i64;
using anton::u64;
using anton::u8;

using namespace anton::literals;

constexpr i64 page_size = 4096;
constexpr i64 page_count = 16;

struct Match {
  i64 offset = 0;
};

struct KMP_Matcher {
public:
  KMP_Matcher(anton::String_View const pattern): pattern(pattern)
  {
    prefix = compute_KMP_prefix(pattern);
  }

  [[nodiscard]] anton::Optional<Match> match(char8 const* const begin,
                                             char8 const* const end)
  {
    char8 const* const p = pattern.data();
    for(char8 const* t = begin; t != end; ++t) {
      while(state > 0 && p[state] != *t) {
        state = prefix[state];
      }
      if(p[state] == *t) {
        state += 1;
      }
      if(state == pattern.size_bytes()) {
        i64 const offset = (t - begin) - pattern.size_bytes() + 1;
        return Match{offset};
      }
    }
    return anton::null_optional;
  }

  void clear_match_state()
  {
    state = 0;
  }

private:
  using KMP_Prefix = anton::Array<i64>;
  anton::String pattern;
  KMP_Prefix prefix;
  i64 state = 0;

  [[nodiscard]] KMP_Prefix compute_KMP_prefix(anton::String_View const pattern)
  {
    KMP_Prefix prefix(pattern.size_bytes(), 0);
    char8 const* const p = pattern.data();
    for(i64 q = 1, k = 0; q < pattern.size_bytes(); ++q) {
      while(k > 0 && p[k] != p[q]) {
        k = prefix[k];
      }
      if(p[k] == p[q]) {
        k += 1;
      }
      prefix[q] = k;
    }
    return prefix;
  }
};

struct FA_Matcher {
public:
  FA_Matcher(anton::String_View const pattern): pattern(pattern)
  {
    transition_table.resize(pattern.size_bytes() * 256);
    compute_transition_table(pattern);
  }

  [[nodiscard]] anton::Optional<Match> match(char8 const* const begin,
                                             char8 const* const end)
  {
    i64 const psize = pattern.size_bytes();
    for(char8 const* t = begin; t != end; ++t) {
      state = delta(state, *t);
      if(state == psize) {
        i64 const offset = (t - begin) - psize + 1;
        return Match{offset};
      }
    }
    return anton::null_optional;
  }

  void clear_match_state()
  {
    state = 0;
  }

private:
  using Transition_Table = anton::Array<i64>;
  anton::String pattern;
  Transition_Table transition_table;
  i64 state = 0;

  [[nodiscard]] i64& delta(i64 const state, i64 const c)
  {
    return transition_table[state * 256 + c];
  }

  void compute_transition_table(anton::String_View const pattern)
  {
    char8 const* const p = pattern.data();
    i64 const psize = pattern.size_bytes();
    anton::Array<char8> pattern_buffer(pattern.size_bytes(), 0);
    for(i64 q = 0; q < psize; q += 1) {
      for(i64 c = 0; c < 256; c += 1) {
        pattern_buffer[q] = c;
        anton::String_View const concat_pattern{pattern_buffer.data(),
                                                pattern_buffer.data() + q + 1};
        i64 k = anton::math::min(psize + 1, q + 2);
        while(true) {
          k -= 1;
          anton::String_View const prefix_pattern{p, p + k};
          bool const suffix = anton::ends_with(concat_pattern, prefix_pattern);
          if(suffix) {
            break;
          }
        }
        delta(q, c) = k;
      }

      pattern_buffer[q] = p[q];
    }
  }
};

struct Line_Limits {
  i64 start;
  i64 end;
};

[[nodiscard]] static Line_Limits find_line_limits(char8 const* const begin,
                                                  char8 const* const end,
                                                  char8 const* const current)
{
  char8 const* const rend = begin - 1;
  // Search backwards
  char8 const* reol = current - 1;
  for(; reol != rend && *reol != '\n'; --reol) {}
  // Search forward
  char8 const* eol = current + 1;
  for(; eol != end && *eol != '\n'; ++eol) {}
  Line_Limits limits{reol - begin + 1, eol - begin};
  return limits;
}

// fill_buffer
//
// Paramters:
//    buffer -
// carryover - number of bytes of the old content to carry over.
//
i64 fill_buffer(anton::Input_Stream& stream, u8* const buffer, i64 const size,
                i64 const carryover)
{
  // Copy carryover.
  memcpy(buffer, buffer + size - carryover, carryover);
  u8* const dst = buffer + carryover;
  // Align count down to page size.
  i64 const count = (size - carryover) & ~(page_size - 1);
  i64 const read = stream.read(dst, count);
  return read;
}

struct Line_Context {
  i64 line = 0;
  i64 column = 0;
};

void update_line_ctx(Line_Context& ctx, char8 const* begin,
                     char8 const* const end)
{
  constexpr char8 MSB_mask = 0x80;
  constexpr char8 continuation_mask = 0xC0;
  constexpr char8 ascii_pattern = 0x00;
  constexpr char8 continuation_pattern = 0x80;
  for(; begin != end; ++begin) {
    char8 const value = *begin;
    if(value == '\n') {
      ctx.column = 0;
      ctx.line += 1;
      continue;
    }

    bool const is_ascii = (value & MSB_mask) == ascii_pattern;
    bool const is_leading =
      ((value & continuation_mask) != continuation_pattern);
    ctx.column += is_ascii || is_leading;
  }
}

[[nodiscard]] u64 align_address(u64 const address, u64 const alignment)
{
  return (address + (alignment - 1)) & ~(alignment - 1);
}

void set_color_match(anton::Output_Stream& stream)
{
  stream.write("\033[38;5;9m"_sv);
}

void clear_color(anton::Output_Stream& stream)
{
  stream.write("\033[0m"_sv);
}

int main(int argc, char** argv)
{
  anton::STDOUT_Stream stdout_stream;
  anton::STDERR_Stream stderr_stream;
  if(argc < 2) {
    stderr_stream.write(format("{} <pattern> <file>..."_sv,
                               anton::fs::get_filename_no_extension(argv[0])));
    return -1;
  }

  anton::String_View const pattern(argv[1]);
  if(pattern.size_bytes() > page_size) {
    stderr_stream.write(format("pattern overlong (max {}, actual {})"_sv,
                               page_size, pattern.size_bytes()));
    return -1;
  }

#if USE_FA_MATCHER
  FA_Matcher matcher(pattern);
#else
  KMP_Matcher matcher(pattern);
#endif
  anton::Array<u8> buffer(page_count * page_size);
  for(i64 i = 2; i < argc; ++i) {
    anton::String const filename(argv[i]);
    anton::fs::Input_File_Stream file(filename);
    if(!file) {
      stderr_stream.write(
        format("could not open file '{}' for reading\n"_sv, filename));
      continue;
    }

    i64 const size = align_address(anton::fs::file_size(filename), page_size);
    buffer.ensure_capacity(size);
    i64 const read = fill_buffer(file, buffer.data(), buffer.capacity(), 0);
    char8 const* const bufbeg = (char8*)buffer.data();
    char8 const* curbuf = bufbeg;
    char8 const* const bufend = bufbeg + read;
    char8 const* last_match = bufbeg;
    Line_Context line_ctx;
    while(true) {
      anton::Optional<Match> match = matcher.match(curbuf, bufend);
      if(!match) {
        break;
      }

      char8 const* match_begin = curbuf + match->offset;
      char8 const* match_end = match_begin + pattern.size_bytes();
      update_line_ctx(line_ctx, last_match, match_begin);
      Line_Limits const limits = find_line_limits(bufbeg, bufend, match_begin);
      anton::String_View const source(bufbeg + limits.start,
                                      bufbeg + limits.end);
      anton::String location = format("{}:{}:{}: "_sv, filename,
                                      line_ctx.line + 1, line_ctx.column + 1);
      char8 const* line_begin = bufbeg + limits.start;
      char8 const* line_end = bufbeg + limits.end;
      stdout_stream.write(location);
      stdout_stream.write(anton::String_View{line_begin, match_begin});
      set_color_match(stdout_stream);
      stdout_stream.write(anton::String_View{match_begin, match_end});
      clear_color(stdout_stream);
      stdout_stream.write(anton::String_View{match_end, line_end});
      stdout_stream.write("\n"_sv);

      last_match = match_begin;
      curbuf += match->offset + 1;
    }
  }
  return 0;
}
