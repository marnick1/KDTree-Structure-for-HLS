#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

#include "ac_integers.hpp"

// K-Dimensional space
static const int dims = 2;

typedef struct Node {
  bool is_leaf;
  Int coords[dims];   // Stores node coordinates
  bool has_left, has_right;
  bool operator==(const Node& a) const;
} Node;

bool Node::operator==(const Node& a) const{
  bool flag = (coords[0] == a.coords[0]);
  #pragma unroll yes
  for (int i=1; i<dims; ++i){
    flag &= (coords[i] == a.coords[i]);
  }
  return flag;
};

#endif