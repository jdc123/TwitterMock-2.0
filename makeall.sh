#!/bin/sh
cp server.c ./rep1 
cp server.c ./rep2
make
make -C ./rep1
make -C ./rep2
