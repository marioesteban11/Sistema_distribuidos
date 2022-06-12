#!/bin/bash

declare -a array1


# array1[0]='5'
# echo ${array1[$i]}
# 
# Unix[0]='Debian'
# Unix[1]='Red hat'
# Unix[2]='Ubuntu'
# Unix[3]='Suse'
# 
# echo ${Unix[1]}
for i in `seq 0 10`; do
    array1[$i]=$i
done 


for i in `seq 0 10`; do
    echo ${array1[$i]}
done 