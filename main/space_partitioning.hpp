#ifndef SPACE_PARTITIONING_STRUCTURE
#define SPACE_PARTITIONING_STRUCTURE

#include "ac_integers.hpp"
#include "ac_channels.hpp"

#include "kdtree.hpp"

#pragma hls_design top
void CCS_BLOCK(run_wrapped)(EvalChannel& in1, NodeChannel& in2, NodeChannel& in3,
NodeChannel& in4, EvalChannel& out4, NodeChannel& in5, EvalChannel& out5,
DataChannel& in6, DataChannel& out6, NodeChannel& in7, DataChannel& out7,
NodeChannel& in8, NodeChannel& out8, EvalChannel& in9, Node treeMem[arr_size], UInt stackMem[arr_size]){
  // KD Tree Structure
  static KDTree mytree(treeMem, stackMem);
  mytree.reset_tree(in1);
  mytree.insert_element(in2);
  mytree.remove_element(in3);
  mytree.search_element(in4, out4);
  mytree.dummy_insert(in5, out5);
  mytree.range_search(in6, out6);
  mytree.nearest_neighbor(in7, out7);
  mytree.tree_traversal(in8, out8);
  #ifndef __SYNTHESIS__
  mytree.printTree(in9);
  #endif
}

void run(InChannel& in, OutChannel& out, Node treeMem[arr_size], UInt stackMem[arr_size]){
  // Internal Channels
  static EvalChannel in1;     // Reset
  static NodeChannel in2;     // Insert
  static NodeChannel in3;     // Remove
  static NodeChannel in4;     // Search
  static EvalChannel out4;    // Search
  static NodeChannel in5;     // Dummy insert
  static EvalChannel out5;    // Dummy insert
  static DataChannel in6;     // Range search
  static DataChannel out6;    // Range search
  static NodeChannel in7;     // Nearest neighbor
  static DataChannel out7;    // Nearest neighbor
  static NodeChannel in8;     // Tree traversal
  static NodeChannel out8;    // Tree traversal
  static EvalChannel in9;     // Printing Results

  // Splitting input
  if (in.available(1)){
    In input = in.read();
    switch (input.opcode){
      case op_reset: {
        Eval tmp;
        in1.write(tmp);
        break;
      } case op_insert: {
        in2.write(input.node);
        break;
      } case op_remove: {
        in3.write(input.node);
        break;
      } case op_search: {
        in4.write(input.node);
        break;
      } case op_dummyIn: {
        in5.write(input.node);
        break;
      } case op_rangeSearch: {
        Data tmp;
        tmp.node = input.node;
        tmp.num = input.num;
        in6.write(tmp);
        break;
      } case op_NearestNeighbor: {
        in7.write(input.node);
        break;
      } case op_Traversal: {
        in8.write(input.node);
        break;
      } case op_print: {
        Eval tmp;
        in9.write(tmp);
        break;
      }
    } // End Switch
  } // End available

  // Running functions
  run_wrapped(in1, in2, in3, in4, out4, in5, out5, in6, out6, in7, out7, in8, out8, in9, treeMem, stackMem);

  // Joining output
  if (out4.available(1)){
    Eval tmp = out4.read();
    Out res;
    res.num = tmp.num;
    res.flag = tmp.flag;
    out.write(res);
  }
  if (out5.available(1)){
    Eval tmp = out5.read();
    Out res;
    res.num = tmp.num;
    res.flag = tmp.flag;
    out.write(res);
  }
  Data tmp6;
  while (out6.nb_read(tmp6)){
    Out res;
    res.num = tmp6.num;
    res.node = tmp6.node;
    out.write(res);
  }
  if (out7.available(1)){
    Data tmp = out7.read();
    Out res;
    res.num = tmp.num;
    res.node = tmp.node;
    out.write(res);
  }
  Node tmp8;
  while (out8.nb_read(tmp8)){
    Out res;
    res.node = tmp8;
    out.write(res);
  }
}

#endif
