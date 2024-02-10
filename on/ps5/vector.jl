# Author: Piotr Kocia

module vector_utils

export magnitude, error_relative, print

import Base.print

function magnitude(this::Vector)
  sum = 0.0
  for i in 1:length(this)
    sum += this[i] * this[i]
  end
  return sqrt(sum)
end

function error_relative(x::Vector, x_initial::Vector)
  return magnitude(x_initial - x) / magnitude(x_initial)
end

function print(this::Vector)
  println(length(this))
  for row in 1:length(this)
    println(this[row])
  end
end

end
