#!/bin/bash

path=$1
ACTIVE=0
INACTIVE=0

echo $path

for _dir in $path/*;
do
	if [ -e $_dir/_boardtoserver ] && [ -e $_dir/pid.txt ] && [ -e $_dir/ ]
	then
		ACTIVE=$((ACTIVE + 1))
	elif [ -e $_dir/ ]
	then
		INACTIVE=$((INACTIVE + 1))
	fi
done

echo $ACTIVE active boards
for _dir in $path/*;
do
	if [ -e $_dir/_boardtoserver ] && [ -e $_dir/pid.txt ] && [ -e $_dir/ ]
	then
		echo $_dir
	fi
done

echo $INACTIVE inactive boards
for _dir in $path/*;
do
	if [ ! -e $_dir/_boardtoserver ] && [ ! -e $_dir/pid.txt ] && [ -e $_dir/ ]
	then
		echo $_dir
	fi
done
