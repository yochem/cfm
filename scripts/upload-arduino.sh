#!/usr/bin/env sh

path=$1

board="-b arduino:avr:mega"

arduino-cli compile $board "$path" && arduino-cli upload $board -p /dev/tty.usb* "$path"
