#!/bin/bash

# for each TLD
for arg in "$@"  
do
    count=0
    # for each .out file in /tmp
    for file_path in /tmp/*.out; do 
        # read each line of the file
        while read line; do
            # get first word of line -> location
            url=$(echo "$line" | awk '{print $1;}')
            # get second word of line -> occurences
            occurences=$(echo "$line" | awk '{print $2;}')
            # tokenize by . , get the last token and compare to the given TLD
            if [ ${url##*.} = $arg ] 
            then
                # increment counter
                count=$((count+$occurences))
            fi
        done <${file_path}
    done
    echo "$arg found $count times";
done