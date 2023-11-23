# Author: Piotr Kocia

include("solver.jl")
using .solvers

function f(x::Float64)::Float64
  return sin(x) - (x / 2)^2
end

function df(x::Float64)::Float64
  return cos(x) - (x / 2)
end

delta::Float64 = 0.5 * 10^(-5)
epsilon::Float64 = 0.5 * 10^(-5)

println("bisection ", solve_bisect(f, 1.5, 2.0, delta, epsilon))
println("Newton ", solve_newton(f, df, 1.5, delta, epsilon, 1000))
println("secant ", solve_secant(f, 1.0, 2.0, delta, epsilon, 1000))