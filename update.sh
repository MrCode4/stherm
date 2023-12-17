#!/bin/bash

# Check if arguments are provided
if [ -z "$2" ]; then
    echo "Source directory not provided."
    exit 1
fi
if [ -z "$3" ]; then
    echo "Destination directory not provided."
    exit 1
fi

# Close the Qt application
touch "$destDir/quit.flag"
sleep 2


# Set source and destination directories from arguments
sourceDir="$2"
destDir="$3"

# Display source and destination directories
echo "Source directory: $sourceDir"
echo "Destination directory: $destDir"

# Perform file copy operation from source to destination
cp -r "$sourceDir"/* "$destDir"/

# Run the updated app
# Change directory to the destination folder
cd "$destDir" || exit
# Run the application executable (replace 'Stherm' with the actual executable name)
./Stherm

# Exit with success code
exit 0
