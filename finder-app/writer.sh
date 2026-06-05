#!/bin/bash

#return 1 when arguments are not specified
if [ $# -ne 2 ] 
then
    echo "The number of arguments should be 2, please specify your arguments."
    echo "Usage: ./writer.sh <file path> <string to write>"
    exit 1
fi

WRITEFILE=$1
WRITESTR=$2

# 3. get the parent directory of the file path
PDIR=$(dirname "$WRITEFILE")

# 4. create the parent directory if it does not exist
if [ ! -d "$PDIR" ]; then
    mkdir -p "$PDIR"
fi

# 5. try to write to the file, if it fails then print an error and exit
if ! echo "$WRITESTR" > "$WRITEFILE"; then
    echo "Error: Failed to create or write to file $WRITEFILE"
    exit 1
fi