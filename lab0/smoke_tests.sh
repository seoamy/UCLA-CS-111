#!/bin/bash
# Name: Amy Seo
# Email: amyseo@g.ucla.edu
# ID: 505328863

# expected return codes
EXIT_OK=0
EXIT_ARG=1
EXIT_BADIN=2
EXIT_BADOUT=3
EXIT_FAULT=4
SIG_SEGFAULT=139

echo "... cleaning"
`make clean`

echo "... building executable"
`make`

# input and output -> file comparison
echo "... testing for correct copying of input file to output file"
touch a.txt
echo "hello this is a test !@$" > 'a.txt'

`./lab0 --input="a.txt" --output="b.txt"`

if [ $? -eq $EXIT_OK ]
then
	echo "Sucessfully exited with exit code 0"
else
	echo "Failed to exit with exit code 0"
fi

`cmp a.txt b.txt`
if [ $? -eq $EXIT_OK ]
then
	echo "Succesfully copied input to output with exit code 0"
else
	echo "Failed to copy input file to output file"
fi

# invalid argument --> exit code 1
echo "... testing invalid argument"
`./lab0 --hello`
if [ $? -eq $EXIT_ARG ]
then
	echo "Successfully caught invalid argument with exit code 1"
else
	echo "Failed to exit with exit code 1"
fi

# incorrect input file given --> exit code 2
echo "... test for invalid input file"
`./lab0 --input='abc.txt'`
if [ $? -eq $EXIT_BADIN ]
then
	echo "Successfully caught invalid input file with exit code 2"
else
	echo "Failed to exit with exit code 2"
fi

# invalid output file --> exit code 3
touch c.txt
chmod a-w c.txt
echo "... testing writing to an invalid output file"
`./lab0 --input='a.txt' --output='c.txt'`

if [ $? -eq $EXIT_BADOUT ]
then
	echo "Succesfully unable to write to output file with exit code 3"
else
	echo "Failed to exit with exit code 3"
fi

# catch segfault --> exit code 4
echo "... testing segfault with catch"
`./lab0 --segfault --catch`

if [ $? -eq $EXIT_FAULT ]
then
	echo "Succesfully caught segfault with exit code 4"
else
	echo "Failed to catch segfault"
fi

# segfault --> exit code 139
echo "... testing segfault"
`./lab0 --segfault`
if [ $? -eq $SIG_SEGFAULT ]
then
	echo "Succesfull segfault"
else
	echo "Failed to segfault"
fi

echo "... removing test txt files"
rm -rf a.txt b.txt c.txt

echo "... SMOKE TESTS COMPLETED."