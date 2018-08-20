cat sec1.txt | ./sender_uart.sh > /dev/ttyACM0 &
cat /dev/ttyACM0 > fb_fifo &
cat <> ack_fifo
