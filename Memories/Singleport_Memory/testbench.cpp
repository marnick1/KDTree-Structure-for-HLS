#include <iostream>
#include <cstdlib>
#include <ctime>

#include "channel_interface.hpp"

static const bool Debug = true;   // Debug messages to console
static const int RUNS = 25;       // Number of test cases for each scenario
static const int rng_limit = 128; // Upper limit for random number generator

// Ticketing order
int new_ticket()
{
  static int ticket = 0;
  ticket = ticket + 1;
  return ticket;
}

// Evaluating success of memory operations
template<typename T, int AddrBits>
void results(MemoryIn<int, AddrBits> input, MemoryOut<int> output, bool flag, T *memory)
{
  if (flag && Debug){
    if (input.rw){
      std::cout << input.ticket << "\t";
      std::cout << (input.data==memory[input.address]);
      std::cout << std::endl;
    } else {
      std::cout << input.ticket << "\t";
      std::cout << (output.data==memory[input.address]);
      std::cout << std::endl;
    }
  } else if (!flag && Debug){
    std::cout << input.ticket << "\t";
    std::cout << "no-op";
    std::cout << std::endl;
  }
}

CCS_MAIN(int argc, char* argv[]){
  // RNG Initialization
  std::srand(1234);

  // Hardware parameters
  static const int AddressBits = 8;
  static const int MemorySize = 1<<AddressBits;

  // Memory Allocation
  int memory[MemorySize];

  // Channels for memory operations
  ac_channel<MemoryIn<int, AddressBits> > in;
  ac_channel<MemoryOut<int> > out;

  // Arrays to write to channels
  MemoryIn<int, AddressBits> input;
  MemoryOut<int> output;

  // Testbench

  // Scenario 1: Test writes
  std::cout << "Testing writes..." << std::endl;
  for (int i=0; i<RUNS; ++i){
    input.address = i;
    input.data = std::rand() % rng_limit;
    input.rw = 1;
    input.ticket = new_ticket();
    in.write(input);
    singleport_memory(in, out, memory);
    bool flag = out.nb_read(output);
    results(input, output, flag, memory);
  }
  std::cout << std::endl;

  // Scenario 2: Test reads
  std::cout << "Testing reads..." << std::endl;
  for (int i=0; i<RUNS; ++i){
    input.address = i;
    // input.data = xxx;
    input.rw = 0;
    input.ticket = new_ticket();
    in.write(input);
    singleport_memory(in, out, memory);
    bool flag = out.nb_read(output);
    results(input, output, flag, memory);
  }
  std::cout << std::endl;

  // Scenario 3: Random operations (with bubbles)
  std::cout << "Testing random operations..." << std::endl;
  for (int i=0; i<RUNS; ++i){
    input.address = i;
    input.data = std::rand() % rng_limit;
    input.rw = std::rand() % 2;
    input.ticket = new_ticket();
    int tmp = std::rand() % 8;
    if (tmp<5){  // Read or Write
      in.write(input);
    }             // Else bubble (no operation)
    singleport_memory(in, out, memory);
    bool flag = out.nb_read(output);
    results(input, output, flag, memory);
  }
  std::cout << std::endl;

  CCS_RETURN(0);
}