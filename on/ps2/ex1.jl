# Author: Piotr Kocia

function forward_sum(T, x, y)::T
  sum::T = zero(T)
  if (length(x) == length(y))
    for i in (1:length(x))
      sum += x[i] * y[i]
    end
  end
  return sum
end

function backward_sum(T, x, y)::T
  sum::T = zero(T)
  if (length(x) == length(y))
    for i in (length(x):-1:1)
      sum += x[i] * y[i]
    end
  end
  return sum
end

function ordered_sum(T, x, y, descending::Bool)::T
  if (length(x) == length(y))
    sum_positive::T = zero(T)
    sum_negative::T = zero(T)
    products = Array{T}(undef, length(x))
    for i in (1:length(x))
      products[i] = x[i] * y[i]
    end
    sort!(products, by = abs, rev = descending)
    for i in (1:length(x))
      if (products[i] >= zero(T))
        sum_positive += products[i]
      end
    end

    for i in (length(x):-1:1)
      if (products[i] < zero(T))
        sum_negative += products[i]
      end
    end
    return sum_positive + sum_negative
  else
    return 0
  end
end

x_f32 = [Float32(2.718281828), Float32(-3.141592654), Float32(1.414213562), Float32(0.577215664), Float32(0.301029995)]
y_f32 = [Float32(1486.2497), Float32(878366.9879), Float32(-22.37492), Float32(4773714.647), Float32(0.000185049)]

println(forward_sum(Float32, x_f32, y_f32))
println(backward_sum(Float32, x_f32, y_f32))
println(ordered_sum(Float32, x_f32, y_f32, true))
println(ordered_sum(Float32, x_f32, y_f32, false))

x_f64 = [2.718281828, -3.141592654, 1.414213562, 0.577215664, 0.301029995]
y_f64 = [1486.2497, 878366.9879, -22.37492, 4773714.647, 0.000185049]

println(forward_sum(Float64, x_f64, y_f64))
println(backward_sum(Float64, x_f64, y_f64))
println(ordered_sum(Float64, x_f64, y_f64, true))
println(ordered_sum(Float64, x_f64, y_f64, false))
