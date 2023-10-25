# Author: Piotr Kocia

function f(x::Float64)::Float64
  return sin(x) + cos(3*x)
end

function derivative(x::Float64)::Float64
  return cos(x) - 3.0 * sin(3.0 * x)
end

function approximated_derivative(x::Float64, h::Float64)::Float64
  return (f(x + h) - f(x)) / h
end

function iterative_comparison()
  n::Int32 = 0
  h::Float64 = 1.0
  df::Float64 = derivative(one(Float64))
  println(df)
  while n <= 54
    approx_df::Float64 = approximated_derivative(one(Float64), h)
    println(rpad(n, 10), rpad(approx_df, 40), rpad(abs(df - approx_df), 40), rpad(one(Float64) + h, 40))
    n += 1
    h = 2.0^-n;
  end
end

iterative_comparison()
