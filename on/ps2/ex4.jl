# Author: Piotr Kocia

using Polynomials
using Printf

function main()
  # Wilkinson coefficients.
  wilkinson = [
    1,                      -210.0,                 20615.0,
    -1256850.0,             53327946.0,             -1672280820.0,
    40171771630.0,          -756111184500.0,        11310276995381.0,
    -135585182899530.0,     1307535010540395.0,     -10142299865511450.0,
    63030812099294896.0,    -311333643161390640.0,  1206647803780373360.0,
    -3599979517947607200.0, 8037811822645051776.0,  -12870931245150988800.0,
    13803759753640704000.0, -8752948036761600000.0, 2432902008176640000.0
  ]

  # Polynomial requires coefficients in the reverse order of p.
  coefficients = wilkinson[end:-1:1]
  p_canonical = Polynomial(coefficients)
  p_factored = fromroots(1:20)
  calculated_roots = roots(p_canonical)

  println("EVAL")
  for z in calculated_roots
    println(z, "  ", p_canonical(z), "  ", p_factored(z))
  end
  println("DIFF")
  for (z, k) in zip(calculated_roots, 1:20)
    println(k, "  ", abs(z - k))
  end
  println("MODIFIED")
  coefficients_modified = copy(coefficients)
  coefficients_modified[20] = -210.0 - (1.0/(2.0^23))
  modified_roots = roots(Polynomial(coefficients_modified))
  for (o, m) in zip(calculated_roots, modified_roots)
    println(o, " ", m)
  end
end

main()
