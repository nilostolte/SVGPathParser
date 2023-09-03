#!/bin/bash
gcc SVGparser.c -std=c99 -lm
status=$?
if [ $status -ne 0 ]
then 
	exit $status;
fi
mv a.out a.exe

