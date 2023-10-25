#include <universal.hpp>

#include <anton/math/math.hpp>

#include <bitio.hpp>

void encode_elias_omega(Bit_Writer& writer, u64 N)
{
  // 256 bits is sufficient to fit the largest value representable by u64.
  Bit_Stack<256> stack;
  while(N > 1) {
    u64 const bits = (64 - anton::math::clz(N));
    for(u64 i = 0; i < bits; i += 1) {
      stack.push((N >> i) & 0x1);
    }
    N = bits - 1;
  }

  while(stack.size() > 0) {
    writer.write(stack.pop());
  }
  writer.write(0);
}

u64 decode_elias_omega(Bit_Reader& reader)
{
  u64 N = 1;
  while(true) {
    u8 const next = reader.read();
    if(next == 0) {
      break;
    }

    u64 const length = N;
    N = 1;
    for(u64 i = 0; i < length; ++i) {
      u8 const bit = reader.read();
      N <<= 1;
      N |= bit;
    }
  }
  return N;
}

void encode_elias_delta(Bit_Writer& writer, u64 const in_X)
{
  // Add 1 to X so that we can code 0. Maximum u64 is improbable, hence this
  // is not an issue.
  u64 const X = in_X + 1;
  u64 const N = 64 - anton::math::clz(X) - 1;
  u64 const L = 64 - anton::math::clz(N + 1) - 1;
  // L zeroes.
  for(u64 i = 0; i < L; i += 1) {
    writer.write(0x0);
  }
  // L + 1 bits of the binary representation of N + 1.
  for(u64 i = 0; i < L + 1; i += 1) {
    u8 const bit = ((N + 1) >> (L - i)) & 0x1;
    writer.write(bit);
  }
  // The last N bits of X.
  for(u64 i = 0; i < N; i += 1) {
    u8 const bit = ((X) >> (N - i - 1)) & 0x1;
    writer.write(bit);
  }
}

u64 decode_elias_delta(Bit_Reader& reader)
{
  u64 L = 0;
  while(reader.read() == 0x0) {
    L += 1;
  }
  u64 N = 1;
  // Read the remaining bits of N + 1.
  for(u64 i = 0; i < L; i += 1) {
    u8 const bit = reader.read();
    N = (N << 1) | bit;
  }
  N -= 1;
  u64 X = 1;
  // Read remaining N bits of X.
  for(u64 i = 0; i < N; i += 1) {
    u8 const bit = reader.read();
    X = (X << 1) | bit;
  }
  return X - 1;
}

void encode_elias_gamma(Bit_Writer& writer, u64 const in_X)
{
  // Add 1 to X so that we can code 0. Maximum u64 is improbable, hence this
  // is not an issue.
  u64 const X = in_X + 1;
  u64 const N = 64 - anton::math::clz(X) - 1;
  // N zeroes.
  for(u64 i = 0; i < N; i += 1) {
    writer.write(0x0);
  }
  // The N + 1 bits of X.
  for(u64 i = 0; i < N + 1; i += 1) {
    u8 const bit = ((X) >> (N - i)) & 0x1;
    writer.write(bit);
  }
}

u64 decode_elias_gamma(Bit_Reader& reader)
{
  u64 N = 0;
  while(reader.read() == 0x0) {
    N += 1;
  }
  u64 X = 1;
  // Read remaining N bits of X.
  for(u64 i = 0; i < N; i += 1) {
    u8 const bit = reader.read();
    X = (X << 1) | bit;
  }
  return X - 1;
}

static constexpr u64 fibonacci[] = {
  1ULL,
  1ULL,
  2ULL,
  3ULL,
  5ULL,
  8ULL,
  13ULL,
  21ULL,
  34ULL,
  55ULL,
  89ULL,
  144ULL,
  233ULL,
  377ULL,
  610ULL,
  987ULL,
  1597ULL,
  2584ULL,
  4181ULL,
  6765ULL,
  10946ULL,
  17711ULL,
  28657ULL,
  46368ULL,
  75025ULL,
  121393ULL,
  196418ULL,
  317811ULL,
  514229ULL,
  832040ULL,
  1346269ULL,
  2178309ULL,
  3524578ULL,
  5702887ULL,
  9227465ULL,
  14930352ULL,
  24157817ULL,
  39088169ULL,
  63245986ULL,
  102334155ULL,
  165580141ULL,
  267914296ULL,
  433494437ULL,
  701408733ULL,
  1134903170ULL,
  1836311903ULL,
  2971215073ULL,
  4807526976ULL,
  7778742049ULL,
  12586269025ULL,
  20365011074ULL,
  32951280099ULL,
  53316291173ULL,
  86267571272ULL,
  139583862445ULL,
  225851433717ULL,
  365435296162ULL,
  591286729879ULL,
  956722026041ULL,
  1548008755920ULL,
  2504730781961ULL,
  4052739537881ULL,
  6557470319842ULL,
  10610209857723ULL,
  17167680177565ULL,
  27777890035288ULL,
  44945570212853ULL,
  72723460248141ULL,
  117669030460994ULL,
  190392490709135ULL,
  308061521170129ULL,
  498454011879264ULL,
  806515533049393ULL,
  1304969544928657ULL,
  2111485077978050ULL,
  3416454622906707ULL,
  5527939700884757ULL,
  8944394323791464ULL,
  14472334024676221ULL,
  23416728348467685ULL,
  37889062373143906ULL,
  61305790721611591ULL,
  99194853094755497ULL,
  160500643816367088ULL,
  259695496911122585ULL,
  420196140727489673ULL,
  679891637638612258ULL,
  1100087778366101931ULL,
  1779979416004714189ULL,
  2880067194370816120ULL,
  4660046610375530309ULL,
  7540113804746346429ULL,
};

static constexpr i64 fibonacci_count = sizeof(fibonacci) / sizeof(u64);

void encode_fibonacci(Bit_Writer& writer, u64 N)
{
  // Add 1 to N so that we can code 0. Maximum u64 is improbable, hence this
  // is not an issue.
  N += 1;

  Bit_Stack<256> stack;
  stack.push(0x1);
  // Find largest fibonacci number such that F <= N.
  i64 index = 0;
  while(index < fibonacci_count - 1 && fibonacci[index] <= N) {
    index += 1;
  }
  index -= 1;
  while(index > 0) {
    if(fibonacci[index] <= N) {
      stack.push(0x1);
      N -= fibonacci[index];
    } else {
      stack.push(0x0);
    }
    index -= 1;
  }

  while(stack.size() > 0) {
    u8 const bit = stack.pop();
    writer.write(bit);
  }
}

u64 decode_fibonacci(Bit_Reader& reader)
{
  u8 last_bit = 0x0;
  i64 index = 1;
  u64 N = 0;
  while(true) {
    u8 const bit = reader.read();
    if(last_bit == 0x1 && bit == 0x1) {
      break;
    }

    if(bit == 0x1) {
      N += fibonacci[index];
    }
    index += 1;
    last_bit = bit;
  }
  return N - 1;
}
