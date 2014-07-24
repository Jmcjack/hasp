#!/bin/sh
INT_FILE=~/interruptLog.txt

# continuously send dmesg to interruptLog.txt

while true;
do
	dmesg -c -t > "$INT_FILE"

	# look for one of three characters then do something
	
	read -r -n1 c < "$INT_FILE"
	
	if [[ "$c" == "$" ]]; then
		echo $c
		# ./event_AB
	fi
	if [[ "$c" == "#" ]]; then
                echo $c
                ./event_A
        fi
	if [[ "$c" == "&" ]]; then
                echo $c
                # ./event_B
        fi
done
