#!/bin/bash

for k in d u; do
    for t in a b; do
        for i in {1..6}; do
            ./main_e4 < "./4/${k}4${t}-${i}.txt" > "./out/4${k}${t}${i}.txt"
        done
    done
done
