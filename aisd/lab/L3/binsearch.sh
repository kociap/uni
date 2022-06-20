for i in {10..100..5}; do 
    n=$((1000 * i));
    path="./p4/in_rand${n}.txt";
    outpath="./p4/$1${n}.txt";
    arg=0;
    if [ "$1" = "rand" ]; then
        arg=$((($RANDOM % 1000) * i))
    elif [ "$1" = "beg" ]; then
        arg=$((100 * i))
    elif [ "$1" = "mid" ]; then
        arg=$((490 * i));
    elif [ "$1" = "end" ]; then
        arg=$((900 * i));
    else
        # Default to outside.
        arg=$((n + 1));
    fi
    # echo "n=${n}; arg=${arg}";
    # Remove previous result file.
    rm -f ${outpath}; 
    for j in {1..10}; do 
        cat ${path} | ./searcharg ${arg} | ./binsearch.exe >> ${outpath}; 
    done
done
