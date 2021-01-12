#!/bin/bash

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
./lab0 --input=input --output=output2
if [ $? -eq 0 ]
then
	echo "Sucess: exit code for copying sucessfully is 0"
else
	echo "Error: exit code for copying sucessfully should be 0"
fi
cmp input output2
if [ $? -eq 0 ]
then
	echo "Success: copied input to output"
else
	echo "Error: output does not match input"
fi
echo ''

# check for the segfault and catch options
echo '... check for the segfault option'
./lab0 --segfault --catch 2>STDERR
if [ $? -eq 4 ]
then
	echo "Sucess: catch the segfault: $STDERR"
else
	echo "Error: should cause and catch the segfault with exit code 4"
fi
echo ''

rm STDERR input output output2