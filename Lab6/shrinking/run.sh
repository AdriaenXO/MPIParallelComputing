#! /usr/bin/env bash
set -ea

SOURCE=${1:-shrinking.cpp}
BINARY=~/lab6/shrinking/shrinking
RESULTS=~/lab6/shrinking/shrinking.o*

echo Running $SOURCE

echo Cleaning
rm --force $BINARY $RESULTS
rm --force out


echo Compiling
mpic++ $SOURCE -std=c++1y -o $BINARY


echo Running
qsub shrinking.pbs # enqueue
qstat # print queue


echo Waiting for results
while [ ! -f $RESULTS ]; do sleep 0.1; done

printf "Results found\n\n"
sort $RESULTS > out