#include "project.h"
#include <fstream>
#include <cstdlib>

simpleGraph mygraph;
#if defined(__AIESIM__) || defined(__X86SIM__)
int main(int argc, char **argv) 
{


  mygraph.init();
  mygraph.run(1);
  mygraph.wait();

  mygraph.end();
  return 0;
} 
#endif


