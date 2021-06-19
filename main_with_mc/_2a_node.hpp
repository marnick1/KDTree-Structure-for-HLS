#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

#include "_1_parameters.hpp"
// Dims parameter

struct Node {
  bool is_leaf;
  int coords[Dims];   // Stores node coordinates
  bool has_left, has_right;
  bool operator==(const Node& a) const{
    bool flag = true;
    #pragma unroll yes
    for (int i=0; i<Dims; ++i){
      flag &= (coords[i] == a.coords[i]);
    }
    return flag;
  };
  static Node empty(){
    Node empty;
    empty.has_left = false;
    empty.has_right = false;
    empty.is_leaf = false;
    return empty;
  };
  static Node plane(
    const int &axis,
    const int &n1_coords,
    const int &n2_coords
  ){
    Node plane;
    plane.is_leaf = false;
    plane.coords[axis] = (n1_coords + n2_coords) >> 1;
  };
};

#endif
