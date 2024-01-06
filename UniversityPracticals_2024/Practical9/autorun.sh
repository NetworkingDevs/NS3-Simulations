# $1 = path of file
# $2 = variable name
# $3 = value of Tcp Algo.
# $4 = gnuplot script name

./ns3 run "$1 --$2=$3" > cwnd.dat 2>&1
gnuplot autocapture.gp

