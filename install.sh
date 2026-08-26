#!/bin/bash

if [ "$EUID" -ne 0 ]
then echo "Please run as root."
    exit
fi

echo "adding centaur executable..."
mv centaur /usr/bin/centaur

echo "creating necessary directories..."
mkdir -p /etc/centaur/packages/{scripts, installed, uninstall, world}
mkdir /etc/centaur/{tmp, doc}

echo "adding package scripts and hashes..."
mv packages/* /etc/centaur/packages/scripts

echo "DONE!!! Use 'centaur' in the terminal to get started!"
