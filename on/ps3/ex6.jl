# Author: Piotr Kocia

include("solver.jl")
using .solvers

function f1(x::Float64)::Float64
  return exp(1 - x) - 1
end

function df1(x::Float64)::Float64
  return -exp(1 - x)
end

function f2(x::Float64)::Float64
  return x * exp(-x)
end

function df2(x::Float64)::Float64
  return (x - 1) * (-exp(-x))
end

delta::Float64 = 10^(-5)
epsilon::Float64 = 10^(-5)
max_iterations = 200

println("f1 bisection [0,2]: ", solve_bisect(f1, 0.0, 2.0, delta, epsilon))
println("f1 bisection [0,3]: ", solve_bisect(f1, 0.0, 3.0, delta, epsilon))

println("f1 Newton -1.0: ", solve_newton(f1, df1, -1.0, delta, epsilon, max_iterations))
println("f1 Newton 0.0: ", solve_newton(f1, df1, 0.0, delta, epsilon, max_iterations))
println("f1 Newton 0.9: ", solve_newton(f1, df1, 0.9, delta, epsilon, max_iterations))
println("f1 Newton 1.0: ", solve_newton(f1, df1, 1.0, delta, epsilon, max_iterations))
println("f1 Newton 1.1: ", solve_newton(f1, df1, 1.1, delta, epsilon, max_iterations))

println("f1 secant 0.0, 0.5: ", solve_secant(f1, 0.0, 0.5, delta, epsilon, max_iterations))
println("f1 secant 0.0, 2.0: ", solve_secant(f1, 0.0, 2.0, delta, epsilon, max_iterations))

println("f2 bisection [-1,1]: ", solve_bisect(f2, -1.0, 1.0, delta, epsilon))
println("f2 bisection [-2,2]: ", solve_bisect(f2, -2.0, 2.0, delta, epsilon))
println("f2 bisection [-0.1,0.2]: ", solve_bisect(f2, -0.1, 0.1, delta, epsilon))

println("f2 Newton -1.0: ", solve_newton(f2, df2, -1.0, delta, epsilon, max_iterations))
println("f2 Newton -0.9: ", solve_newton(f2, df2, -0.9, delta, epsilon, max_iterations))
println("f2 Newton -0.1: ", solve_newton(f2, df2, -0.1, delta, epsilon, max_iterations))
println("f2 Newton 0.0: ", solve_newton(f2, df2, 0.0, delta, epsilon, max_iterations))
println("f2 Newton 0.1: ", solve_newton(f2, df2, 0.1, delta, epsilon, max_iterations))
println("f2 Newton 0.9: ", solve_newton(f2, df2, 0.9, delta, epsilon, max_iterations))
println("f2 Newton 1.0: ", solve_newton(f2, df2, 1.0, delta, epsilon, max_iterations))

println("f2 secant -1.0, -0.5: ", solve_secant(f2, -1.0, -0.5, delta, epsilon, max_iterations))
println("f2 secant 1.0, 0.5: ", solve_secant(f2, 1.0, 0.5, delta, epsilon, max_iterations))
println("f2 secant 0.0, 2.0: ", solve_secant(f2, 0.0, 2.0, delta, epsilon, max_iterations))


println("EXPERIMENT")
println("f1 Newton 2.0: ", solve_newton(f1, df1, 2.0, delta, epsilon, max_iterations))
println("f1 Newton 5.0: ", solve_newton(f1, df1, 5.0, delta, epsilon, max_iterations))
println("f1 Newton 100.0: ", solve_newton(f1, df1, 100.0, delta, epsilon, max_iterations))
println("f2 Newton 1.0: ", solve_newton(f2, df2, 1.0, delta, epsilon, max_iterations))
println("f2 Newton 2.0: ", solve_newton(f2, df2, 2.0, delta, epsilon, max_iterations))
println("f2 Newton 5.0: ", solve_newton(f2, df2, 5.0, delta, epsilon, max_iterations))
println("f2 Newton 100.0: ", solve_newton(f2, df2, 100.0, delta, epsilon, max_iterations))
