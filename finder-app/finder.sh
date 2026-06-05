#!/bin/bash

#return 1 when arguments are not specified
if [ $# -ne 2 ] 
then
    echo "The number of arguments should be 2, please specify your arguments."
    echo "Usage: ./finder.sh <directory> <search string>"
    exit 1
fi

FILESDIR=$1
SEARCHSTR=$2

#return 1 when directory is not exist
if [ ! -d "$FILESDIR" ]
then
    echo "The first argument should be a directory existing."
    exit 1
fi

#using loop to find matching files
X=$(find "$FILESDIR" -type f | wc -l)
Y=$(grep -r "$SEARCHSTR" "$FILESDIR" | wc -l)

echo "The number of files are ${X} and the number of matching lines are ${Y}"
