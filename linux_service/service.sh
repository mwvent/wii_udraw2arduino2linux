#!/bin/bash
#  wii_udraw2arduino2linux
#  By Matthew Watts 2018
#  This just wraps the python script
#  Re-running it if it errors (usually due to serial port not ready)
while :; do
	/usr/bin/python /opt/udraw/driver.py 2>&1
	sleep 5
done
