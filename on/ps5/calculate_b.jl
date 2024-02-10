# Author: Piotr Kocia

include("io.jl")
include("matrix.jl")
include("vector.jl")

using .band_mat
using .vector_utils

function entry(argv)
  if length(argv) < 1
    println("calculate_bx MATRIX_FILE")
    return
  end

  matrix = read_matrix(argv[1])

  x = ones(matrix.n)
  b = multiply(matrix, x)
  print(b)
end

entry(ARGS)
