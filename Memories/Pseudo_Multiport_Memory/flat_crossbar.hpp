#ifndef MY_FLAT_CROSSBAR
#define MY_FLAT_CROSSBAR

#include "ac_int.h"
#include "ac_channel.h"

#include "round_robin_arbiter.hpp"

#include "mc_scverify.h"

// (FL)ow-controlled dig(IT)
template<typename T, int OUT>
struct Flit {
  ac_int<ac::log2_ceil<OUT>::val,false> dst;
  T data;
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
  void CCS_BLOCK(run)(
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

#endif
