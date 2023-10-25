# Author: Piotr Kocia

function calculate_step_at(ibegin::Float64)
  return nextfloat(ibegin) - ibegin;
end

function main()
  v1 = 1.0
  for i in (1:10)
    s1 = calculate_step_at(v1)
    println(rpad(1.0, 4), rpad(2.0, 4), rpad(s1, 25), bitstring(s1))
    v1 += s1;
  end

  v2 = 0.5
  for i in (1:10)
    s2 = calculate_step_at(v2)
    println(rpad(0.5, 4), rpad(1.0, 4), rpad(s2, 25), bitstring(s2))
    v2 += s2;
  end

  v3 = 2.0
  for i in (1:10)
    s3 = calculate_step_at(v3)
    println(rpad(2.0, 4), rpad(4.0, 4), rpad(s3, 25), bitstring(s3))
    v3 += s3;
  end
end

main()
