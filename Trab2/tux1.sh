#!/bin/bash
/etc/init.d/networking restart
ifconfig eth0 up
ifconfig eth0 172.16.60.1/24
route add -net default gw 172.16.60.254
