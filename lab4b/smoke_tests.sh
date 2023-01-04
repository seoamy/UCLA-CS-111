#!/bin/bash
# Name: Amy Seo
# Email: amyseo@g.ucla.edu
# ID: 505328863

EXIT_OK=0

# testing for correct output format
echo "... running lab4b executable"
touch a.txt
touch b.txt
timeout 3s ./lab4b --log=a.txt > /dev/null

`grep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9][0-9]\.[0-9]' a.txt > b.txt`
`cmp a.txt b.txt`
if [ $? -eq $EXIT_OK ]
then
	echo "Succesfully reported formatted time and temperature"
else
	echo "Failed to report time and temperature in correct format"
fi

# testing for stdin commands logging to logfile
touch c.txt
timeout 3s ./lab4b --log=c.txt >/dev/null 2>/dev/null <<-EOF
SCALE=C
PERIOD=3
LOG HELLO WORLD
RANDOM STUFF
EOF

SCALE_RESULT=`grep "SCALE=C" c.txt`
PERIOD_RESULT=`grep "PERIOD=3" c.txt`
LOG_RESULT=`grep "LOG HELLO WORLD" c.txt`
if [ "$SCALE_RESULT" == "SCALE=C" ]
then
    echo "Succesfully logged SCALE change."
else
    echo "Failed to log SCALE change."
fi

if [ "$PERIOD_RESULT" == "PERIOD=3" ]
then
    echo "Succesfully logged PERIOD change."
else
    echo "Failed to log PERIOD change."
fi

if [ "$LOG_RESULT" == "LOG HELLO WORLD" ]
then
    echo "Succesfully logged LOG statement."
else
    echo "Failed to log LOG statement."
fi

echo "... removing test txt files"
rm -rf *.txt

echo "... SMOKE TESTS COMPLETED."