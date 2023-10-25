# Author: Piotr Kocia

function find_least_noninvertible()::Float64
  current::Float64 = 1.0
  next::Float64 = nextfloat(1.0)
  while (next * (Float64(1.0) / next) == Float64(1.0))
    current = next;
    next = nextfloat(next)
  end
  return next
end

noninvertible::Float64 = find_least_noninvertible()
println(noninvertible)
