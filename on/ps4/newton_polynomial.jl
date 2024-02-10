# Author: Piotr Kocia

module newton_polynomial
export divided_differences, evaluate, explicit_form, draw_interpolated

using Plots


# divided_differences
#
# Parameters:
# x - roots x_i.
# f - values of f at x_i, i.e. f(x_0), ..., f(x_n).
#
# Returns:
# Divided differences f[x_0], ..., f[x_0, ..., x_n].
#
function divided_differences(x::Vector{Float64}, f::Vector{Float64})::Vector{Float64}
  n = length(x)
  fx = copy(f)
  for i in 2:n
      for j in n:-1:i
          fx[j] = (fx[j] - fx[j - 1]) / (x[j] - x[j - i + 1])
      end
  end

  return fx
end

# evaluate
# Evaluate the Newton's polynomial at t.
#
# Parameters:
#  x - roots x_i.
# fx - divided differences f[x_0], ... f[x_0, ..., x_n] of the polynomial.
#  t - point to evaluate the polynomial at.
#
# Returns:
# The value of the polynomial at t.
#
function evaluate(x::Vector{Float64}, fx::Vector{Float64}, t::Float64)::Float64
  n = length(x)
  result::Float64 = fx[n]
  for i in (n - 1):-1:1
    result = fx[i] + (t - x[i]) * result
  end
  return result
end

# explicit_form
# Calculate the coefficients a_i of the explicit form of the Newton's
# polynomial.
#
# Parameters:
# x - roots x_i.
# fx - divided differences f[x_0], ... f[x_0, ..., x_n] of the polynomial.
#
# Returns:
# The coefficients a_i of the polynomial in its explicit form.
#
function explicit_form(x::Vector{Float64}, fx::Vector{Float64})::Vector{Float64}
  n = length(x)
  a::Vector{Float64} = zeros(n)
  a[n] = fx[n]
  for i in (n - 1):-1:1
    a[i] = fx[i] - x[i] * a[i + 1]
    for j in (i + 1):(n - 1)
      a[j] += -x[i] * a[j + 1]
    end
  end

  return a
end

# draw_interpolated
#
# Parameters:
# f - function.
# a - start of the interval.
# b - end of the interval.
# n - degree of the interpolating polynomial.
#
# Returns:
# Plot with the function and the intepolating polynomial.
#
function draw_interpolated(f, a::Float64, b::Float64, n::Int, labels::Matrix{String})
  # First, evaluate the function. We need its values for the divided
  # differences.
  x_values::Vector{Float64} = collect(range(a, stop=b, length=n))
  f_values::Vector{Float64} = f.(x_values)

  dd::Vector{Float64} = divided_differences(x_values, f_values)
  # Then draw the plots.
  points = 100 * n
  x_plot::Vector{Float64} = collect(range(a, stop=b, length=points))
  f_plot_values::Vector{Float64} = f.(x_plot)
  p_plot_values::Vector{Float64} = [evaluate(x_values, dd, xi) for xi in x_plot]
  p = plot(x_plot, [f_plot_values, p_plot_values], label = labels)
  return p
end

end
