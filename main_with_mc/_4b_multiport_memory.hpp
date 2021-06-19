#ifndef MY_MULTIPORT_MEMORY_INTERFACE
#define MY_MULTIPORT_MEMORY_INTERFACE

#include "_2c_data_structures.hpp"

// Memory Interface
#pragma hls_design interface
template<typename T, int AddrBits, int N>
void singleport_memory(
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

// Round Robin Arbiter
template<int IN>
class RoundRobinArbiter {
  typedef ac_int<ac::log2_ceil<IN>::val,false> bvector; // log2(IN) bits
  typedef ac_int<IN,false> nvector;                     // IN bits
private:
  bvector high_pri;
public:
  // Constructor
  RoundRobinArbiter(): high_pri(0){}
  // Interface
  bool arb(const nvector &req, bvector &pos) {
    bool found_hp=false, found_lp=false;
    bvector gnt_hp=0, gnt_lp=0;
    #pragma unroll yes
    ARB:for(int i=0; i<IN; i++) {
      if (i >= high_pri) {
        if (req[i] && !found_hp) {
          gnt_hp=i;
          found_hp = true;
        }
      } else {
        if (req[i] && !found_lp) {
          gnt_lp=i;
          found_lp = true;
        }
      }
    }
    pos = (found_hp) ? gnt_hp : gnt_lp;
    if (found_hp || found_lp) {
      if (pos==IN-1) {
        high_pri = 0;
      } else {
        high_pri = pos + 1;
      }
    }
    return (found_hp || found_lp);
  }
};

// Crossbar inputs IN-to-OUT outputs, non-blocking, with arbitration
template<typename T, int IN, int OUT>
class FlatCrossbar {
  typedef ac_int<IN,false> nvector;                       // IN bits
  typedef ac_int<ac::log2_ceil<IN>::val,false> bvector;   // log2(IN) bits
  typedef ac_int<OUT,false> mvector;                      // OUT bits
private:
  // Input data - active flag
  nvector in_actv;
  Flit<T,OUT> in_data[IN];
  // Output data - active flag
  mvector out_actv;
  Flit<T,IN> out_data[OUT];
  // Output arbitration
  RoundRobinArbiter<IN> rra[OUT];
public:
  // Constructor
  FlatCrossbar() : in_actv(0), out_actv(0){}
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<Flit<T,OUT> > in[IN],
    ac_channel<Flit<T,IN> > out[OUT]
  ){
    // Read from input depending on active flag
    nvector in_new = 0;
    #pragma unroll yes
    INPUT:for (int i=0; i<IN; ++i) {
      if (!in_actv[i]) {
        in_new[i] = in[i].nb_read(in_data[i]);
      }
    }
    in_actv |= in_new;

    // Generate requests per arbiter
    nvector arb_req[OUT];
    #pragma unroll yes
    OUT_REQ:for (int i=0; i<OUT; ++i) {
      nvector dst_actv;
      #pragma unroll yes
      OUT_SEL:for (int j=0; j<IN; ++j) {
        dst_actv[j] = (in_data[j].dst == i) & (~out_actv[i]);
      }
      arb_req[i] = in_actv & dst_actv;
    }

    // If there is at least one request per output
    // (either previous or current)
    #pragma unroll yes
    OUTPUT:for (int i=0; i<OUT; ++i) {
      bvector gnt;
      if (rra[i].arb(arb_req[i], gnt)) {
        // Send out the data of the winner
        Flit<T,IN> tmp;
        tmp.data = in_data[gnt].data;
        tmp.dst = gnt;
        out_actv[i] = !(out[i].nb_write(tmp));
        out_data[i] = tmp;
        in_actv[gnt] = 0;
      } else if (out_actv[i]){
        // Send data from previous cycles that passed arbitration
        out_actv[i] = !(out[i].nb_write(out_data[i]));
      }
    }
  }
};

// Pseudo multiport memory interface
template<typename T, int AddrBits, int Ports, int Banks>
class PseudoMultiportMemoryController {
private:
  FlatCrossbar<   MemoryIn<T,AddrBits>, Ports, Banks> xbar1;
  FlatCrossbar<   MemoryOut<T>        , Banks, Ports> xbar2;
  ac_channel<Flit<MemoryIn<T,AddrBits>, Ports> > internal_in[Banks];
  ac_channel<Flit<MemoryOut<T>        , Ports> > internal_out[Banks];
public:
  // Constructor
  PseudoMultiportMemoryController(){}
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<Flit<MemoryIn<T,AddrBits>, Banks> > in[Ports],
    ac_channel<Flit<MemoryOut<T>,         Banks> > out[Ports],
    Arrays<T,AddrBits,Banks> &memory
  ){
    xbar1.run(in, internal_in);
    #pragma unroll yes
    for (int i=0; i<Banks; ++i){
      singleport_memory(internal_in[i], internal_out[i], memory.get_array(i));
    }
    xbar2.run(internal_out, out);
  }
};



#endif
