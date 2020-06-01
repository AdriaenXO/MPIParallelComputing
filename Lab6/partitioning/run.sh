#! /usr/bin/env bash
set -ea

SOURCE=${1:-partitioning.cpp}
BINARY=~/lab6/partitioning/partitioning
RESULTS=~/lab6/partitioning/partitioning.o*

echo Running $SOURCE

echo Cleaning
rm --force $BINARY $RESULTS
rm --force out


echo Compiling
mpic++ $SOURCE -std=c++1y -o $BINARY


echo Running
qsub partitioning.pbs # enqueue
qstat # print queue


echo Waiting for results
while [ ! -f $RESULTS ]; do sleep 0.1; done

printf "Results found\n\n"
sort $RESULTS > out