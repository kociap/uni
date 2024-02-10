# Author: Piotr Kocia

include("io.jl")
include("matrix.jl")
include("vector.jl")

using .band_mat
using .vector_utils

function entry(argv)
  if length(argv) < 2
    println("calculate_bx MATRIX_FILE VECTOR_FILE")
    return
  end

  matrix_base = read_matrix(argv[1])
  matrix_gauss_base = matrix_base
  matrix_gauss_select = deepcopy(matrix_base)
  matrix_lu_base = deepcopy(matrix_base)
  matrix_lu_select = deepcopy(matrix_base)

  vector_base = read_vector(argv[2])
  vector_gauss_base = deepcopy(vector_base)
  vector_gauss_select = deepcopy(vector_base)
  println("size ", matrix_base.n)
  # Gauss without selection.
  gauss_base_time = @elapsed begin
    gauss_elimination(matrix_gauss_base, vector_gauss_base)
    x = solve_upper_triangular(matrix_gauss_base, vector_gauss_base)
    error = error_relative(x, ones(matrix_gauss_base.n))
    # println(error)
    # print(x)
  end
  println(stderr, "gauss_base: ", gauss_base_time)
  # Gauss with selection.
  gauss_select_time = @elapsed begin
    gauss_elimination_partial_primary_selection(matrix_gauss_select, vector_gauss_select)
    x = solve_upper_triangular(matrix_gauss_select, vector_gauss_select)
    inverse_permutation = invperm(matrix_gauss_select.perm)
    x = x[inverse_permutation]
    error = error_relative(x, ones(matrix_gauss_select.n))
    # println(error)
    # print(x)
  end
  println(stderr, "gauss_select: ", gauss_select_time)
  # LU without selection.
  lu_base_time = @elapsed begin
    x_initial = ones(matrix_lu_base.n)
    L_matrix = decompose_LU(matrix_lu_base)
    x = solve_LU(L_matrix, matrix_lu_base, vector_base)
    error = error_relative(x, ones(matrix_lu_base.n))
    # println(error)
    # print(x)
  end
  println(stderr, "LU_base: ", lu_base_time)
  # LU with selection.
  lu_select_time = @elapsed begin
    x_initial = ones(matrix_lu_select.n)
    L_matrix = decompose_LU_partial_primary_selection(matrix_lu_select)
    x = solve_LU(L_matrix, matrix_lu_select, vector_base[matrix_lu_select.perm])
    inverse_permutation = invperm(matrix_lu_select.perm)
    x = x[inverse_permutation]
    error = error_relative(x, ones(matrix_lu_select.n))
    # println(error)
    # print(x)
  end
  println(stderr, "LU_select: ", lu_select_time)
end

entry(ARGS)
