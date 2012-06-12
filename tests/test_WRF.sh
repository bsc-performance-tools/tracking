#!/bin/bash

export TRACKING_HOME=$HOME/apps/TRACKING/64

$TRACKING_HOME/bin/tracking -v -c 2 -f cl_IPC -o WRF /home/bsc41/bsc41127/gscratch/RUNS/WRF/WRF.MN.128p.chop2.clustered.prv /home/bsc41/bsc41127/gscratch/RUNS/WRF/WRF.MN.256p.chop2.clustered.prv

