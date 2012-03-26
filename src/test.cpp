#include "SequenceTracker.h"

int main(int argc, char **argv)
{
  SequenceTracker *SS1 = new SequenceTracker("align1", "clusters_info1");
  SequenceTracker *SS2 = new SequenceTracker("align1", "clusters_info1");


  map<INT32, INT32> Unique;
  Unique[1] = 1;
  Unique[2] = 2;

  ClusterCorrelationMatrix * CCM = SS1->CorrelateWithAnother( Unique, SS2 );
  CCM->Print();
  CCM->Stats();
}
