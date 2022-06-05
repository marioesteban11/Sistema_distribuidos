#!/bin/sh

for i in `seq 1 100`; do
    (./subscriber --ip 0.0.0.0 --port 9963 --topic TEST  ) &
done