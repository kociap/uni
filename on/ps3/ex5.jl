# Author: Piotr Kocia

include("solver.jl")
using .solvers

function f(x::Float64)::Float64
  return exp(x) - 3*x
end

delta::Float64 = 10^(-4)
epsilon::Float64 = 10^(-4)

# a = 0.5, b = 1
println("1st root ", solve_bisect(f, 0.5, 1.0986, delta, epsilon))

# a = 1, b = 1.75
println("2nd root ", solve_bisect(f, 1.0986, 2.0, delta, epsilon))
