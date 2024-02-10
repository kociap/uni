# Author: Piotr Kocia

include("newton_polynomial.jl")
using .newton_polynomial
using Test

@testset "primary" begin
  f = x -> 2*x^3 + x^2 + 3*x + 7
  x::Vector{Float64} = [0.0, 1.0, 2.0, 3.0, 4.0, 5.0]
  f_values::Vector{Float64} = [f(x[1]), f(x[2]), f(x[3]), f(x[4]), f(x[5]), f(x[6])]
  dd::Vector{Float64} = divided_differences(x, f_values)
  # Calculated by hand.
  @test dd[1] == 7.0
  @test dd[2] == 6.0
  @test dd[3] == 7.0
  @test dd[4] == 2.0
  @test dd[5] == 0.0
  @test dd[6] == 0.0
  @test evaluate(x, dd, x[1]) == f(x[1])
  @test evaluate(x, dd, x[2]) == f(x[2])
  @test evaluate(x, dd, x[3]) == f(x[3])
  @test evaluate(x, dd, x[4]) == f(x[4])
  @test evaluate(x, dd, x[5]) == f(x[5])
  @test evaluate(x, dd, x[6]) == f(x[6])
  coefficients = explicit_form(x, dd)
  @test coefficients[1] == 7.0
  @test coefficients[2] == 3.0
  @test coefficients[3] == 1.0
  @test coefficients[4] == 2.0
  @test coefficients[5] == 0.0
  @test coefficients[6] == 0.0
end
