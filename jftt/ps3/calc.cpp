#include <string>

#include <calc.h>

static std::string rpn;

void append_rpn_value(i64 value)
{
  rpn += std::to_string(value);
  rpn += " ";
}

void append_rpn_operator(char op)
{
  rpn += op;
  rpn += " ";
}

char const* get_rpn()
{
  return rpn.c_str();
}

void clear_rpn()
{
  rpn.clear();
}

i64 normalise(i64 const x, i64 const p)
{
  return ((x % p) + p) % p;
}

i64 add(i64 const x, i64 const y, i64 const p)
{
  return normalise(normalise(x, p) + normalise(y, p), p);
}

i64 subtract(i64 const x, i64 const y, i64 const p)
{
  return normalise(normalise(x, p) - normalise(y, p), p);
}

i64 multiply(i64 const x, i64 const y, i64 const p)
{
  return normalise(normalise(x, p) * normalise(y, p), p);
}

[[nodiscard]] static i64 inverse(i64 a, i64 const p)
{
  i64 m = p;
  i64 x = 1;
  i64 y = 0;

  while(a > 1) {
    i64 quotient = a / m;
    i64 t = m;

    m = a % m;
    a = t;
    t = y;

    y = x - quotient * y;
    x = t;
  }

  if(x < 0) {
    x += p;
  }

  return x;
}

i64 divide_unchecked(i64 const x, i64 const y, i64 const p)
{
  i64 const inv = inverse(y, p);
  return multiply(x, inv, p);
}

i64 mod_unchecked(i64 const x, i64 const y, i64 const p)
{
  return normalise(normalise(x, p) % normalise(y, p), p);
}

i64 power(i64 x, i64 y, i64 const p)
{
  x = normalise(x, p);
  i64 result = 1;
  for(i64 i = 0; i < y; i++) {
    result = normalise(result * x, p);
  }
  return result;
}

i64 unary_minus(i64 const x, i64 const p)
{
  return normalise(-x, p);
}
