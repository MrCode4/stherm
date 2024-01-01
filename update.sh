#!/bin/bash

# exit with 1: restart
#           0: success, stop the service

# Set source and destination directories from arguments
sourceDir="/mnt/update/latestVersion"
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


rm -rf "content"
unzip "update.zip" -d "content"
cd "content"
gunzip *

echo "stopping service"
systemctl stop appStherm.service
echo "service stopped"

sleep 10

echo "copying ... "

# Perform file copy operation from source to destination
cp -r * "$destDir"/


echo "cleaning up "

rm -rf $sourceDir/*

echo "starting service"
sleep 2
systemctl start appStherm.service

# Exit with success code
exit 0
