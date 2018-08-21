#!/bin/bash
while IFS='' read -r line || [[ -n "$line" ]]; do
    echo $line;
    if read ack < fb_fifo; then
      echo "$ack" > ack_fifo;
    fi;
done
