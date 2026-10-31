#!/bin/bash

if [ "$EUID" -ne 0 ]
then echo "Please run as root."
    exit
fi

echo "adding centaur executable..."
cp centaur /usr/bin/centaur

echo "creating necessary directories..."
mkdir -p /var/db/centaur/packages/{scripts,installed,manifests}
mkdir /var/db/centaur/{tmp,doc}

echo "adding package scripts and hashes..."
cp -r packages/* /var/db/packages/scripts

echo "DONE!!! Use 'centaur' in the terminal to get started!"
