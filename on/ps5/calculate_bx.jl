# Author: Piotr Kocia

include("io.jl")
include("vector.jl")
include("matrix.jl")

using .band_mat
using .vector_utils

function entry(argv)
  if length(argv) < 1
    println("calculate_bx MATRIX_FILE")
    return
  end

  matrix = read_matrix(argv[1])
  x_initial = ones(matrix.n)
  b = multiply(matrix, x_initial)
  gauss_elimination(matrix, b)
  x = solve_upper_triangular(matrix, b)
  error = error_relative(x, x_initial)
  println(error)
  print(x)
end

entry(ARGS)
