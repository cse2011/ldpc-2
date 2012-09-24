#!/bin/bash -v
#time ./client -s $1 -p $2 -l $3 -m 1400 
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR && sudo ./sender $1 $3
#time sudo ./sender $1 $3
