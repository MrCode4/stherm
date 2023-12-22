#!/bin/bash

# Set source and destination directories from arguments
sourceDir="/mnt/update/latestVersion"
destDir="/usr/local/bin"

# Display source and destination directories
echo "Source directory: $sourceDir"
echo "Destination directory: $destDir"

cd "$sourceDir"
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

rm -rf "$sourceDir/*"

echo "starting service"
sleep 2
systemctl start appStherm.service

# Exit with success code
exit 0
