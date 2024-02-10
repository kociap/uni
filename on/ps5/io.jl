# Author: Piotr Kocia

include("matrix.jl")

function read_matrix(filename::String)
  open(filename) do file
    args = split(readline(file))
    n = parse(Int64, args[1])
    l = parse(Int64, args[2])
    matrix::Band_Matrix = Band_Matrix(n, l)
    while !eof(file)
      line = split(readline(file))
      i = parse(Int, line[1])
      j = parse(Int, line[2])
      v = parse(Float64, line[3])
      set(matrix, i, j, v)
    end

    return matrix
  end
end

function read_vector(filename::String)
  open(filename) do file
    n = parse(Int64, readline(file))
    vector = zeros(n)
    for i in 1:n
      vector[i] = parse(Float64, readline(file))
    end
    return vector
  end
end
