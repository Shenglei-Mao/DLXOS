#include "usertraps.h"

void main (int x)
{
  Printf("Hello World!\n");
  Printf("Current pid is: %d\n", Getpid());
  while(1); // Use CTRL-C to exit the simulator
}
