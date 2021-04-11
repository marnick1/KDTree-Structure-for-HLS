#ifndef KD_TREE_STRUCTURE
#define KD_TREE_STRUCTURE

#include "ac_int.h"
#include "stack.hpp"
#include "mc_scverify.h"

typedef ac_int<3,false> op;

// K-Dimensional space
static const int k = 2;

struct node {
  bool is_leaf;
  int coords[k];
  bool has_left, has_right;
  bool operator==(const node& a) const{
    bool flag = true;
    for (int i=0; i<k; ++i){
      if (coords[i] != a.coords[i]){
        flag = false;
        break;
      }
    }
    return flag;
  };
};

// Max number of return nodes
static const int size = 5;

template <int S>
struct sd {
  int num;
  node element[S];
};

// Get left child position in linear array
int left_child(int pos){
  return (pos<<1)+1;
}

// Get right child position in linear array
int right_child(int pos){
  return (pos<<1)+2;
}

// Get parent position in linear array
int parent(int pos){
  if (pos%2){
    return (pos>>1);
  } else {
    return (pos>>1)-1;
  }
}

// Get the depth of a position in linear array
int depth(int pos){
  for (int i=0;; ++i){
    for (int j=(1<<i)-1; j<(2<<i)-1; ++j){
      if (j==pos){
        return i;
      }
    }
  }
}

template <typename T>
T pow2(T& in){
  return in*in;
}

class kd_tree {
private:
  node* mytree;
  stack mystack;
public:
  kd_tree(node* memory, int* mem):mytree(memory),mystack(mem){};
  void reset_tree();
  void insert_element(node& d_in);
  void remove_element(node& d_in);
  void search_element(node& d_in, int& d_out);
  void dummy_insert(node& d_in, sd<1>& d_out);
  void range_search(sd<1>& d_in, sd<size>& d_out);
  void nearest_neighbor(node& d_in, sd<1>& d_out);
  void tree_traversal(node& d_in);
};

void kd_tree::reset_tree(){
  node tmp;
  tmp.has_left = false;
  tmp.has_right = false;
  tmp.is_leaf = false;
  mytree[0] = tmp;
}

void kd_tree::insert_element(node& d_in){
  int cur_pos = 0;
  int depth = 0;
  while (true){
    int axis = depth%k;
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){    // -----------------leaf------------------
      if (cur_node==d_in){
        break;          // Was duplicate, stopping
      } else {
        node plane;     // New partitioning plane
        plane.is_leaf = false;
        plane.coords[axis] = (d_in.coords[axis] + cur_node.coords[axis])>>1;
        // Move both leafs down on different sides
        plane.has_right = true;
        plane.has_left = true;
        if ((cur_node.coords[axis] > plane.coords[axis]) && (d_in.coords[axis] <= plane.coords[axis])){
          mytree[cur_pos] = plane;                      // parent
          mytree[right_child(cur_pos)] = cur_node;      // child 1
          mytree[left_child(cur_pos)] = d_in;           // child 2
          break;
        } else if ((cur_node.coords[axis] <= plane.coords[axis]) && (d_in.coords[axis] > plane.coords[axis])){
          mytree[cur_pos] = plane;                      // parent
          mytree[left_child(cur_pos)] = cur_node;       // child 1
          mytree[right_child(cur_pos)] = d_in;          // child 2
          break;
        } else {                    // both leafs left(equal value on axis)
          plane.has_right = false;
          mytree[cur_pos] = plane;                  // parent
          mytree[left_child(cur_pos)] = cur_node;   // child 1
          cur_pos = left_child(cur_pos);
          depth += 1;
        }

      }
    } else {            // --------------partitioning plane--------------
      if (cur_node.has_left || cur_node.has_right){
        bool lr = d_in.coords[axis] > cur_node.coords[axis];
        if (lr==false && !cur_node.has_left){         // empty left
          cur_node.has_left = true;
          mytree[cur_pos] = cur_node;                   // parent
          mytree[left_child(cur_pos)] = d_in;           // child 1
          break;
        } else if (lr==true && !cur_node.has_right){  // empty right
          cur_node.has_right = true;
          mytree[cur_pos] = cur_node;                   // parent
          mytree[right_child(cur_pos)] = d_in;          // child 1
          break;
        } else {                                        // plane not empty
          depth += 1;
          cur_pos = (lr) ? right_child(cur_pos) : left_child(cur_pos);
        }
      } else {      // --------------node is empty-----------------
        mytree[cur_pos] = d_in;
        break;
      }
    }
  }
}

void kd_tree::remove_element(node& d_in){
  int cur_pos = 0;
  int depth = 0;
  node prev_node;
  bool prev_lr;
  while (true){
    int axis = depth%k;
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){    // -----------------leaf------------------
      if (cur_node==d_in){          // Element found
        if (cur_pos == 0){          // If it is the root node
          node tmp;
          tmp.has_left = false;
          tmp.has_right = false;
          tmp.is_leaf = false;
          mytree[cur_pos] = tmp;    // Remove the root node
          break;                    // Node removed
        } else {                    // If it is not the root node
          if (prev_lr){             // If it was the right child
            prev_node.has_right = false;
          } else {                  // If it was the left child
            prev_node.has_left = false;
          }
          mytree[parent(cur_pos)] = prev_node;  // Remove the child
          break;                                // Node removed
        }
      } else {          // Leaf but not match, element not in the tree
        break;          // Element not found, stopping
      }
    } else {            // --------------partitioning plane--------------
      if (cur_node.has_left || cur_node.has_right){
        bool lr = d_in.coords[axis] > cur_node.coords[axis];
        if (lr==false && cur_node.has_left){        // If node would go left
          cur_pos = left_child(cur_pos);
          depth += 1;
          prev_node = cur_node;
          prev_lr = lr;
        } else if (lr==true && cur_node.has_right){  // If node would go right
          cur_pos = right_child(cur_pos);
          depth += 1;
          prev_node = cur_node;
          prev_lr = lr;
        } else {                          // If node doesn't lead to leaf
          break;                          // Element not found, stopping
        }
      } else {      // --------------node is empty-----------------
        break;                            // Element not found, stopping
      }
    }
  }
}

void kd_tree::search_element(node& d_in, int& d_out){
  int cur_pos = 0;
  int depth = 0;
  while (true){
    int axis = depth%k;
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){    // -----------------leaf------------------
      if (cur_node==d_in){    // Element found
        d_out = cur_pos;
      } else {                // Element not found
        d_out = -1;
      }
      break;
    } else {            // --------------partitioning plane--------------
      bool lr = d_in.coords[axis] > cur_node.coords[axis];
      if (lr==false && cur_node.has_left){          // If node would go left
        cur_pos = left_child(cur_pos);
        depth += 1;
      } else if (lr==true && cur_node.has_right){   // If node would go right
        cur_pos = right_child(cur_pos);
        depth += 1;
      } else {                                      // Element not found
        d_out = -1;
        break;
      }
    }
  }
}

void kd_tree::dummy_insert(node& d_in, sd<1>& d_out){
  int cur_pos = 0;
  int depth = 0;
  node par_node = mytree[cur_pos];
  while (true){
    int axis = depth%k;
    node cur_node = (cur_pos==0) ? par_node : mytree[cur_pos];
    if (cur_node.is_leaf){    // ----------------Leaf Node----------------
      d_out.num = cur_pos;
      d_out.element[0] = par_node;
      break;
    } else if (!cur_node.has_left && !cur_node.has_right){  // ---Empty---
      d_out.num = cur_pos;
      d_out.element[0] = par_node;
      break;
    } else {              // --------------Partitioning Plane--------------
      bool lr = d_in.coords[axis] > cur_node.coords[axis];
      if (!lr && !cur_node.has_left){         // empty left
        d_out.num = left_child(cur_pos);
        d_out.element[0] = cur_node;
        break;
      } else if (lr && !cur_node.has_right){  // empty right
        d_out.num = right_child(cur_pos);
        d_out.element[0] = cur_node;
        break;
      } else {                               // plane not empty
        depth += 1;
        cur_pos = (lr) ? right_child(cur_pos) : left_child(cur_pos);
        par_node = cur_node;
      }
    }
  }
}

void kd_tree::range_search(sd<1>& d_in, sd<size>& d_out){
  int cur_pos = 0;
  sd<size> output;
  output.num = 0;
  do {
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){    // -------Leaf Node-------
      int dist = 0;
      for(int i=0; i<k; ++i){
        int tmp =cur_node.coords[i]-d_in.element[0].coords[i];
        dist += pow2(tmp);
      }
      //dist = sqrt(dist);
      if (dist <= pow2(d_in.num)){
        ++output.num;
        if (output.num<=size){
          output.element[output.num-1] = cur_node;
        } else {
          break;
        }
      }
    } else {        // -------Plane Node or Empty-------
      int axis = depth(cur_pos)%k;
      int range_min = d_in.element[0].coords[axis] - d_in.num;
      if (cur_node.has_left && (range_min<=cur_node.coords[axis])){
        mystack.push(left_child(cur_pos));
      }
      int range_max = d_in.element[0].coords[axis] + d_in.num;
      if (cur_node.has_right && (range_max>cur_node.coords[axis])){
        mystack.push(right_child(cur_pos));
      }
    }
    mystack.pop(cur_pos);
  } while (cur_pos!=-1);
  mystack.push(-1);
  d_out = output;
}

void kd_tree::nearest_neighbor(node& d_in, sd<1>& d_out){
  // Find the first neighbor
  node cur_neighbor;
  int cur_distance;
  int cur_pos = 0;
  while (true) {
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){                      // -------Leaf Node-------
      int dist = 0;
      for(int i=0; i<k; ++i){
        int tmp = cur_node.coords[i]-d_in.coords[i];
        dist += pow2(tmp);
      }
      //dist = sqrt(dist);
      cur_neighbor = cur_node;
      cur_distance = dist;
      break;
    } else if (!cur_node.has_left && !cur_node.has_right){  // ---Empty---
      // Error, to fix later
      std::cout << "Error" << std::endl;
      break;
    } else {                            // -------Partitioning Plane-------
      mystack.push(cur_pos);
      int axis = depth(cur_pos)%k;
      if (cur_node.has_left && !cur_node.has_right){          // Only left
        cur_pos = left_child(cur_pos);
      } else if (!cur_node.has_left && cur_node.has_right){   // Only right
        cur_pos = right_child(cur_pos);
      } else {                                // Both directions available
        if (d_in.coords[axis] > cur_node.coords[axis]){
          cur_pos = right_child(cur_pos);
        } else {
          cur_pos = left_child(cur_pos);
        }
      }
    }
  } // End while
  // Reverse visit the graph using the positions in the stack
  mystack.pop(cur_pos);
  do {
    node cur_node = mytree[cur_pos];
    if (cur_node.is_leaf){    // -------Leaf Node-------
      int dist = 0;
      for(int i=0; i<k; ++i){
        int tmp = cur_node.coords[i]-d_in.coords[i];
        dist += pow2(tmp);
      }
      // dist = sqrt(dist);
      if (dist<cur_distance){
        cur_distance = dist;
        cur_neighbor = cur_node;
      }
    } else {        // -------Plane Node or Empty-------
      int axis = depth(cur_pos)%k;
      int range_min = d_in.coords[axis] - cur_distance; //sqrt(cur_distance)
      if (cur_node.has_left && (range_min <= cur_node.coords[axis])){
        mystack.push(left_child(cur_pos));
      }
      int range_max = d_in.coords[axis] + cur_distance; //sqrt(cur_distance)
      if (cur_node.has_right && (range_max > cur_node.coords[axis])){
        mystack.push(right_child(cur_pos));
      }
    }
    mystack.pop(cur_pos);
  } while (cur_pos!=-1);
  mystack.push(-1);
  // Fill the output
  sd<1> tmp;
  tmp.element[0] = cur_neighbor;
  tmp.num = cur_distance;
  d_out = tmp;
}

void kd_tree::tree_traversal(node& d_in){
  // ???
};

#endif
