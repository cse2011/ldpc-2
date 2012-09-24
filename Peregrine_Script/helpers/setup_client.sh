#!/bin/bash -v 

echo "Running Setup Script On Client!"
sudo umount /mnt
sudo /usr/testbed/bin/mkextrafs -f /mnt || sudo mount -a
sudo chown $USER /mnt
