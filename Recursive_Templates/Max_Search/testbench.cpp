#include <iostream>
#include <cstdlib>
#include <ctime>

#include "max_search.hpp"

// Number of registers
static const int N_REGS = 4;

void test_max(int din[N_REGS], int &dout){
  dout = max<N_REGS>(din);
}

CCS_MAIN(int argc, char* argv[])
{
  // RNG Initialization
  std::srand(1234);

  // Declarations
  int din[N_REGS];
  int dout;

  // Initialization
  INIT_VALS:for (int i=0; i<N_REGS; ++i){
    din[i] = std::rand() % 32;
    std::cout << din[i] << " ";
  }
  std::cout << std::endl;

  // Run test
  test_max(din, dout);
  std::cout << dout << std::endl;

  CCS_RETURN(0);
}