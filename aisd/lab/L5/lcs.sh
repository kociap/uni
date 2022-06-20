for i in {1..10}; do 
    n=$((1000 * i));
    echo "iteration ${n}";
    # Remove old data files.
    rm "./o/out_lcs_${n}.txt";
    for j in {1..10}; do
        { ./o/seqgen $n; ./o/seqgen $n; } | ./o/lcs >> "./o/out_lcs_${n}.txt";
    done
done