#ifndef KD_TREE_STRUCTURE
#define KD_TREE_STRUCTURE

#include "ac_integers.hpp"
#include "ac_channels.hpp"

#include "stack.hpp"
#include "node.hpp"

template <typename T>
T pow2(T& in){
  return in*in;
}

UInt dist(Node& a, Node& b){
  UInt distance = 0;
  #pragma unroll yes
  for(int i=0; i<dims; ++i){
    LongInt tmp = a.coords[i] - b.coords[i];
    distance += tmp*tmp;
  }
  return distance;
}

// Depth and memory size
static const int tree_depth = 4;
static const int arr_size = (1<<tree_depth)-1;

typedef class KDTree {
private:
  Node* mytree;     // Stores nodes
  UInt capacity;    // Stores mytree size
  Depth maxDepth;   // Stores mytree depth
  Stack mystack;
public:
  // Constructor
  KDTree(Node* NodeMem, UInt* StackMem);
  // Helpful functions
  Depth get_depth(UInt position);
  UInt left_child(UInt parent);
  UInt right_child(UInt parent);
  UInt get_parent(UInt child);
  // Tree actions
  void reset_tree();
  void insert_element(NodeChannel& in);
  void remove_element(NodeChannel& in);
  void search_element(NodeChannel& in, EvalChannel& out);
  void dummy_insert(NodeChannel& in, EvalChannel& out);
  void range_search(DataChannel& in, DataChannel& out);
  void nearest_neighbor(NodeChannel& in, DataChannel& out);
  void tree_traversal(NodeChannel& in, NodeChannel& out);
  // Simulation & debugging
  #ifndef __SYNTHESIS__
  void printNode(UInt position);
  void printTree();
  #endif
} KDTree;

#ifdef __SYNTHESIS__
KDTree::KDTree(Node* NodeMem, UInt* StackMem):
mytree(NodeMem),
capacity(arr_size),
maxDepth(tree_depth),
mystack(StackMem, arr_size){};
#else
KDTree::KDTree(Node* NodeMem, UInt* StackMem):
mytree(new Node[arr_size]),
capacity(arr_size),
maxDepth(tree_depth),
mystack(StackMem, arr_size){};
#endif

Depth KDTree::get_depth(UInt position){
  Depth depth=10000;
  bool flag = true;
  #pragma unroll yes
  for (Depth i=0; flag; ++i){
    UInt maxPos = (1<<i) - 1;
    if ( maxPos >= position){
      depth = i;
      flag = false;
    }
  }
  return depth;
}

UInt KDTree::left_child(UInt parent){
  return (parent<<1) + 1;
}

UInt KDTree::right_child(UInt parent){
  return (parent<<1) + 2;
}

UInt KDTree::get_parent(UInt child){
  UInt parent = (child>>1);
  parent -= ~child[0];
  return parent;
}

void KDTree::reset_tree(){
  Node tmp;
  tmp.has_left = false;
  tmp.has_right = false;
  tmp.is_leaf = false;
  mytree[0] = tmp;
}

void KDTree::insert_element(NodeChannel& in){
  if (in.available(1)){
    Node in_node = in.read();       // Retrieve the input node
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      bool right = in_node.coords[axis] > node.coords[axis];
      bool equal = in_node.coords[axis] == node.coords[axis];
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      // Cases
      if (node.is_leaf && !(node==in_node)){         // ---LEAF---
        Node plane;                             // New partitioning plane
        plane.is_leaf = false;
        plane.coords[axis] = (in_node.coords[axis] + node.coords[axis]) >> 1;
        plane.has_right = true;
        plane.has_left = true;
        if (equal){                             // Insert left & repeat
          plane.has_right = false;
          mytree[pos_left] = node;
          mystack.push(pos_left);
        } else {                                // Insert one right & one left
          mytree[pos_left] = (right) ? node : in_node;
          mytree[pos_right] = (right) ? in_node : node;
        }
        mytree[position] = plane;               // Insert plane
      } else if (!node.has_left && !node.has_right){  // ---EMPTY---
        mytree[position] = in_node;             // Insert without checks
      } else {                                        // ---PLANE---
        if (right && !node.has_right){          // in --> right
          node.has_right = true;
          mytree[position] = node;
          mytree[pos_right] = in_node;
        } else if (!right && !node.has_left){   // in --> left
          node.has_left = true;
          mytree[position] = node;
          mytree[pos_left] = in_node;
        } else {                                // position taken
          UInt child = (right) ? pos_right : pos_left;
          mystack.push(child);
        }
      } // End cases
    } // End while
  } // End available
}

void KDTree::remove_element(NodeChannel& in){
  if (in.available(1)){
    Node in_node = in.read();       // Retrieve the input node
    Node par_node;                  // Parent node
    bool par_right;                 // If current node is right of parent
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      bool right = in_node.coords[axis] > node.coords[axis];
      bool equal = in_node.coords[axis] == node.coords[axis];
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      bool found = (node == in_node);
      // Cases
      if (node.is_leaf && found){       // ---LEAF---
        if (position == 0){       // If root, reset
          reset_tree();
        } else {                  // Else change pointer of parent
          par_node.has_right = (par_right) ? false : par_node.has_right ;
          par_node.has_left = (par_right) ? par_node.has_left : false;
          mytree[get_parent(position)] = par_node;
        }
      } else {                          // ---PLANE/EMPTY---
        par_node = node;
        par_right = right;
        if (right && node.has_right){           // Go right
          mystack.push(pos_right);
        } else if (!right && node.has_left){    // Go left
          mystack.push(pos_left);
        }
      } // End cases
    } // End while
  } // End available
}

void KDTree::search_element(NodeChannel& in, EvalChannel& out){
  if (in.available(1)){
    Node in_node = in.read();       // Retrieve the input node
    Eval result;
    result.flag = false;            // Preset to NOT FOUND
    result.num = 0;                 // Preset to ROOT
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      bool right = in_node.coords[axis] > node.coords[axis];
      bool equal = in_node.coords[axis] == node.coords[axis];
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      bool found = (node == in_node);
      // Cases
      if (node.is_leaf && found){       // ---LEAF---
        result.flag = true;
        result.num = position;
      } else {                          // ---PLANE/EMPTY---
        if (right && node.has_right){           // Go right
          mystack.push(pos_right);
        } else if (!right && node.has_left){    // Go left
          mystack.push(pos_left);
        }
      } // End cases
    } // End while
    out.write(result);
  } // End available
}

void KDTree::dummy_insert(NodeChannel& in, EvalChannel& out){
  if (in.available(1)){
    Node in_node = in.read();       // Retrieve the input node
    Eval result;
    result.flag = false;            // Preset to PARENT IS NOT FREE
    result.num = 0;                 // Preset to ROOT
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      bool right = in_node.coords[axis] > node.coords[axis];
      bool equal = in_node.coords[axis] == node.coords[axis];
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      UInt child = (right) ? pos_right : pos_left;
      // Cases
      if (node.is_leaf){                              // ---LEAF---
        result.num = position;
      } else if (!node.has_left && !node.has_right){  // ---EMPTY---
        result.flag = true;
        result.num = position;
      } else {                                        // ---PLANE---
        if ((right && !node.has_right) || (!right && !node.has_left)){
          result.flag = true;
          result.num = child;
        } else {        // position taken
          mystack.push(child);
        }
      } // End cases
    } // End while
    out.write(result);
  } // End available
}

void KDTree::range_search(DataChannel& in, DataChannel& out){
  if (in.available(1)){
    Data in_data = in.read();       // Retrieve the input node
    Data result;
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      // Cases
      if (node.is_leaf){            // ---LEAF---
        UInt distance = dist(node, in_data.node);
        if (distance <= pow2(in_data.num)){
          result.node = node;
          result.num = distance;
          out.write(result);
        }
      } else {                      // ---PLANE/EMPTY---
        Int range_min = in_data.node.coords[axis] - in_data.num;
        if (node.has_left && (range_min <= node.coords[axis])){
          mystack.push(pos_left);
        }
        Int range_max = in_data.node.coords[axis] + in_data.num;
        if (node.has_right && (range_max > node.coords[axis])){
          mystack.push(pos_right);
        }
      } // End cases
    } // End while
  } // End available
}

void KDTree::nearest_neighbor(NodeChannel& in, DataChannel& out){
  if (in.available(1)){
    Node in_node = in.read();       // Retrieve the input node
    Data result;
    UInt neighbor;
    // Find the 1st neighbor
    mystack.push(0);                // Push the starting position (root)
    while (!mystack.isEmpty()){     // Iterate on the stack
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      bool right = in_node.coords[axis] > node.coords[axis];
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      UInt child = (right) ? pos_right : pos_left;
      // Cases
      if (node.is_leaf){                              // ---LEAF---
        result.num = dist(node, in_node);
        neighbor = get_parent(position);
      } else if (!node.has_left && !node.has_right){  // ---EMPTY---
        // Error, fix later
      } else {                                        // ---PLANE---
        if (node.has_left && !node.has_right){
          mystack.push(pos_left);
        } else if (!node.has_left && node.has_right){
          mystack.push(pos_right);
        } else {
          mystack.push(child);
        }
      } // End cases
    } // End while
    // Reverse visit the graph using the position of the current neighbor
    mystack.push(neighbor);
    while (!mystack.isEmpty()) {
      // Data
      UInt position = mystack.pop();
      Node node = mytree[position];
      Axis axis = get_depth(position) % dims;
      UInt pos_left = left_child(position);
      UInt pos_right = right_child(position);
      // Cases
      if (node.is_leaf){    // -------Leaf Node-------
        UInt new_distance = dist(node, in_node);
        if (new_distance<result.num){
          result.node = node;
          result.num = new_distance;
        }
      } else {        // -------Plane Node or Empty-------
        mystack.push(get_parent(position));
        Int range_min = in_node.coords[axis] - result.num;  // sqrt(result.num)
        if (node.has_left && (range_min <= node.coords[axis])){
          mystack.push(pos_left);
        }
        Int range_max = in_node.coords[axis] + result.num;  // sqrt(result.num)
        if (node.has_right && (range_max > node.coords[axis])){
          mystack.push(pos_right);
        }
      };
    } // End while
    out.write(result);
  } // End available
}

void KDTree::tree_traversal(NodeChannel& in, NodeChannel& out){
  // Dummy code
  if (in.available(1)){
    Node in_node = in.read();
    out.write(in_node);
  }
};

#ifndef __SYNTHESIS__
void KDTree::printNode(UInt position){
  Axis axis = get_depth(position) % dims;
  Node n = mytree[position];
  if (n.is_leaf){   // Leaf
    std::cout << "{" << n.coords[0];
    for (int i=1; i<dims; ++i){
      std::cout << "," << n.coords[i];
    }
    std::cout << "}\t";
  } else {          // Plane
    if (n.has_left || n.has_right){
      std::cout << "Axis" << axis << ":" << n.coords[axis] << "\t";
    } else {        // Root
      std::cout << "Empty \t";
    }
  }
}

void KDTree::printTree(){
  for (int i=0; i<tree_depth; ++i){
    std::cout << i << ": ";
    for (int j=(1<<i)-1; j<(2<<i)-1; ++j){
      std::cout << j << ": ";
      Node par = (j==0) ? mytree[0] : mytree[get_parent(j)];
      if (j==0){
        printNode(j);
      } else if (par.is_leaf){
        std::cout << "Stop  \t";
        mytree[j].is_leaf = true;
      } else if (par.has_left && j==left_child(get_parent(j))){
        printNode(j);
      } else if (par.has_right && j==right_child(get_parent(j))){
        printNode(j);
      } else {
        std::cout << "Stop  \t";
      }
    }
    std::cout << std::endl;
  }
}
#endif

#endif
