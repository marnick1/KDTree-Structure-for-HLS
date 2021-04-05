#ifndef SPACE_PARTITIONING_STRUCTURE
#define SPACE_PARTITIONING_STRUCTURE

#include "kdtree.hpp"

#pragma hls_design top
void CCS_BLOCK(run)(op& opcode, sd<1>& d_in, sd<size>& d_out, node* memory, int* mem){
  static kd_tree mytree(memory, mem);
  switch(opcode) {
    case 0: {
      // Reset the tree
      mytree.reset_tree();
      break;
    } case 1: {
      // Insert element
      mytree.insert_element(d_in.element[0]);
      break;
    } case 2: {
      // Remove element
      mytree.remove_element(d_in.element[0]);
      break;
    } case 3: {
      // Search for element position
      int pos;
      mytree.search_element(d_in.element[0], pos);
      sd<size> tmp;
      tmp.num = pos;
      d_out = tmp;
      break;
    } case 4: {
      // Insert dummy element
      sd<1> pos;
      mytree.dummy_insert(d_in.element[0], pos);
      sd<size> tmp;
      tmp.num = pos.num;
      d_out = tmp;
      break;
    } case 5: {
      // Range search
      mytree.range_search(d_in, d_out);
      break;
    } case 6: {
      // Nearest Neighbor search
      sd<1> output;
      mytree.nearest_neighbor(d_in.element[0], output);
      sd<size> tmp;
      tmp.num = output.num;
      tmp.element[0] = output.element[0];
      d_out = tmp;
      break;
    } case 7: {
      // Traversal from given position
      mytree.tree_traversal(d_in.element[0]);
      break;
    }
  }
}

#endif
