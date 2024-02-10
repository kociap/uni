# Author: Piotr Kocia

include("newton_polynomial.jl")
using .newton_polynomial
using Plots

f = x -> abs(x)
g = x -> 1 / (1 + x^2)

for n in [5, 10, 15, 20]
  plot_f = draw_interpolated(f, -1.0, 1.0, n, ["f" "p"])
  plot_g = draw_interpolated(g, -5.0, 5.0, n, ["g" "p"])
  savefig(plot_f, "ex6_f1_$n.png")
  savefig(plot_g, "ex6_f2_$n.png")
end
