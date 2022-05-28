#!/bin/bash

for i in `seq 1 2`; do
	(sleep 1; echo "Lanzando cliente $i ..."; ./subscriber --ip 0.0.0.0 --port 9963 --topic TEST | tee fichero.txt )&
	echo "Lanzando cliente $i ...";
done