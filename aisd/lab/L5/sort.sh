for i in {1..10}; do 
    n=$((100 * i));
    echo "iteration ${n}";
    # Remove old data files.
    rm "./o/out_sort_heap_${n}.txt" "./o/out_sort_qsort_${n}.txt" "./o/out_sort_msort_${n}.txt" "./o/out_sort_dpqsort_${n}.txt";
    for j in {1..10}; do
        input=$(./o/randgen ${n});
        { echo ${n}; echo ${input}; } | ./o/heap >> "./o/out_sort_heap_${n}.txt";
        echo ${input} | ./o/qsort >> "./o/out_sort_qsort_${n}.txt";
        echo ${input} | ./o/msort >> "./o/out_sort_msort_${n}.txt";
        echo ${input} | ./o/dpqsort >> "./o/out_sort_dpqsort_${n}.txt";
    done
done
