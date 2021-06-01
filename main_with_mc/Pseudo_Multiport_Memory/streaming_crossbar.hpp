#ifndef MY_STREAMING_CROSSBAR
#define MY_STREAMING_CROSSBAR

#include "ac_int.h"
#include "ac_channel.h"

#include "round_robin_arbiter.hpp"

#include "mc_scverify.h"

template<typename T, int OUT>
class Split;

template<typename T, int IN>
class Funnel;

template<typename T, int IN, int OUT>
class StreamCrossbar;

// (FL)ow-controlled dig(IT)
template<typename T, int OUT>
struct Flit {
  ac_int<ac::log2_ceil<OUT>::val,false> dst;
  T data;
};

// Flow-controlled splitter 1-to-4
template<typename T>
class Split<T,4> {
public:
  // Constructor
  Split(){};
  // Interface
  void run(
    ac_channel<Flit<T,4> > &in,
    ac_channel<T> &out0,
    ac_channel<T> &out1,
    ac_channel<T> &out2,
    ac_channel<T> &out3
  ){
    // Blocking read input
    Flit<T,4> in_data;
    if (in.available(1)) {
      in.read(in_data);

      // Blocking write output
      switch (in_data.dst){
      case 0:
        out0.write(in_data.data);
        break;
      case 1:
        out1.write(in_data.data);
        break;
      case 2:
        out2.write(in_data.data);
        break;
      case 3:
        out3.write(in_data.data);
        break;
      }

    }
  }
};

// Funnel 3-to-1 with arbitration and multiplexing
template<typename T>
class Funnel<T,3> {
  typedef ac_int<ac::log2_ceil<3>::val,false> bvector;
  typedef ac_int<3,false> nvector;
private:
  T in_data[3];
  nvector actv_req;
  RoundRobinArbiter<3> rra;
public:
  // Constructor
  Funnel() : actv_req(0){}
  // Interface
  void run(
    ac_channel<T> &in0,
    ac_channel<T> &in1,
    ac_channel<T> &in2,
    ac_channel<T> &out
  ){
    // Read inputs with no pending requests
    if (actv_req[0] == 0) {
      actv_req[0] = in0.nb_read(in_data[0]);
    }
    if (actv_req[1] == 0) {
      actv_req[1] = in1.nb_read(in_data[1]);
    }
    if (actv_req[2] == 0) {
      actv_req[2] = in2.nb_read(in_data[2]);
    }

    // If there is at least one request
    bvector gnt;
    if (rra.arb(actv_req, gnt)) {
      // Send out the data of the winner
      out.write(in_data[gnt]);
      actv_req[gnt] = 0;
    }
  }
};

// Crossbar 3-to-4
#pragma hls_design top
template<typename T>
class StreamCrossbar<T,3,4> {
private:
  Split<T,4> sp[3];
  Funnel<T,3> fn[4];
  ac_channel<T> ntwrk[3][4];
public:
  // Constructor
  StreamCrossbar(){}
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<Flit<T,4> > in[3],
    ac_channel<T> out[4]
  ){
    // Connect inputs to Splitters to interconnect network
    #pragma unroll yes
    for (int i=0; i<3; ++i) {
      sp[i].run(in[i], ntwrk[i][0], ntwrk[i][1], ntwrk[i][2], ntwrk[i][3]);
    }

    // Connect interconnect network to Funnels to outputs
    #pragma unroll yes
    for (int i=0; i<4; ++i) {
      fn[i].run(ntwrk[0][i], ntwrk[1][i], ntwrk[2][i], out[i]);
    }
  }
};

#endif
