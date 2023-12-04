#pragma once

typedef long long i64;

void append_rpn_value(i64 value);
void append_rpn_operator(char op);
char const* get_rpn();
void clear_rpn();

static constexpr i64 group_p = 1234577;
static constexpr i64 ring_p = 1234577 - 1;

i64 normalise(i64 x, i64 p);
i64 add(i64 x, i64 y, i64 p);
i64 subtract(i64 x, i64 y, i64 p);
i64 multiply(i64 x, i64 y, i64 p);
i64 divide_unchecked(i64 x, i64 y, i64 p);
i64 mod_unchecked(i64 x, i64 y, i64 p);
i64 power(i64 x, i64 y, i64 p);
i64 unary_minus(i64 x, i64 p);
