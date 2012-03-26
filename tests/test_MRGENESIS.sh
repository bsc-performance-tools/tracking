#!/bin/bash

export TRACKING_HOME=$HOME/apps/TRACKING/64

$TRACKING_HOME/bin/TrackClusters -v -m 1 -c 1 -f cl_IPC -o MR_GENESIS /home/bsc41/bsc41127/gscratch/traces/TrackingTestBed/MR_GENESIS/RUN/MR_GENESIS.128.chop1.clustered.prv /home/bsc41/bsc41127/gscratch/traces/TrackingTestBed/MR_GENESIS/RUN/MR_GENESIS.256.chop1.clustered.prv



