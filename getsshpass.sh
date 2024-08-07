#!/bin/bash

# exit with 1, 2: restart
#              0: success, stop the service

# Set source and destination directories from arguments
sourceDir="/usr/local/bin"
destDir="sshpass_dl"
exitCode=0

# Display source and destination directories
echo "Source directory: $sourceDir"
echo "Destination directory: $destDir"

cd "$sourceDir"

mkdir -p "$destDir"

cd "$destDir"

echo "Downloading sshpass in $destDir"

wget http://fileserver.nuvehvac.com/files/sshpass-1.05-2-armv7h.pkg.tar.xz

if [ -f "sshpass-1.05-2-armv7h.pkg.tar.xz" ]; then
    echo "uncomressing sshpass-1.05-2-armv7h.pkg.tar.xz in $destDir"
    tar -xvf "sshpass-1.05-2-armv7h.pkg.tar.xz"

    if [ -f "usr/bin/sshpass" ]; then
        echo "applying access rules"
        cd "usr/bin"
        chmod +x "sshpass"

        echo "copying sshpass to /usr/bin"
        cp "sshpass" "/usr/bin/sshpass"
    else
        echo "err: sshpass not exists in $destDir/usr/bin"
        exitCode=2
    fi

else
    echo "err: sshpass-1.05-2-armv7h.pkg.tar.xz does not exists in $destDir"
    exitCode=1
fi


echo "cleaning up"
cd "$sourceDir"
rm -rf "$destDir"

echo "Exit with code $exitCode, 0 means success, positive value has error and needs retry"
exit $exitCode
