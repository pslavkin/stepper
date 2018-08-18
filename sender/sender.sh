#!/bin/bash
while IFS='' read -r line || [[ -n "$line" ]]; do
    echo $line;
    if read -r ack <out; then
      echo $ack >&2;
    fi;
done < "$1"
