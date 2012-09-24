#!/bin/bash -v

echo "Running Setup Script On Server!"
sudo umount /mnt
sudo /usr/testbed/bin/mkextrafs -f /mnt || sudo mount -a
sudo chown $USER /mnt
