# Author: Piotr Kocia

function fn_p(p::T, r::T)::T where{T <: AbstractFloat}
  return p + r * p * (one(T) - p)
end

function run_compare_f32_and_f64(p32::Float32, r32::Float32, p64::Float64, r64::Float64, n::Int64)
  println(0, " ", p32, " ", p64)
  for i in 1:n
    p32 = fn_p(p32, r32)
    p64 = fn_p(p64, r64)
    println(i, " ", p32, " ", p64)
  end
end

function run_trunc_f32(p32::Float32, r32::Float32, n::Int64, t::Int64)
  p32_correct::Float32 = p32
  println(0, " ", p32, " ", p32_correct)
  for i in 1:t
    p32_correct = fn_p(p32_correct, r32)
    p32 = fn_p(p32, r32)
    println(i, " ", p32, " ", p32_correct)
  end
  p32 = trunc(p32, digits=3)
  for i in (t+1):n
    p32_correct = fn_p(p32_correct, r32)
    p32 = fn_p(p32, r32)
    println(i, " ", p32, " ", p32_correct)
  end
end

function main()
  println("TRUNC")
  run_trunc_f32(Float32(0.01), Float32(3.0), 40, 10)
  println("COMPARE")
  run_compare_f32_and_f64(Float32(0.01), Float32(3.0), 0.01, 3.0, 40)
end

main()
