#!/bin/bash
if [ -z "$1" ]; then
	echo "usage: $0 \"HEX-STRING\""
	exit 1
fi

INSTRING="$1"
INSTRINGLEN=${#INSTRING}
LOOPCNT=$((INSTRINGLEN/2))

i=0;
echo -n "char* arr[${LOOPCNT}] = {"

while [ $i -ne $LOOPCNT ]; do
	CUTVAR=$((i*2))
	echo -n "0x"
	echo -n ${INSTRING:$CUTVAR:2}
	i=$((i+1))
	if [ $i -ne $LOOPCNT ]; then echo -n ", "; fi
done
echo "}"
