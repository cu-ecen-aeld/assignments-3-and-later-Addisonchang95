#!/bin/bash

#return 1 when arguments are not specified
if [ $# -ne 2 ] 
then
    echo "The number of arguments should be 2, please specify your arguments."
    exit 1
fi

FILESDIR=$1
SEARCHSTR=$2

#return 1 when directory is not exist
if [ ! -d "$FILESDIR" ]
then
    echo "The first argument should be a directory."
    exit 1
fi

#using loop to find matching files
X=0
Y=0
for file in $(find "$FILESDIR" -type f)
do
    X=$((X+1))
    Y=$(( $(grep -c "${SEARCHSTR}" "${file}") + Y))
done

echo "The number of files are ${X} and the number of matching lines are ${Y}"
