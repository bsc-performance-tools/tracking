#!/bin/bash

export TRACKING_HOME=$HOME/apps/TRACKING/64

$TRACKING_HOME/bin/tracking -v -m 1 -c 2 -f cl_IPC -o WRF /home/bsc41/bsc41127/RUNS/WRF/WRF.MN.128p.chop2_clustered.prv /home/bsc41/bsc41127/RUNS/WRF/WRF.MN.256p.chop2_clustered.prv



