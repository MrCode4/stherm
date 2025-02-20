#!/bin/bash

# exit with 1: restart
#           0: success, stop the service

# Set source and destination directories from arguments
sourceDir="/mnt/update/nrf_fw"
destDir="/usr/local/bin"

# Display source and destination directories
echo "Source directory: $sourceDir"
echo "Destination directory: $destDir"

# https://man7.org/linux/man-pages/man1/mountpoint.1.html
if mountpoint -q /mnt/update; then
    echo "/mnt/update already mounted"
else
    # Create the directory using 'mkdir -p' if it doesn't exist.
	mkdir -p /mnt/update
	mount /dev/mmcblk1p3 /mnt/update
	
	 # Check if the mount was successful
	 # check the exit status of the previous command executed in the script
    if [ $? -eq 0 ]; then
        echo "/mnt/update mounted successfully"
    else
        echo "Failed to mount /mnt/update"
        exit 0
    fi
fi

cd "$sourceDir"

if [ -f "update.zip" ]; then
    echo "File 'update.zip' exists."
else
    echo "File 'update.zip' does not exist."
	exit 0
fi

echo "stopping watchdog process"
pkill watchdog_stherm
echo "watchdog process stopped"

echo "stopping service"
systemctl stop appStherm.service
echo "service stopped"

sleep 10

echo "copying ... "

# Perform file copy operation from source to destination
printf '\xf0\x4b\x00\x01\x00\x00\xf1' > /dev/ttymxc1
nrfdfu serial --port /dev/ttymxc1 /mnt/update/nrf_fw/update.zip

echo "cleaning up "

rm -rf $sourceDir/*

echo "starting service"
sleep 2
systemctl start appStherm.service

# Exit with success code
exit 0
