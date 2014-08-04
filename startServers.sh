#!/bin/sh

port1=$(($1))
port2=$(($1+1))
port3=$(($1+2))

echo $port1";"$port2";"$port3";"> ports.txt
gnome-terminal -e "./server "$port1" "$port2" "$port3 &

cd "rep1";gnome-terminal -e "./server "$port2" "$port1" "$port3 &

cd "../rep2";gnome-terminal -e "./server "$port3" "$port1" "$port2 &
