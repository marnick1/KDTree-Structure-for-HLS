#ifndef MY_CHANNEL_INTERFACE
#define MY_CHANNEL_INTERFACE

#include "flat_crossbar.hpp"
#include "singleport_memory_interface.hpp"

#include "mc_scverify.h"

// Struct for memory banks
template<typename T, int AddrBits, int Banks>
struct Memory;

// Partial Specialization
template<typename T, int AddrBits>
struct Memory<T,AddrBits,4>
{
  T bank0[1<<AddrBits];
  T bank1[1<<AddrBits];
  T bank2[1<<AddrBits];
  T bank3[1<<AddrBits];
  T* get_bank(const int &i){
    switch (i){
    case 0:
      return bank0;
    case 1:
      return bank1;
    case 2:
      return bank2;
    default:        // case 3
      return bank3;
    }
  }
};

#pragma hls_design top
template<typename T, int AddrBits, int Ports, int Banks>
void CCS_BLOCK(pseudo_multiport_memory)(
  ac_channel< Flit< MemoryIn<T,AddrBits>, Banks> > in[Ports],
  ac_channel< Flit< MemoryOut<T>,         Banks> > out[Ports],
  Memory<T,AddrBits,Banks> &mem
){
  // Internal Channels
  static ac_channel< Flit< MemoryIn<T,AddrBits>,Ports> > internal_in[Banks];
  static ac_channel< Flit< MemoryOut<T>,        Ports> > internal_out[Banks];

  // Crossbar from In_Ports to Banks
  static FlatCrossbar<MemoryIn<T,AddrBits>, Ports, Banks> xbar1;
  xbar1.run(in, internal_in);

  // Multiple singleport interfaces
  #pragma unroll yes
  for (int i=0; i<Banks; ++i){
    singleport_memory(internal_in[i], internal_out[i], mem.get_bank(i));
  }

  // Crossbar from Banks to Out_Ports
  static FlatCrossbar<MemoryOut<T>, Banks, Ports> xbar2;
  xbar2.run(internal_out, out);
}

#endif
