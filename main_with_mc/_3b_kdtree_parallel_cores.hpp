#ifndef OPERATIONAL_UNITS
#define OPERATIONAL_UNITS

#include "_3a_kdtree_operations.hpp"

template<int count>
class UnitSplitter{
  typedef Token<DataIn,cbuf_size> TokenDataIn;
private:
  bool available[count];
  TokenDataIn data;
  bool active;
public:
  // Constructor
  UnitSplitter():active(false){
    #pragma unroll yes
    for (int i=0; i<count; ++i){
      available[i] = true;
    }
  };
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn> &in,        // Input
    ac_channel<bool> finish[count],     // Input
    ac_channel<TokenDataIn> out[count], // Output
  ){
    // Refresh availability
    #pragma unroll yes
    for (int i=0; i<count; ++i){
      bool tmp;
      if (finish[i].nb_read(tmp)){
        available[i] = true;
      }
    }
    // Get input
    if (!active){
      active = in.nb_read(data);
    }
    // Splitter logic
    if (active){
      bool flag = true;
      #pragma unroll yes
      for (int i=0; i<count; ++i){
        if (flag && available[i]){
          flag = !out[i].nb_write(data);
          available[i] = flag;
        }
      }
      active = flag;
    }
  };
};

template <int count>
class ResultFunnel{
  typedef Token<Out,cbuf_size> TokenOut;
private:
  TokenOut data;
  bool active;
public:
  // Constructor
  ResultFunnel():active(false){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenOut> in[count],   // Input
    ac_channel<TokenOut> &out         // Output
  ){
    // Get input
    if (!active){
      bool flag = false;
      #pragma unroll yes
      for (int i=0; i<count; ++i){
        if (!flag){
          flag = in[i].nb_read(data);
        }
      }
      active = flag;
    }
    // Write to output
    if (active){
      active = !out.nb_write(data);
    }
  };
};

// Template declaration
template <typename T,int Depth, int count, bool stack>
class Parallel_Cores{};

// Partial template specialization        --> Without stack / multicore
template <typename T,int Depth, int count>
class Parallel_Cores<T,Depth,count,false>{
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
private:
  // Unit Splitter
  UnitSplitter<count> split;
  // Units
  ac_channel<TokenDataIn> unit_in[count];
  T unit[count];
  ac_channel<TokenOut> unit_out[count];
  ac_channel<bool> finish[count];
  // Merger
  ResultFunnel<count> merge;
public:
  // Constructor
  Parallel_Cores(){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn> &in,        // Input
    ac_channel<TokenOut> &out           // Output
    ac_channel<MemIn> mem_req[count],   // Memory operation in
    ac_channel<MemOut> mem_res[count]   // Memory operation out
  ){
    // Run merger
    merge.run(unit_out, out);
    // Run cores
    #pragma unroll yes
    for (int i=0; i<count; ++i){
      unit[i].run(unit_in[i], unit_out[i], mem_req[i], mem_res[i], finish[i]);
    }
    // Run splitter
    split.run(in, finish, unit_in);
  };
};

// Partial template specialization        --> With stack / multicore
template <typename T,int Depth, int count>
class Parallel_Cores<T,Depth,count,true>{
  typedef ac_int<Depth,false> Addr_t;
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
private:
  // Unit Splitter
  UnitSplitter<count> split;
  // Units
  ac_channel<TokenDataIn> unit_in[count];
  T unit[count];
  ac_channel<TokenOut> unit_out[count];
  ac_channel<bool> finish[count];
  // Merger
  ResultFunnel<count> merge;
public:
  // Constructor
  Parallel_Cores(){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn> &in,            // Input
    ac_channel<TokenOut> &out               // Output
    ac_channel<MemIn> mem_req[count],       // Memory operation in
    ac_channel<MemOut> mem_res[count],      // Memory operation out
    Arrays<Addr_t,Depth,count> &stack_mem   // Stack memory
  ){
    // Run merger
    merge.run(unit_out, out);
    // Run cores
    #pragma unroll yes
    for (int i=0; i<count; ++i){
      unit[i].run(
        unit_in[i], unit_out[i],
        mem_req[i], mem_res[i],
        finish[i],
        stack_mem.get_array(i)
      );
    }
    // Run splitter
    split.run(in, finish, unit_in);
  };
};

// Partial template specialization        --> Without stack / singlecore
template<typename T, int Depth>
class Parallel_Cores<T,Depth,1,false>{
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
private:
  T unit;
  ac_channel<bool> finish;
public:
  // Constructor
  Parallel_Cores():active(false),available(true){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn> &in,    // Input
    ac_channel<TokenOut> &out       // Output
    ac_channel<MemIn> mem_req[1],   // Memory operation in
    ac_channel<MemOut> mem_res[1]   // Memory operation out
  ){
    // Run core
    unit.run(in, out, mem_req[0], mem_res[0], finish);
    // Empty finish channel
    bool tmp;
    finish.nb_read(tmp);
  };
};

// Partial template specialization        --> With stack / singlecore
template<typename T, int Depth>
class Parallel_Cores<T,Depth,1,true>{
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
private:
  T unit;
  ac_channel<bool> finish;
public:
  // Constructor
  Parallel_Cores():available(true){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn> &in,        // Input
    ac_channel<TokenOut> &out           // Output
    ac_channel<MemIn> mem_req[1],       // Memory operation in
    ac_channel<MemOut> mem_res[1],      // Memory operation out
    Arrays<Addr_t,Depth,1> &stack_mem   // Stack memory
  ){
    // Run core
    unit.run(
      in, out,
      mem_req[0], mem_res[0],
      finish,
      stack_mem.get_array(0)
    );
    // Empty finish channel
    bool tmp;
    finish.nb_read(tmp);
  };
};

#endif
