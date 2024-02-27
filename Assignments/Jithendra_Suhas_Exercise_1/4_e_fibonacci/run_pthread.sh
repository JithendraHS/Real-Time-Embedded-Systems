#!/bin/bash

# Run make
make

#Run the Pthread executable with sudo
echo "Jithu@8971" | sudo -S ./pthread

#Display the last 25 lines of /var/log/syslog
tail -n 25 /var/log/syslog
