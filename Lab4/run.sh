#! /usr/bin/env bash
set -ea

SOURCE=${1:-lab4.cpp}
BINARY=~/lab4/lab4
RESULTS=~/lab4/lab4.o*

echo Running $SOURCE

echo Cleaning
rm --force $BINARY $RESULTS


echo Compiling
mpic++ $SOURCE -std=c++1y -o $BINARY


echo Running
qsub lab4.pbs # enqueue
qstat # print queue


echo Waiting for results
while [ ! -f $RESULTS ]; do sleep 0.1; done

printf "Results found\n\n"
sort $RESULTS