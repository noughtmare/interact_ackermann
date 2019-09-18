#!/bin/sh
make clean
make all
time -v ./interact_ackermann_call_stack
time -v ./interact_ackermann-hs
