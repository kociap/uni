for i in {1..10}; do 
    n=$((10000 * i));
    echo "iteration ${n}";
    # Remove old data files.
    rm "./o/out_heap_${n}.txt";
    for j in {1..10}; do
        { echo "$n"; ./o/randgen ${n}; } | ./o/heap >> "./o/out_heap_${n}.txt";
    done
done
