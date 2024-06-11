samples=$1
delay=$2

for (( i=1; i<=$samples; ++i ))
do
        echo Sample $i of $samples, every $delay sec
        top -b -n1 | head -n 5
        echo ----
        who -d -u --ips --lookup
        echo ----
        free -m
        echo ----
        uname -a
        sleep $delay
done

