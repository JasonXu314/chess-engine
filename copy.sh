#! /bin/bash

DIR=$(pwd)
cd /home/jason/cs/cs5400/chess-engine
cp board.cpp chess.cpp constants.cpp *.h $DIR/$1
cd $DIR