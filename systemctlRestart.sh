#!/bin/bash

# exit with 0: restart

echo "stopping appStherm"
systemctl stop appStherm
echo "appStherm stopped"

sleep 10

echo "start appStherm"
systemctl start appStherm

# Exit with success code
exit 0
