# Author: Piotr Kocia

eps_f16::Float16 = (Float16(3.0) * ((Float16(4.0) / Float16(3.0)) - Float16(1.0))) - Float16(1.0)
println(eps_f16)
println(eps(Float16))
eps_f32::Float32 = (Float32(3.0) * ((Float32(4.0) / Float32(3.0)) - Float32(1.0))) - Float32(1.0);
println(eps_f32)
println(eps(Float32))
eps_f64::Float64 = (Float64(3.0) * ((Float64(4.0) / Float64(3.0)) - Float64(1.0))) - Float64(1.0)
println(eps_f64)
println(eps(Float64))
