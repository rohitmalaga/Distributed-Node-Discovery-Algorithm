#!/bin/bash


# Change this to your netid
netid=rxm145830

#
# Root directory of your project
PROJDIR=$HOME/AOS/Project1

#
# This assumes your config file is named "config.txt"
# and is located in your project directory
#
CONFIG=$PROJDIR/config_rohit.txt

#
# Directory your java classes are in
#
BINDIR=$PROJDIR/bin

#
# Your main project class
#
PROG=Project1
isNodeInfo=0
doneInfo=0
node=0
n=1

cat $CONFIG | sed -e "s/#.*//" | sed -e "/^\s*$/d" |
(
    read i
    echo $i
    while read line 
    do
		if [ $isNodeInfo == 1 ] && [ $doneInfo == 0 ]  
		then
			echo $line
		arr=$(echo $line | tr ";" "\n")

		for x in $arr
		do
			echo $x
			ssh $netid@$x killall -u $netid &
			node=$((node+1))
		done
		doneInfo=1
		fi
		if [[ $line == *"--Start of Node Info--"* ]] 
		then
			isNodeInfo=1
		fi
    done
   
)


echo "Cleanup complete"
