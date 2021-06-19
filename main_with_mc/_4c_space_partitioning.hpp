#ifndef SPACE_PARTITIONING_STRUCTURE
#define SPACE_PARTITIONING_STRUCTURE

#include "_3b_kdtree_parallel_cores.hpp"
#include "_4a_completion_buffer.hpp"
#include "_4b_multiport_memory.hpp"

// Split from 1 channel with opcode signal to 6 channels for each operation
class OperationSplitter{
  typedef Token<In,cbuf_size> TokenIn;
  typedef Token<DataIn,cbuf_size> TokenDataIn;
private:
  TokenIn tk;
  bool tk_actv;
  // Helper function
  TokenDataIn remove_opcode(const TokenIn &tk){
    TokenDataIn tmp;
    tmp.token = tk.token;
    tmp.data.node = tk.data.node;
    tmp.data.distance = tk.data.distance;
    return tmp;
  };
public:
  // Constructor
  OperationSplitter():tk_actv(false){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenIn>& tk_in,
    ac_channel<TokenDataIn> units_in[operations]
  ){
    // Get Input
    if (!tk_actv){
      tk_actv = tk_in.nb_read(tk);
    }
    // Write to output
    if (tk_actv){
      TokenDataIn tmp = remove_opcode(tk);
      tk_actv = !units_in[tk.data.opcode].nb_write(tmp);
    }
  };
};

template<typename T>
class ChannelMerger{
private:
  T data[memPorts];
  bool actv[memPorts];
public:
  // Constructor
  ChannelMerger(){
    #pragma unroll yes
    for (int i=0; i<memPorts; ++i){
      actv[i] = false;
    }
  };
  // Interface
  #pragma hls_design interface
  void run(
    // Inputs
    ac_channel<T> rst[reset_units],
    ac_channel<T> ins[insert_units],
    ac_channel<T> rmv[remove_units],
    ac_channel<T> src[search_units],
    ac_channel<T> rng[range_units],
    ac_channel<T> nns[nn_units],
    // Outputs
    ac_channel<T> merged[memPorts]
  ){
    // Get input
    int port = 0;
    // 1. Reset
    #pragma unroll yes
    for (int i=port; i<(port+reset_units); ++i){
      if (!actv[i]){
        actv[i] = rst[i-port].nb_read(data[i]);
      }
    }
    port = port + reset_units;
    // 2. Insert
    #pragma unroll yes
    for (int i=port; i<(port+insert_units); ++i){
      if (!actv[i]){
        actv[i] = ins[i-port].nb_read(data[i]);
      }
    }
    port = port + insert_units;
    // 3. Remove
    #pragma unroll yes
    for (int i=port; i<(port+remove_units); ++i){
      if (!actv[i]){
        actv[i] = rmv[i-port].nb_read(data[i]);
      }
    }
    port = port + remove_units;
    // 4. Search
    #pragma unroll yes
    for (int i=port; i<(port+search_units); ++i){
      if (!actv[i]){
        actv[i] = src[i-port].nb_read(data[i]);
      }
    }
    port = port + search_units;
    // 5. Range Search
    #pragma unroll yes
    for (int i=port; i<(port+range_units); ++i){
      if (!actv[i]){
        actv[i] = rng[i-port].nb_read(data[i]);
      }
    }
    port = port + range_units;
    // 6. Nearest Neighbor Search
    #pragma unroll yes
    for (int i=port; i<(port+nn_units); ++i){
      if (!actv[i]){
        actv[i] = nns[i-port].nb_read(data[i]);
      }
    }
    port = port + nn_units;

    // Write to output
    #pragma unroll yes
    for (int i=0; i<memPorts; ++i){
      if (actv[i]){
        actv[i] = !merged[i].nb_write(data[i]);
      }
    }
  };
};

template<typename T>
class ChannelSplitter{
private:
  T data[memPorts];
  bool actv[memPorts];
public:
  // Constructor
  ChannelSplitter(){
    #pragma unroll yes
    for (int i=0; i<memPorts; ++i){
      actv[i] = false;
    }
  };
  // Interface
  #pragma hls_design interface
  void run(
    // Inputs
    ac_channel<T> merged[memPorts],
    // Outputs
    ac_channel<T> rst[reset_units],
    ac_channel<T> ins[insert_units],
    ac_channel<T> rmv[remove_units],
    ac_channel<T> src[search_units],
    ac_channel<T> rng[range_units],
    ac_channel<T> nns[nn_units]
  ){
    // Get input
    #pragma unroll yes
    for (int i=0; i<memPorts; ++i){
      if (!actv[i]){
        actv[i] = merged[i].nb_read(data[i]);
      }
    }

    // Write output
    int port = 0;
    // 1. Reset
    #pragma unroll yes
    for (int i=port; i<(port+reset_units); ++i){
      if (actv[i]){
        actv[i] = !rst[i-port].nb_write(data[i]);
      }
    }
    port = port + reset_units;
    // 2. Insert
    #pragma unroll yes
    for (int i=port; i<(port+insert_units); ++i){
      if (actv[i]){
        actv[i] = !ins[i-port].nb_write(data[i]);
      }
    }
    port = port + insert_units;
    // 3. Remove
    #pragma unroll yes
    for (int i=port; i<(port+remove_units); ++i){
      if (actv[i]){
        actv[i] = !rmv[i-port].nb_write(data[i]);
      }
    }
    port = port + remove_units;
    // 4. Search
    #pragma unroll yes
    for (int i=port; i<(port+search_units); ++i){
      if (actv[i]){
        actv[i] = !src[i-port].nb_write(data[i]);
      }
    }
    port = port + search_units;
    // 5. Range Search
    #pragma unroll yes
    for (int i=port; i<(port+range_units); ++i){
      if (actv[i]){
        actv[i] = !rng[i-port].nb_write(data[i]);
      }
    }
    port = port + range_units;
    // 6. Nearest Neighbor Search
    #pragma unroll yes
    for (int i=port; i<(port+nn_units); ++i){
      if (actv[i]){
        actv[i] = !nns[i-port].nb_write(data[i]);
      }
    }
    port = port + nn_units;
  };
};

/**
 * Interface for space partinioning operations based on the KD-Tree data structure
 *
 * Data Structure:
 * --> Multiple banks treated as a single multiport memory
 * --> Memory treated as a binary tree (KD-Trees are binary trees)
 *
 * Supported operations:
 * 1. Reset         : Empty the tree
 *      input   : none
 *      output  : none
 * 2. Insert        : Add a new given node in the tree
 *      input   : node
 *      output  : none
 * 3. Remove        : Remove a given node from the tree, if it exists
 *      input   : node
 *      output  : none
 * 4. Search        : Check if a given node exists in the tree
 *      input   : node
 *      output  : evaluation (true/false)
 * 5. Range Search  : Find all nodes in the tree that within a given
 *                    radius from a given center point
 *      input   : node, distance (node is used as center)
 *      output  : node, distance_square, finish_flag
 * 6. NN Search     : Find the nearest node to a given center point
 *      input   : node (node is used as center)
 *      output  : node, distance_square
 *
 * Template parameters:
 * @tparam Depth    : The depth of the KD-Tree. Dependent on depth:
 *                    --> Total memory size
 *                    --> Number of bits used for memory addressing
 *                    --> Break conditions when operations exceed maximum depth
 * @tparam Banks    : The number of banks from which the tree memory is made of.
 *                    Dependent on banks:
 *                    --> Bank memory size
 *                    --> Number of bits used for bank selection
 *
 * Interface:
 * @param treeMem   : Multiple memories, treated as one multiport binary tree
 * @param stackMem  : Multiple memories, each one used as an address stack
 * @param input     : A single input channel containing:
 *                    --> operation type
 *                    --> node
 *                    --> distance
 * @param output    : A single output channel containing:
 *                    --> evaluation
 *                    --> node
 *                    --> distance_square
 */
#pragma hls_design top
template<int Depth, int Banks>
class SpacePartitioningInterface{
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // Token types
  typedef Token<In,cbuf_size> TokenIn;
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
private:
  // Completion Buffer
  CompletionBuffer<In,Out,cbuf_size> cbuf;
  ac_channel<TokenIn> tk_in;
  // Operation Handling
  OperationSplitter op_split;
  ac_channel<TokenDataIn> unit_in[operations];
  // Parallel units handling
  // 1. Reset
  Parallel_Cores<ResetTree_Unit<Depth,Banks>,Depth,reset_units,false> rst;
  ac_channel<MemIn> rst_mem_in[reset_units];
  ac_channel<MemIn> rst_mem_out[reset_units];
  // 2. Insert
  Parallel_Cores<InsertNode_Unit<Depth,Banks>,Depth,insert_units,false> ins;
  ac_channel<MemIn> ins_mem_in[insert_units];
  ac_channel<MemIn> ins_mem_out[insert_units];
  // 3. Remove
  Parallel_Cores<RemoveNode_Unit<Depth,Banks>,Depth,remove_units,false> rmv;
  ac_channel<MemIn> rmv_mem_in[remove_units];
  ac_channel<MemIn> rmv_mem_out[remove_units];
  // 4. Search
  Parallel_Cores<SearchNode_Unit<Depth,Banks>,Depth,search_units,false> src;
  ac_channel<MemIn> src_mem_in[search_units];
  ac_channel<MemIn> src_mem_out[search_units];
  // 5. Range search
  Parallel_Cores<RangeSearch_Unit<Depth,Banks>,Depth,range_units,true> rng;
  ac_channel<MemIn> rng_mem_in[range_units];
  ac_channel<MemIn> rng_mem_out[range_units];
  // 6. NN Search
  Parallel_Cores<NNSearch_Unit<Depth,Banks>,Depth,nn_units,true> nns;
  ac_channel<MemIn> nns_mem_in[nn_units];
  ac_channel<MemIn> nns_mem_out[nn_units];
  // Result merger
  ac_channel<TokenOut> unit_out[operations];
  ResultFunnel<operations> res_merge;
  ac_channel<TokenOut> res_out;
  // Memory Controller
  ChannelMerger<MemIn> mem_merge;
  ac_channel<MemIn> memory_in[memPorts];
  PseudoMultiportMemoryController<Node,Depth,memPorts,Banks> mc;
  ac_channel<MemOut> memory_out[memPorts];
  ChannelSplitter<MemOut> mem_split;
public:
  // Constructor
  SpacePartitioningInterface(){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<In>& in,
    ac_channel<Out>& out,
    Arrays<Node,Depth,Banks> &treeMem,
    Arrays<Addr_t,Depth,range_units> &rng_stackMem,
    Arrays<Addr_t,Depth,nn_units> &nns_stackMem,
  ){
    // Get token from the completion buffer & put result in the completion buffer
    cbuf.run(in, tk_in, res_out, out);

    // Send to appropriate operational unit
    op_split.run(tk_in, unit_in);

    // Operational units
    // 1. Reset
    rst.run(unit_in[0], unit_out[0], rst_mem_in, rst_mem_out);
    // 2. Insert
    ins.run(unit_in[1], unit_out[1], ins_mem_in, ins_mem_out);
    // 3. Remove
    rmv.run(unit_in[2], unit_out[2], rmv_mem_in, rmv_mem_out);
    // 4. Search
    src.run(unit_in[3], unit_out[3], src_mem_in, src_mem_out);
    // 5. Range Search  ---> Needs stack
    rng.run(unit_in[4], unit_out[4], rng_mem_in, rng_mem_out, rng_stackMem);
    // 6. NN Search     ---> Needs stack
    nns.run(unit_in[5], unit_out[5], nns_mem_in, nns_mem_out, nns_stackMem);

    // Merging results
    res_merge.run(unit_out, res_out);

    // Memory operations
    mem_merge.run(
      rst_mem_in, ins_mem_in, rmv_mem_in,
      src_mem_in, rng_mem_in, nns_mem_in,
      memory_in
    );
    mc.run(memory_in, memory_out, treeMem);
    mem_split.run(
      memory_out,
      rst_mem_out, ins_mem_out, rmv_mem_out,
      src_mem_out, rng_mem_out, nns_mem_out
    );
  };
};

#endif
