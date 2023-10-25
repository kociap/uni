# Author: Piotr Kocia

function calc_macheps(T)::T
  current::T = one(T)
  next::T = current / 2
  while (one(T) + next > one(T))
    current = next
    next = current / 2
  end
  return current
end

function calc_eta(T)::T
  current::T = one(T)
  next::T = current / 2
  while (next > zero(T))
    current = next
    next = current / 2
  end
  return current
end

function calc_max(T)::T
  current::T = one(T)
  next::T = current * 2
  while (!isinf(next))
    current = next
    next = current * 2
  end
  step::T = current / 2
  while (step > zero(T) && current + step > current)
    next = current + step
    if (!isinf(next))
      current = next
    else
      step /= 2
    end
  end
  return current
end

println(calc_macheps(Float16))
println(eps(Float16))
println(calc_macheps(Float32))
println(eps(Float32))
println(calc_macheps(Float64))
println(eps(Float64))
println(calc_eta(Float16))
println(nextfloat(Float16(0.0)))
println(calc_eta(Float32))
println(nextfloat(Float32(0.0)))
println(calc_eta(Float64))
println(nextfloat(Float64(0.0)))
println(floatmin(Float32))
println(floatmin(Float64))
println(calc_max(Float16))
println(floatmax(Float16))
println(calc_max(Float32))
println(floatmax(Float32))
println(calc_max(Float64))
println(floatmax(Float64))
