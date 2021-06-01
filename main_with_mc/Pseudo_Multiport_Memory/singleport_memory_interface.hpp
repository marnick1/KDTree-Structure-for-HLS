#ifndef MY_SINGLEPORT_MEMORY_INTERFACE
#define MY_SINGLEPORT_MEMORY_INTERFACE

#include "ac_int.h"
#include "ac_channel.h"

#include "mc_scverify.h"

// Memory Input Variables
template<typename T, int AddrBits>
struct MemoryIn
{
  ac_int<AddrBits,false> address;
  T data;
  bool rw;
  // Testing Timings
  int ticket;
};

// Memory Output Variables
template<typename T>
struct MemoryOut
{
  T data;
  // Testing Timings
  int ticket;
};

// Memory Interface
#pragma hls_design interface
template<typename T, int AddrBits, int N>
void CCS_BLOCK(singleport_memory)(
ac_channel< Flit< MemoryIn<T,AddrBits>, N> > &in,
ac_channel< Flit< MemoryOut<T>,         N> > &out,
T memory[1<<AddrBits])
{
  // Internal variables
  Flit< MemoryIn<T,AddrBits>, N> input;
  Flit< MemoryOut<T>,         N> output;

  // Read input channel (non-blocking)
  bool flag = in.nb_read(input);

  // If there was an input
  if (flag){

    // Execute memory operation
    if (!input.data.rw){    // Read operation
      output.data.data = memory[input.data.address];
    } else {        // Write operation
      memory[input.data.address] = input.data.data;
    }
    output.dst = input.dst;
    output.data.ticket = input.data.ticket;

    // Write output channel (blocking)
    out.write(output);
  }
}

#endif
