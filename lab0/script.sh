#!/bin/bash

# NAME: Zhengtong Liu
# EMAIL: ericliu2023@g.ucla.edu
# ID: 505375562

# check for invalid arguments
echo '... check for invalid arguments'
./lab0 --verbose > /dev/null 2>STDERR
if [ $? -eq 1 ]
then
	echo "Sucess: exit code for unrecognized argument is 1"
else
	echo "Error: exit code for unrecognized arugment should be 1"
fi

if [ ! -s STDERR ]
then
	echo "Error: not detecting invalid arguments"
else
	echo "Success: detected invalid arguments"
fi
echo ''

# check for the default option
echo '... check for the default option'
touch input
chmod +x input
echo 'hello' > input
./lab0 <input >output
if [ $? -eq 0 ]
then
	echo "Sucess: exit code for copying sucessfully is 0"
else
	echo "Error: exit code for copying sucessfully should be 0"
fi
cmp input output
if [ $? -eq 0 ]
then
	echo "Success: copied stdin to stdout"
else
	echo "Error: stdout does not match stdin"
fi
echo ''

# check for the input and output options
echo '... check for the input and output options'
./lab0 --input=input --output=output1
if [ $? -eq 0 ]
then
	echo "Sucess: exit code for copying sucessfully is 0"
else
	echo "Error: exit code for copying sucessfully should be 0"
fi
cmp input output1
if [ $? -eq 0 ]
then
	echo "Success: copied input to output"
else
	echo "Error: output does not match input"
fi
echo ''

# check for the segfault and catch options
echo '... check for the segfault option'
./lab0 --segfault --catch 
if [ $? -eq 4 ]
then
	echo "Success: catch the segfault and exit code is 4"
else
	echo "Error: should cause and catch the segfault with exit code 4"
fi
echo ''

# check for two other exit codes: 2 and 3
echo '... check for two other exit codes: 2 and 3'
./lab0 --input=hello --output=output
if [ $? -eq 2 ]
then
	echo "Success: unable to open the input file and exit code is 2"
else
	echo "Error: exit code should be 2 when unable to open the input file"
fi
touch output2
chmod -w output2
./lab0 --input=input --output=output2
if [ $? -eq 3 ]
then
	echo "Success: unable to open the output file and exit code is 3"
else
	echo "Error: exit code should be 3 when unable to open the output file"
fi
echo ''

rm -rf STDERR input output output1 output2