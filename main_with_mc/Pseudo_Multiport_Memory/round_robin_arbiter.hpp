#ifndef MY_CUSTOM_RRA
#define MY_CUSTOM_RRA

#include "ac_int.h"

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

#endif
