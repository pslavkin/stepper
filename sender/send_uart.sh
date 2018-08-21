cat sec1.txt | ./sender_uart.sh > /dev/ttyACM1 &
cat /dev/ttyACM1 > fb_fifo &
cat <> ack_fifo
