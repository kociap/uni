# Author: Piotr Kocia

function f(x::Float64)::Float64
  return sqrt(x^2 + 1) - 1
end

function g(x::Float64)::Float64
  return x^2 / (sqrt(x^2 + 1) + 1)
end

function iterative_comparison()
  i::Int32 = -1
  arg::Float64 = 8.0^-1;
  while (arg > zero(Float64))
    println(rpad(i, 20), rpad(arg, 40), rpad(f(arg), 40), rpad(g(arg), 40))
    i -=  1
    arg = 8.0^i;
  end
end

iterative_comparison()
