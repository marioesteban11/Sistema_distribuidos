for i in `seq 1 300`; do
    WAIT=`printf '0.%06d\n' $RANDOM`;
    (sleep $WAIT; echo "Lanzando cliente $i ..."; ./subscriber --ip 0.0.0.0 --port 9963 --topic TEST | tee -a fichero.txt ) &
done