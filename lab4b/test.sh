# NAME: ZHENGTONG LIU
# ID: 505375562
# EMAIL: ericliu2023@g.ucla.edu

#!/bin/bash

./lab4b --bogus &>/dev/null
if [ $? -ne 1 ]
then
    echo "... error: invalid options"
else
    echo "... pass: invalid options"
fi

./lab4b --period=0 &>/dev/null
if [ $? -ne 1 ]
then
    echo "... error: invalid arguments"
else
    echo "... pass: invalid arguements"
fi

./lab4b --scale=S &>/dev/null
if [[$? -ne 1 ]
then
    echo "... error: invalid arguments"
else
    echo "... pass: invalid arguements"
fi

./lab4b --period=1 --scale=C --log="LOG_FILE" <<-EOF
SCALE=F
PERIOD=3
STOP
START
LOG test
OFF
EOF
if [ $? -ne 0 ]
then
    echo "... error: something wrong with commands from stdin"
else
    echo "... pass: commands from stdin 1"
fi

if [ ! -s LOG_FILE ]
then
    echo "... error: did not create log file"
else
    echo "... pass: created log file"
fi

for c in SCALE=F PERIOD=3 START STOP OFF SHUTDOWN "LOG test"
	do
		grep "$c" LOG_FILE > /dev/null
		if [ $? -ne 0 ]
		then
			echo "... error: DID NOT LOG $c command"
		else
			echo "... pass:    $c ... RECOGNIZED AND LOGGED"
		fi
done

rm -rf LOG_FILE

