# Author: Piotr Kocia

function eval_next_x(x::Float64, c::Float64)
  return x * x + c
end

mutable struct Params
  x::Float64
  c::Float64
end

function main()
  parameters::Array{Params} = [
    Params(1.0, -2.0),
    Params(2.0, -2.0),
    Params(1.99999999999999, -2.0),
    Params(1.0, -1.0),
    Params(-1.0, -1.0),
    Params(0.75, -1.0),
    Params(0.2, -1.0),
  ]

  for p in parameters
    print("\$c = ", p.c, ", x_0 = ", p.x, "\$ &")
  end
  print("\n")


  for i in 1:40
    for p in parameters
      p.x = eval_next_x(p.x, p.c)
      print(p.x, " ")
    end
    print("\n")
  end
end

main()
