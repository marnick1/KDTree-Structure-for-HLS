#include <iostream>
#include <cstdlib>
#include <ctime>

#include "multiport_memory.hpp"

#include "mc_scverify.h"

static const bool Debug = true;   // Debug messages to console
static const int RUNS = 25;       // Number of test cases for each scenario
static const int rng_limit = 100; // Upper limit for random number generator
static const int scenario = 1;

#pragma hls_design top
template<typename T, int AddrBits, int Ports, int Banks>
void CCS_BLOCK(run)(
  ac_channel<Flit<MemoryIn<T,AddrBits>, Banks> > in[Ports],
  ac_channel<Flit<MemoryOut<T>,         Banks> > out[Ports],
  Memory<T,AddrBits,Banks> &memory
){
  static MultiportMemory<int, AddrBits, Ports, Banks> mc;
  mc.run(in, out, memory);
}

// Ticketing order
int new_ticket()
{
  static int ticket = 0;
  ticket = ticket + 1;
  return ticket;
}

// Evaluating success of memory operations
template<typename T, int AddrBits, int Ports, int Banks>
void results(
  Flit< MemoryIn<T, AddrBits>, Banks> input[Ports],
  bool in_flag[Ports],
  Flit< MemoryOut<T>,          Banks> output[Ports],
  bool out_flag[Ports],
  Memory<T, AddrBits, Banks> memory
){
  if (Debug){
    std::cout << "New Cycle: " << std::endl;
    for (int i=0; i<Ports; ++i){
      std::cout << i << "\t";
      if (in_flag[i]){
        std::cout << "Ticket in: ";
        std::cout << input[i].data.ticket  << "\t";
        if (input[i].data.rw){    // Write operation
          std::cout << input[i].data.data << " write @ ";
          std::cout << int(input[i].dst) << " " << int(input[i].data.address) << "   \t\t";
        } else {                  // Read operation
          std::cout << "Data read @ ";
          std::cout << int(input[i].dst) << " " << int(input[i].data.address)<< " \t\t";
        }
      } else {
        std:: cout << "------------------------------- \t\t";
      }
      if (out_flag[i]){
        std::cout << "Ticket out: ";
        std::cout << output[i].data.ticket << "\t";
        std::cout << output[i].data.data;
      } else {
        std:: cout << "------------";
      }
      std::cout << std::endl;
    }
  }
}

CCS_MAIN(int argc, char* argv[]){
  // RNG Initialization
  std::srand(1234);

  // Hardware specifications
  static const int Ports = 3;
  static const int Banks = 4;
  static const int AddrBits = 14;

  // Generate memory and interface
  Memory<int, AddrBits, Banks> mem;

  // Input & Output Channels
  ac_channel< Flit< MemoryIn< int, AddrBits>, Banks> > in[Ports];
  ac_channel< Flit< MemoryOut< int>,          Banks> > out[Ports];

  // Arrays to write/read channels
  Flit< MemoryIn< int, AddrBits>, Banks> input[Ports];
  bool input_valid[Ports];
  Flit< MemoryOut< int>,          Banks> output[Ports];
  bool output_valid[Ports];

  // TestBench
  if (scenario == 1){
    // Scenario 1: No congestion
    std::cout << "Running scenario 1" << std::endl;
    // Write
    for (int i=0; i<RUNS; ++i) {
      // Input Generation
      for (int j=0; j<Ports; ++j) {
        input[j].dst = (i+j) % Banks;   // Lower Address
        input[j].data.address = i;      // Upper Address
        input[j].data.data = std::rand() % rng_limit;
        input[j].data.rw = 1;
        input[j].data.ticket = new_ticket();
        // Write to channel
        input_valid[j] = true;
        in[j].write(input[j]);
      }
      // Memory interface
      run<int, AddrBits, Ports, Banks>(in, out, mem);
      // Retrieving Results
      for (int j=0; j<Ports; ++j) {
        output_valid[j] = out[j].nb_read(output[j]);
      }
      results<int, AddrBits, Ports, Banks>(input, input_valid, output, output_valid, mem);
    }
    // Read
    for (int i=0; i<RUNS; ++i) {
      // Input Generation
      for (int j=0; j<Ports; ++j) {
        input[j].dst = (i+j) % Banks;   // Lower Address
        input[j].data.address = i;      // Upper Address
        input[j].data.data = std::rand() % rng_limit;
        input[j].data.rw = 0;
        input[j].data.ticket = new_ticket();
        // Write to channel
        input_valid[j] = true;
        in[j].write(input[j]);
      }
      // Memory interface
      run<int, AddrBits, Ports, Banks>(in, out, mem);
      // Retrieving Results
      for (int j=0; j<Ports; ++j) {
        output_valid[j] = out[j].nb_read(output[j]);
      }
      results<int, AddrBits, Ports, Banks>(input, input_valid, output, output_valid, mem);
    }
  } else if (scenario == 2){
    // Scenario 2: Random Operations (with bubbles)
    std::cout << "Running scenario 2" << std::endl;
    // Random ops
    for (int i=0; i<RUNS; ++i) {
      // Input Generation
      for (int j=0; j<Ports; ++j) {
        input[j].dst = std::rand() % Banks;   // Lower Address
        input[j].data.address = i;            // Upper Address
        input[j].data.data = std::rand() % rng_limit;
        input[j].data.rw = std::rand() % 2;
        input[j].data.ticket = new_ticket();
        int tmp = std::rand() % 10;
        // Write to channel
        if (tmp<7){
          input_valid[j] = true;
          in[j].write(input[j]);
        } else {
          input_valid[j] = false;
        }
      }
      // Memory interface
      run<int, AddrBits, Ports, Banks>(in, out, mem);
      // Retrieving Results
      for (int j=0; j<Ports; ++j) {
        output_valid[j] = out[j].nb_read(output[j]);
      }
      results<int, AddrBits, Ports, Banks>(input, input_valid, output, output_valid, mem);
    }
    // No-ops
    for (int i=0; i<RUNS/5; ++i) {
      // No inputs
      for (int j=0; j<Ports; ++j) {
        input_valid[j] = false;
      }
      // Memory interface
      run<int, AddrBits, Ports, Banks>(in, out, mem);
      // Retrieving Results
      for (int j=0; j<Ports; ++j) {
        output_valid[j] = out[j].nb_read(output[j]);
      }
      results<int, AddrBits, Ports, Banks>(input, input_valid, output, output_valid, mem);
    }
  }
  CCS_RETURN(0);
}
