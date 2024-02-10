# Author: Piotr Kocia

include("newton_polynomial.jl")
using .newton_polynomial
using Plots

f = x -> exp(x)
g = x -> x^2 * sin(x)

for n in [5, 10, 15]
  plot_f = draw_interpolated(f, 0.0, 1.0, n, ["f" "p"])
  plot_g = draw_interpolated(g, -1.0, 1.0, n, ["g" "p"])
  savefig(plot_f, "ex5_f1_$n.png")
  savefig(plot_g, "ex5_f2_$n.png")
end
