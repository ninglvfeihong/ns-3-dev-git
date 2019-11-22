#!/bin/bash

# LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib ./$1
for i in `seq 1 7`
do 
	terminator -e "mkdir -p mode$i && cd mode$i && LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/ns3/build/lib ../$1 --mode=$i; bash"
done
