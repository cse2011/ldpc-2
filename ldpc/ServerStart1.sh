#!/bin/bash -v
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR && sudo ./receiver
sudo chown $USER $DIR/data
mv data test1G.bin
#./server -p $1
#time sudo ./receiver
