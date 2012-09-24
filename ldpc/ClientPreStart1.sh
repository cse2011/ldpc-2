#!/bin/bash
echo ClientPreStart1.sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pwd;
cd $DIR; make clean; make
