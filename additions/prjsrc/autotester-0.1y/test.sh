# !/bin/sh
# Command farview_mm can only control the loop number of the messages and use default interval
#
sudo ./farview -l 3 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29
while [ 0 = 0 ]
do
        sudo ./farview -l 0 -f 1 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -p 600
#       sudo ./farview -l 0 -f 0 -a -5 -v 390 -c 88 -s 39 -t 5 -h 29 -p 600
        sudo ./farview -l 1 -f 0
done
