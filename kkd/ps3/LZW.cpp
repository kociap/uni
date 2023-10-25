#include <LZW.hpp>

#include <anton/flat_hash_map.hpp>
#include <anton/string7.hpp>

constexpr u64 EOF_symbol = 256;

Encoding_Statistics compress_LZW(anton::fs::Input_File_Stream& input,
                                 anton::Output_Stream& output,
                                 universal_encode_fn encode)
{
  anton::Flat_Hash_Map<anton::String7, u64> dictionary;
  for(u64 i = 0; i < 256; ++i) {
    char8 c = i;
    anton::String7 symbol(&c, &c + 1);
    dictionary.emplace(symbol, i);
  }

  Encoding_Statistics statistics = {};

  Bit_Writer writer;
  writer.reset(&output);

  u64 current_code = 257;
  anton::String7 prefix;
  while(true) {
    char8 const next_byte = input.get();
    if(input.eof()) {
      break;
    }

    statistics.frequencies[(u8)next_byte] += 1;
    statistics.read += 1;

    anton::String7 prefix_symbol = prefix;
    prefix_symbol.append(next_byte);
    if(dictionary.find(prefix_symbol) != dictionary.end()) {
      prefix = prefix_symbol;
    } else {
      u64 const value = dictionary.find(prefix)->value;
      encode(writer, value);
      dictionary.emplace(prefix_symbol, current_code);
      current_code += 1;
      prefix = anton::String7(&next_byte, &next_byte + 1);
    }
  }

  if(prefix.size() > 0) {
    u64 const value = dictionary.find(prefix)->value;
    encode(writer, value);
  }

  encode(writer, EOF_symbol);
  writer.flush();
  statistics.written = writer.get_written();
  return statistics;
}

void decompress_LZW(anton::fs::Input_File_Stream& input,
                    anton::Output_Stream& output, universal_decode_fn decode)
{
  anton::Flat_Hash_Map<u64, anton::String7> dictionary;
  for(u64 i = 0; i < 256; ++i) {
    char8 c = i;
    anton::String7 symbol(&c, &c + 1);
    dictionary.emplace(i, symbol);
  }

  u64 current_code = 257;
  Bit_Reader reader;
  reader.reset(&input);
  anton::String7 prefix;
  {
    u64 const code = decode(reader);
    if(code == EOF_symbol) {
      return;
    } else {
      output.put(code);
    }
    prefix.append(code);
  }

  while(true) {
    u64 const code = decode(reader);
    if(code == EOF_symbol) {
      return;
    }

    if(input.eof()) {
      return;
    }

    auto iter = dictionary.find(code);
    if(iter != dictionary.end()) {
      anton::String7 entry = iter->value;
      output.write(entry.begin(), entry.size());
      prefix.append(entry[0]);
      dictionary.emplace(current_code, prefix);
      current_code += 1;
      prefix = ANTON_MOV(entry);
    } else {
      prefix.append(prefix[0]);
      output.write(prefix.begin(), prefix.size());
      dictionary.emplace(current_code, prefix);
      current_code += 1;
    }
  }
}
