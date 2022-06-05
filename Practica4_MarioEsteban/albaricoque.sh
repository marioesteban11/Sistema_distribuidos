#!/bin/bash

times=5
while [ $times -gt 0 ];
do
    (./subscriber --ip 0.0.0.0 --port 9963 --topic TEST  | tee -a paralelo_10.txt)&
    times=$(($times-1))
done