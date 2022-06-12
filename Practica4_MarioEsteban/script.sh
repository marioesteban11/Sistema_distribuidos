#!/bin/sh

for i in `seq 1 1000`; do
    (./subscriber --ip 0.0.0.0 --port 9963 --topic TEST | tee -a justo_1000.txt ) &
done
