#!/bin/bash

# LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib ./$1
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/ns3/build/lib ./$1
