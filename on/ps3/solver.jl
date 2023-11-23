# Author: Piotr Kocia

module solvers

export solve_bisect, solve_newton, solve_secant

# solve_bisect
# Find the root of a function using the bisection method.
# A precondition is that f changes sign in [a, b].
#
# Parameters:
#       f - Function(Float64)::Float64 to evaluate.
#       a - lower bound.
#       b - upper bound.
#   delta - maximum length of the interval [a_n, b_n] where n is the current
#           iteration.
#   delta - maximum distance between the consecutive root approximations.
# epsilon - maximum distance between f(x) and 0.
#
# Returns:
#       root - the approximated root of f.
#      value - the value of f at root, i.e. f(root).
# iterations - the number of iterations done to find the root.
#      error - error indicator.
#              0 - no errors.
#              1 - function does not change the sign in [a, b].
#
function solve_bisect(f, a::Float64, b::Float64, delta::Float64, epsilon::Float64)
  fa::Float64 = f(a)
  fb::Float64 = f(b)
  e::Float64 = b - a
  if sign(fa) == sign(fb)
    return Nothing, Nothing, Nothing, 1
  end
  it = 1
  r::Float64 = a + e
  v::Float64 = f(r)
  while true
    e /= 2
    r = a + e
    v = f(r)
    if abs(e) < delta || abs(v) < epsilon
      return r, v, it, 0
    end
    if sign(v) != sign(fa)
      b = r
      fb = v
    else
      a = r
      fa = v
    end
    it += 1
  end
end


# solve_newton
# Find the root of a function using the Newton method.
# A precondition is that df is not close to zero.
#
# Parameters:
#              f - Function(x::Float64)::Float64 to evaluate.
#             df - derivative of f.
#             x0 - initial root guess.
#          delta - maximum distance between the consecutive root approximations.
#        epsilon - maximum distance between f(x) and 0.
# max_iterations - maximum number of iterations.
#
# Returns:
#       root - the approximated root of f.
#      value - the value of f at root, i.e. f(root).
# iterations - the number of iterations done to find the root.
#      error - error indicator.
#              0 - no errors.
#              1 - sufficient precision could not be reached in max_iterations.
#              2 - df close to 0.
#
function solve_newton(f, df, x0::Float64, delta::Float64, epsilon::Float64, max_iterations::Int)
  v::Float64 = f(x0)

  if abs(v) < epsilon
    return x0, v, 0, 0
  end

  for i in 1:max_iterations
    dfx0::Float64 = df(x0)
    if abs(dfx0) < eps(Float64)
      return x0, v, i, 2
    end
    x1::Float64 = x0 - (v / dfx0)
    v = f(x1)
    if abs(x1 - x0) < delta || abs(v) < epsilon
      return x1, v, i, 0
    end
    x0 = x1
  end

  return x0, v, max_iterations, 1
end


# solve_secant
# Find the root of a function using the secant method.
#
# Input:
#              f - Function(x::Float64)::Float64 to evaluate.
#             x0 - initial root guess.
#             x1 - kolejne przybliżenia początkowe miejsca zerowego
#          delta - maximum distance between the consecutive root approximations.
#        epsilon - maximum distance between f(x) and 0.
# max_iterations - maximum number of iterations.
#
# Returns:
#       root - the approximated root of f.
#      value - the value of f at root, i.e. f(root).
# iterations - the number of iterations done to find the root.
#      error - error indicator.
#              0 - no errors.
#              1 - sufficient precision could not be reached in max_iterations.
#
function solve_secant(f, x0::Float64, x1::Float64, delta::Float64, epsilon::Float64, max_iterations::Int)
  fx0::Float64 = f(x0)
  fx1::Float64 = f(x1)
  for i in 1:max_iterations
    if abs(fx1) > abs(fx0)
      x0, x1 = x1, x0
      fx0, fx1 = fx1, fx0
    end
    s::Float64 = (x0 - x1) / (fx0 - fx1)
    x0 = x1
    fx0 = fx1
    x1 = x1 - fx1 * s
    fx1 = f(x1)
    if abs(x0 - x1) < delta || abs(fx1) < epsilon
      return x1, fx1, i, 0
    end
  end

  return x1, fx1, max_iterations, 1
end

end
