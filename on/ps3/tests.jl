# Author: Piotr Kocia

include("solver.jl")
using .solvers
using Test

epsilon = delta = 10^(-5)

@testset "bisection" begin
  # Good example.
  f = x -> 2 * x^2 - x
  exact_solution = 0.5
  x, fx, i, error = solve_bisect(f, 0.1, 1.0, delta, epsilon)
  @test abs(fx) <= epsilon
  @test abs(exact_solution - x) <= delta
  @test error == 0

  # No sign change.
  x, fx, i, error = solve_bisect(f, 0.7, 1.0, delta, epsilon)
  @test fx == Nothing
  @test x == Nothing
  @test i == Nothing
  @test error == 1

  # Root in the middle of the interval.
  x, fx, i, error = solve_bisect(f, 0.0, 1.0, delta, epsilon)
  @test fx == 0
  @test x == 0.5
  @test i == 1
  @test error == 0
end

@testset "Newton" begin
  # Good example.
  f = x -> x^3 - 1
  df = x -> 3 * x^2
  exact_solution = 1.0
  x, fx, i, error = solve_newton(f, df, 0.5, delta, epsilon, 10)
  @test abs(fx) < epsilon
  @test abs(exact_solution-x) < delta
  @test error == 0

  # Derivative equal 0 at the initial guess.
  f = x -> 2 * x^2 - x
  df = x -> 4 * x - 1
  x, fx, i, error = solve_newton(f, df, 0.25, delta, epsilon, 10)
  @test error == 2

  # Bad initial guess.
  f = x -> x^3 - 2 * x + 2
  df = x -> 3 * x^2 - 2
  x, fx, i, error  = solve_newton(f, df, 0.0, delta, epsilon, 51)
  @test abs(fx) > epsilon
  @test abs(x) > delta
  @test i == 51
  @test error == 1
end

@testset "secant" begin
  # Good example.
  f = x -> x^3 - 1
  exact_solution = 1.0
  x, fx, i, error = solve_secant(f, 0.5, 0.7, delta, epsilon, 10)
  @test abs(fx) < epsilon
  @test abs(exact_solution-x) < delta
  @test error == 0

  # Derivative is 0 at the root.
  f = x -> x^2
  x, fx, i, error = solve_secant(f, -1.0, 1.0, delta, epsilon, 50)
  @test isnan(fx)
  @test isnan(x)
  @test i == 50
  @test error == 1
end
