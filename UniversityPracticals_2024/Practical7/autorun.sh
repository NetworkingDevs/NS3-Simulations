# $1 = file name/file path (i.e. ./scratch/Test)
# $2 = variable name (i.e. mDist)
# $3 = variable start value (i.e. 5)
# $4 = distance jump (i.e. 5)

dist=$3
lines=4
jump=$4

while [ $lines -gt 3 ]
do
	echo "dist : $dist"
	./ns3 run "$1 $2=$dist" > test.txt 2>&1
	lines=$(wc -l < "test.txt")
	echo "lines : $lines"
	dist=$((dist + jump))
	sleep 1
done

echo "The packet drop at dist: $dist"
