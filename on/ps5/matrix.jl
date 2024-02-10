# Author: Piotr Kocia

module band_mat

export Band_Matrix, identity, at_unchecked, at, set, print, swap_rows,
  multiply
export gauss_elimination, gauss_elimination_partial_primary_selection,
  decompose_LU, decompose_LU_partial_primary_selection
export solve_upper_triangular, solve_lower_triangular, solve_LU

import Base.print
using Printf

struct Band_Matrix
  storage::Matrix
  perm::Vector{Int64}
  n::Int64
  l::Int64

  # We allocate 4 times l to have an overlap with the C submatrix of the
  # following section. This is to accommodate row swapping between adjacent
  # sections.
  Band_Matrix(n, l) = new(zeros(Float64, n, 4 * l), collect(1:n), n, l)
end

function identity(n::Int64, l::Int64)
  matrix::Band_Matrix = Band_Matrix(n, l)
  for row in 1:n
    set(matrix, row, row, 1.0)
  end
  return matrix
end

function at_unchecked(this::Band_Matrix, row::Int64, column::Int64)
  row = this.perm[row] - 1
  column -= 1
  offset::Int64 = this.l * div(row, this.l)
  return this.storage[row+1, column-offset+this.l+1]
end

function at(this::Band_Matrix, row::Int64, column::Int64)
  row = this.perm[row] - 1
  column -= 1
  offset::Int64 = this.l * div(row, this.l)
  y = column - offset + this.l + 1
  if y >= 1 && y <= 4 * this.l
    @inbounds return this.storage[row+1, y]
  else
    return 0
  end
end

function set(this::Band_Matrix, row::Int64, column::Int64, value::Float64)
  row = this.perm[row] - 1
  column -= 1
  offset::Int64 = this.l * div(row, this.l)
  @inbounds this.storage[row+1, column-offset+this.l+1] = value
end

function swap_rows(this::Band_Matrix, row1::Int64, row2::Int64)
  this.perm[row1], this.perm[row2] = this.perm[row2], this.perm[row1]
end

function print(this::Band_Matrix)
  println("matrix n: ", this.n, ", l: ", this.l)
  for row in 1:this.n
    print("  ")
    for column in 1:this.n
      @printf("%7.2lf ", Float64(at(this, row, column)))
    end
    println("")
  end
end

function multiply(matrix::Band_Matrix, vector::Vector)
  x = zeros(matrix.n)
  for row in 1:matrix.n
    sum = 0.0
    min_cutoff = max(row - matrix.l, 1)
    max_cutoff = min(row + 2 * matrix.l, matrix.n)
    for column in min_cutoff:max_cutoff
      sum += vector[column] * at(matrix, row, column)
    end
    x[row] = sum
  end
  return x
end

function gauss_elimination(matrix::Band_Matrix, vector::Vector)
  for src_row in 1:matrix.n
    pivot = at(matrix, src_row, src_row)
    # This goes over many 0 entries, but not too keen on optimising that atm.
    row_cutoff = min(src_row + matrix.l - (src_row % matrix.l), matrix.n)
    for dst_row in (src_row+1):row_cutoff
      dst_pivot = at(matrix, dst_row, src_row)
      # Ignore all rows that have a 0 in the pivot location.
      if dst_pivot == 0
        continue
      end
      factor = dst_pivot / pivot
      # The first destination entry ought to be 0. Set it manually to avoid
      # computation errors.
      set(matrix, dst_row, src_row, 0.0)
      column_cutoff = min(src_row + matrix.l, matrix.n)
      for column in (src_row+1):column_cutoff
        src_entry = at(matrix, src_row, column)
        dst_entry = at(matrix, dst_row, column)
        value = dst_entry - src_entry * factor
        set(matrix, dst_row, column, value)
      end
      # Update the solution vector.
      vector[dst_row] -= vector[src_row] * factor
    end
  end
end

function gauss_elimination_partial_primary_selection(matrix::Band_Matrix,
  vector::Vector)
  for src_row in 1:matrix.n
    pivot = at(matrix, src_row, src_row)
    pivot_index = src_row
    # Select the largest pivot.
    row_cutoff = min(src_row + matrix.l - (src_row % matrix.l), matrix.n)
    for row in (src_row+1):row_cutoff
      candidate = at_unchecked(matrix, row, src_row)
      if abs(candidate) > abs(pivot)
        pivot = candidate
        pivot_index = row
      end
    end
    swap_rows(matrix, src_row, pivot_index)
    vector[src_row], vector[pivot_index] = vector[pivot_index], vector[src_row]
    # This goes over many 0 entries, but not too keen on optimising that atm.
    for dst_row in (src_row+1):row_cutoff
      dst_pivot = at(matrix, dst_row, src_row)
      # Ignore all rows that have a 0 in the pivot location.
      if dst_pivot == 0
        continue
      end
      factor = dst_pivot / pivot
      # The first destination entry ought to be 0. Set it manually to avoid
      # computation errors.
      set(matrix, dst_row, src_row, 0.0)
      column_cutoff = min(src_row + 2 * matrix.l, matrix.n)
      for column in (src_row+1):column_cutoff
        src_entry = at(matrix, src_row, column)
        dst_entry = at(matrix, dst_row, column)
        value = dst_entry - src_entry * factor
        set(matrix, dst_row, column, value)
      end
      # Update the solution vector.
      vector[dst_row] -= vector[src_row] * factor
    end
  end
end

function decompose_LU(U_matrix::Band_Matrix)
  L_matrix::Band_Matrix = identity(U_matrix.n, U_matrix.l)
  for src_row in 1:U_matrix.n
    pivot = at(U_matrix, src_row, src_row)
    # This goes over many 0 entries, but not too keen on optimising that atm.
    row_cutoff = min(src_row + U_matrix.l - (src_row % U_matrix.l), U_matrix.n)
    for dst_row in (src_row+1):row_cutoff
      dst_pivot = at(U_matrix, dst_row, src_row)
      # We want to skip all rows that have a 0 in the pivot location. Otherwise
      # we would have to work around division by zero.
      if dst_pivot == 0
        continue
      end
      factor = dst_pivot / pivot
      # The first destination entry ought to be 0. Set it manually to avoid
      # computation errors.
      set(L_matrix, dst_row, src_row, factor)
      set(U_matrix, dst_row, src_row, 0.0)
      column_cutoff = min(src_row + U_matrix.l, U_matrix.n)
      for column in (src_row+1):column_cutoff
        src_entry = at_unchecked(U_matrix, src_row, column)
        dst_entry = at_unchecked(U_matrix, dst_row, column)
        value = dst_entry - src_entry * factor
        set(U_matrix, dst_row, column, value)
      end
    end
  end
  return L_matrix
end

function decompose_LU_partial_primary_selection(U_matrix::Band_Matrix)
  L_matrix::Band_Matrix = Band_Matrix(U_matrix.n, U_matrix.l)
  for src_row in 1:U_matrix.n
    pivot = at(U_matrix, src_row, src_row)
    pivot_index = src_row
    # Select the largest pivot.
    row_cutoff = min(src_row + U_matrix.l - (src_row % U_matrix.l), U_matrix.n)
    for row in (src_row+1):row_cutoff
      candidate = at(U_matrix, row, src_row)
      if abs(candidate) > abs(pivot)
        pivot = candidate
        pivot_index = row
      end
    end
    swap_rows(U_matrix, src_row, pivot_index)
    swap_rows(L_matrix, src_row, pivot_index)
    # This goes over many 0 entries, but not too keen on optimising that atm.
    row_cutoff = min(src_row + U_matrix.l, U_matrix.n)
    for dst_row in (src_row+1):row_cutoff
      dst_pivot = at(U_matrix, dst_row, src_row)
      # We want to skip all rows that have a 0 in the pivot location. Otherwise
      # we would have to work around division by zero.
      if dst_pivot == 0
        continue
      end
      factor = dst_pivot / pivot
      # The first destination entry ought to be 0. Set it manually to avoid
      # computation errors.
      set(L_matrix, dst_row, src_row, factor)
      set(U_matrix, dst_row, src_row, 0.0)
      column_cutoff = min(src_row + 2 * U_matrix.l, U_matrix.n)
      for column in (src_row+1):column_cutoff
        src_entry = at_unchecked(U_matrix, src_row, column)
        dst_entry = at_unchecked(U_matrix, dst_row, column)
        value = dst_entry - src_entry * factor
        set(U_matrix, dst_row, column, value)
      end
    end
  end
  # Set diagonal of the L matrix to 1.
  for row in 1:L_matrix.n
    set(L_matrix, row, row, 1.0)
  end
  return L_matrix
end

function solve_upper_triangular(matrix::Band_Matrix, vector::Vector)
  x = zeros(matrix.n)
  for row in matrix.n:-1:1
    # x_i = (b_i - sum) / m_ii
    column_cutoff = min(row + 2 * matrix.l, matrix.n)
    sum = 0.0
    for column in (row+1):column_cutoff
      sum += at(matrix, row, column) * x[column]
    end
    pivot = at(matrix, row, row)
    x[row] = (vector[row] - sum) / pivot
  end
  return x
end

function solve_lower_triangular(matrix::Band_Matrix, vector::Vector)
  x = zeros(matrix.n)
  for row in 1:matrix.n
    # x_i = (b_i - sum) / m_ii
    column_cutoff = max(row - matrix.l, 1)
    sum = 0.0
    for column in column_cutoff:row
      sum += at(matrix, row, column) * x[column]
    end
    pivot = at(matrix, row, row)
    x[row] = (vector[row] - sum) / pivot
  end
  return x
end

function solve_LU(L_matrix::Band_Matrix, U_matrix::Band_Matrix, vector::Vector)
  y = solve_lower_triangular(L_matrix, vector)
  x = solve_upper_triangular(U_matrix, y)
  return x
end

end
