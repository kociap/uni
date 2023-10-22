#include <string>
#include <vector>

static std::vector<long> arguments;
static std::string error;

extern "C" void push_arg(long const value)
{
  arguments.push_back(value);
}

extern "C" void do_op(char const* const op)
{
  if(error.size() > 0) {
    return;
  }

  if(arguments.size() < 2) {
    error = "error: too few arguments";
    return;
  }

  long const rhs = arguments.back();
  arguments.pop_back();
  long const lhs = arguments.back();
  arguments.pop_back();
  switch(op[0]) {
  case '+': {
    long const value = lhs + rhs;
    arguments.push_back(value);
  } break;

  case '-': {
    long const value = lhs - rhs;
    arguments.push_back(value);
  } break;

  case '*': {
    long const value = lhs * rhs;
    arguments.push_back(value);
  } break;

  case '/': {
    if(rhs == 0) {
      error = "error: division by 0";
      return;
    }

    long const value = lhs / rhs;
    arguments.push_back(value);
  } break;

  case '%': {
    if(rhs == 0) {
      error = "error: modulo by 0";
      return;
    }

    long const value = lhs % rhs;
    arguments.push_back(value);
  } break;

  case '^': {
    if(lhs == 0 && rhs == 0) {
      error = "error: 0^0 not evaluable";
      return;
    }

    // Negative exponent always yields 0 except in the case of 1 which yields 1.
    if(rhs < 0) {
      if(lhs == 1) {
        arguments.push_back(1);
      } else {
        arguments.push_back(0);
      }
      return;
    }

    long value = 1;
    for(long e = 0; e < rhs; e += 1) {
      value *= lhs;
    }
    arguments.push_back(value);
  } break;

  default: {
    error = std::string("error: unrecognised symbol '") + op[0] + "'";
    return;
  }
  }
}

extern "C" char const* get_error(void)
{
  if(error.size() > 0) {
    return error.c_str();
  } else {
    return nullptr;
  }
}

extern "C" long get_result(void)
{
  return arguments.back();
}

extern "C" void clear_state(void)
{
  error.clear();
  arguments.clear();
}
