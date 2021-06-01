#ifndef MY_MEMORY_INTERFACE
#define MY_MEMORY_INTERFACE

#include "ac_int.h"
#include "ac_channel.h"

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
template<typename T, int AddrBits>
void singleport_memory(
  ac_channel<MemoryIn<T,AddrBits> > &in,
  ac_channel<MemoryOut<T> > &out,
  T memory[1<<AddrBits]
){
  // Internal variables
  MemoryIn<T,AddrBits> input;
  MemoryOut<T> output;

  // Read input channel (non-blocking)
  bool flag = in.nb_read(input);

  // If there was an input
  if (flag){

    // Execute memory operation
    if (!input.rw){    // Read operation
      output.data = memory[input.address];
    } else {        // Write operation
      memory[input.address] = input.data;
    }
    output.ticket = input.ticket;

    // Write output channel (blocking)
    out.write(output);
  }
}

#endif
