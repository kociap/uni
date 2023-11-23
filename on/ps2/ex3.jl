# Author: Piotr Kocia

using LinearAlgebra
using Printf

function hilb(n::Int)
# Function generates the Hilbert matrix  A of size n,
#  A (i, j) = 1 / (i + j - 1)
# Inputs:
#	n: size of matrix A, n>=1
#
#
# Usage: hilb(10)
#
# Pawel Zielinski
  if n < 1
    error("size n should be >= 1")
  end
  return [1 / (i + j - 1) for i in 1:n, j in 1:n]
end

function matcond(n::Int, c::Float64)
# Function generates a random square matrix A of size n with
# a given condition number c.
# Inputs:
#	n: size of matrix A, n>1
#	c: condition of matrix A, c>= 1.0
#
# Usage: matcond(10, 100.0)
#
# Pawel Zielinski
  if n < 2
    error("size n should be > 1")
  end
  if c< 1.0
    error("condition number  c of a matrix  should be >= 1.0")
  end
  (U,S,V)=svd(rand(n,n))
  return U*diagm(0 =>[LinRange(1.0,c,n);])*V'
end

# calculate_solve_error
# Calculate the relative error for Gauss elimination and inverse matrix solving
# methods.
#
# Parameters:
# n - the rank of the Hilbert matrix.
#
# Returns:
# Tuple containing (condition number, Gauss error, inverse error).
#
function calculate_solve_error(m::Matrix{Float64})::Tuple{Float64, Float64, Float64}
  x = ones(Float64, size(m, 2))
  b = m * x

  x_gauss = m \ b
  x_inverse = inv(m) * b

  error_gauss = norm(x_gauss - x) / norm(x)
  error_inverse = norm(x_inverse - x) / norm(x)
  return (cond(m), error_gauss, error_inverse);
end

function main()
  println("HILBERT")
  for n = 1:20
    H = hilb(n)
    cond, gauss, inverse = calculate_solve_error(H)
    println(n, "  ", cond, "  ", gauss, "  ", inverse)
  end

  println("RANDOM")
  sizes = [5, 10, 20]
  cond_nums = [Float64(1), Float64(10), Float64(10^3), Float64(10^7), Float64(10^12), Float64(10^16)]
  for s in sizes, c in cond_nums
    M = matcond(s, c)
    cond, gauss, inverse = calculate_solve_error(M)
    println(s, "  ", cond, "  ", gauss, "  ", inverse)
  end
end

main()
