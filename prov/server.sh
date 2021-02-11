#!/bin/bash

INSTANCE=$1

# Read current settings
source /vagrant/prov/params

# First kill other instances of this script
#sudo pkill -f ${SCRIPT}
# TODO

# Kill the server application
#sudo pkill -f ${PROC}

# If instance already running, exit
#if [[ $(pgrep -f ${SCRIPT}) != $$ ]]; then
#	exit 0
#fi

# Go to server directory
if [[ $INSTANCE == 1 ]]; then
	cd ${TGTPATH_1}
elif [[ $INSTANCE == 2 ]]; then
	cd ${TGTPATH_2}
fi

pid=9999999

# Ensure server instance is kept running
# Interval checking
while [ 1 ]; do 

# Update stored settings
source /vagrant/prov/params

if [ ${STOP} -eq 1 ]; then
	sudo kill $pid
	exit 0
fi

if [ ${RESTART} -eq 1 ]; then
	# sudo pkill -f ${PROC}
	sudo kill $pid
fi

# if [[ $(pgrep -f ${PROC}) == "" ]]; then
if ps -p $pid > /dev/null; then
	: # Do nothing
else
	eval ${COMMAND}
	pid=$!
fi

sleep ${TIMEOUT}

done
