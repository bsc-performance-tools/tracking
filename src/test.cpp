#include "CallersTracker.h"

int main(int argc, char **argv)
{

  CallersTracker *CT = new CallersTracker();

  Histo3D *H = CT->Compute3D("/home/bsc41/bsc41127/RUNS/BT/BT.16.S.chop1.clustered.prv", "/home/bsc41/bsc41127/projects/Tracking/tests/CallersLvl2.cfg");
}

