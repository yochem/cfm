#!/usr/bin/env sh

git pull

board="arduino:avr:mega"

arduino-cli compile -b $board "arduino/" && arduino-cli upload -b $board -p /dev/tty.usb* "arduino/"
