#!/bin/bash

# exit with 1: restart
#           0: success, stop the service

# Set source and destination directories from arguments
sourceDir="/usr/local/bin"
destDir="sshpass_dl"

# Display source and destination directories
echo "Source directory: $sourceDir"
echo "Destination directory: $destDir"

cd "$sourceDir"

mkdir -p "$destDir"

cd "$destDir"

echo "Downloading sshpass in $destDir"

wget http://fileserver.nuvehvac.com/files/sshpass-1.05-2-armv7h.pkg.tar.xz

echo "uncomressing sshpass-1.05-2-armv7h.pkg.tar.xz in $destDir"

tar -xvf "sshpass-1.05-2-armv7h.pkg.tar.xz"

echo "applying access rules"
cd "usr/bin"
chmod +x "sshpass"

echo "copying sshpass to /usr/bin"
cp "sshpass" "/usr/bin/sshpass"

echo "cleaning up"
cd "$sourceDir"
rm -rf "$destDir"

echo "Exit with success code"
exit 0
