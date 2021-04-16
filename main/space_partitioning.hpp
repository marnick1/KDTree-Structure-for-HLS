#ifndef SPACE_PARTITIONING_STRUCTURE
#define SPACE_PARTITIONING_STRUCTURE

#include "ac_integers.hpp"
#include "ac_channels.hpp"

#include "kdtree.hpp"

#include "mc_scverify.h"

// #pragma hls_design top
// void CCS_BLOCK(run)(InChannel& in, OutChannel& out, Node treeMem[arr_size], UInt stackMem[arr_size]){
//   static KDTree mytree(treeMem, stackMem);
//   // Insert
//   static NodeChannel in1;
//   // Remove
//   static NodeChannel in2;
//   // Search
//   static NodeChannel in3;
//   static EvalChannel out3;
//   // Dummy insert
//   static NodeChannel in4;
//   static EvalChannel out4;
//   // Range search
//   static DataChannel in5;
//   static DataChannel out5;
//   // Nearest neighbor
//   static NodeChannel in6;
//   static DataChannel out6;
//   // Tree traversal
//   static NodeChannel in7;
//   static NodeChannel out7;

//   if (in.available(1)){
//     In input = in.read();
//     switch (input.opcode){
//       case op_reset: {
//         mytree.reset_tree();
//         break;
//       } case op_insert: {
//         in1.write(input.node);
//         mytree.insert_element(in1);
//         break;
//       } case op_remove: {
//         in2.write(input.node);
//         mytree.remove_element(in2);
//         break;
//       } case op_search: {
//         in3.write(input.node);
//         mytree.search_element(in3, out3);
//         while (out3.available(1)){
//           Eval tmp = out3.read();
//           Out res;
//           res.num = tmp.num;
//           res.flag = tmp.flag;
//           out.write(res);
//         }
//         break;
//       } case op_dummyIn: {
//         in4.write(input.node);
//         mytree.dummy_insert(in4, out4);
//         while (out4.available(1)){
//           Eval tmp = out4.read();
//           Out res;
//           res.num = tmp.num;
//           res.flag = tmp.flag;
//           out.write(res);
//         }
//         break;
//       } case op_rangeSearch: {
//         Data tmp;
//         tmp.node = input.node;
//         tmp.num = input.num;
//         in5.write(tmp);
//         mytree.range_search(in5, out5);
//         while (out5.available(1)){
//           Data tmp = out5.read();
//           Out res;
//           res.num = tmp.num;
//           res.node = tmp.node;
//           out.write(res);
//         }
//         break;
//       } case op_NearestNeighbor: {
//         in6.write(input.node);
//         mytree.nearest_neighbor(in6, out6);
//         while (out6.available(1)){
//           Data tmp = out6.read();
//           Out res;
//           res.num = tmp.num;
//           res.node = tmp.node;
//           out.write(res);
//         }
//         break;
//       } case op_Traversal: {
//         in7.write(input.node);
//         mytree.tree_traversal(in7, out7);
//         while (out7.available(1)){
//           Node tmp = out7.read();
//           Out res;
//           res.node = tmp;
//           out.write(res);
//         }
//         break;
//       }
//       #ifndef __SYNTHESIS__
//       case op_print: {
//         mytree.printTree();
//       }
//       #endif
//     } // End Switch
//   } // End available
// }

#pragma hls_design top
void CCS_BLOCK(run)(InChannel& in, OutChannel& out, Node treeMem[arr_size], UInt stackMem[arr_size]){
  static KDTree mytree(treeMem, stackMem);
  // Insert
  static NodeChannel in1;
  // Remove
  static NodeChannel in2;
  // Search
  static NodeChannel in3;
  static EvalChannel out3;
  // Dummy insert
  static NodeChannel in4;
  static EvalChannel out4;
  // Range search
  static DataChannel in5;
  static DataChannel out5;
  // Nearest neighbor
  static NodeChannel in6;
  static DataChannel out6;
  // Tree traversal
  static NodeChannel in7;
  static NodeChannel out7;

  if (in.available(1)){
    In input = in.read();
    switch (input.opcode){
      case op_reset: {
        break;
      } case op_insert: {
        in1.write(input.node);
        break;
      } case op_remove: {
        in2.write(input.node);
        break;
      } case op_search: {
        in3.write(input.node);
        break;
      } case op_dummyIn: {
        in4.write(input.node);
        break;
      } case op_rangeSearch: {
        Data tmp;
        tmp.node = input.node;
        tmp.num = input.num;
        in5.write(tmp);
        break;
      } case op_NearestNeighbor: {
        in6.write(input.node);
        break;
      } case op_Traversal: {
        in7.write(input.node);
        break;
      }
      #ifndef __SYNTHESIS__
      case op_print: {
        break;
      }
      #endif
    } // End Switch
  } // End available
  // Use channels
  mytree.reset_tree();
  mytree.insert_element(in1);
  mytree.remove_element(in2);
  mytree.search_element(in3, out3);
  if (out3.available(1)){
    Eval tmp = out3.read();
    Out res;
    res.num = tmp.num;
    res.flag = tmp.flag;
    out.write(res);
  }
  mytree.dummy_insert(in4, out4);
  if (out4.available(1)){
    Eval tmp = out4.read();
    Out res;
    res.num = tmp.num;
    res.flag = tmp.flag;
    out.write(res);
  }
  mytree.range_search(in5, out5);
  Data tmp5;
  while (out5.nb_read(tmp5)){
    Out res;
    res.num = tmp5.num;
    res.node = tmp5.node;
    out.write(res);
  }
  mytree.nearest_neighbor(in6, out6);
  if (out6.available(1)){
    Data tmp = out6.read();
    Out res;
    res.num = tmp.num;
    res.node = tmp.node;
    out.write(res);
  }
  mytree.tree_traversal(in7, out7);
  Node tmp7;
  while (out7.nb_read(tmp7)){
    Out res;
    res.node = tmp7;
    out.write(res);
  }
  #ifndef __SYNTHESIS__
    mytree.printTree();
  #endif
}

#endif
