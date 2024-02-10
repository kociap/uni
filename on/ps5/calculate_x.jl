# Author: Piotr Kocia

include("io.jl")
include("matrix.jl")

using .band_mat
using .vector_utils

function entry(argv)
  if length(argv) < 2
    println("calculate_bx MATRIX_FILE VECTOR_FILE")
    return
  end

  matrix = read_matrix(argv[1])
  vector = read_vector(argv[2])
  gauss_elimination(matrix, vector)
  x = solve_upper_triangular(matrix, vector)
  print(x)
end

entry(ARGS)
