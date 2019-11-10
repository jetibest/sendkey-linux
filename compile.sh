#!/bin/bash

if ! [ -e src/main.cpp ]
then
	echo "First cd to the project directory (sendkey-linux/)."
	exit 1
fi

if ! [ -d bin ]
then
	mkdir bin
fi

if [ -e bin/sendkey ]
then
	rm bin/sendkey
fi

exec g++ -std=c++11 -pthread src/*.cpp -o bin/sendkey
