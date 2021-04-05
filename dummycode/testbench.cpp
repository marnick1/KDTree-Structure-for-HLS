#include <iostream>
#include <cstdlib>
#include <ctime>

#include "space_partitioning.hpp"

// Max value for node coordinates
static const int limit = 32;

// Depth and memory size
static const int tree_depth = 4;
static const int arr_size = (1<<tree_depth)-1;

// Test nodes
static const int N = 6;

// Printing function for debugging
void printNode(node n, int depth){
  int axis = depth%k;
  if (n.is_leaf){   // Leaf
    std::cout << "{" << n.coords[0] << "," << n.coords[1] << "}\t";
  } else {          // Plane
    if (n.has_left || n.has_right){
      std::cout << "Axis" << axis << ":" << n.coords[axis] << "\t";
    } else {        // Root
      std::cout << "Empty \t";
    }
  }
}

// Printing function for debugging
void printTree(node memory[]){
  for (int i=0; i<tree_depth; ++i){
    std::cout << i << ": ";
    for (int j=(1<<i)-1; j<(2<<i)-1; ++j){
      std::cout << j << ": ";
      node par = (j==0) ? memory[0] : memory[parent(j)];
      if (j==0){
        printNode(memory[j], i);
      } else if (par.is_leaf){
        std::cout << "Stop  \t";
        memory[j].is_leaf = true;
      } else if (par.has_left && j==left_child(parent(j))){
        printNode(memory[j], i);
      } else if (par.has_right && j==right_child(parent(j))){
        printNode(memory[j], i);
      } else {
        std::cout << "Stop  \t";
      }
    }
    std::cout << std::endl;
  }
}

CCS_MAIN(int argc, char* argv[]){
  // RNG Initialization
  std::srand(1234);

  // Declarations
  ac_int<3,false> opcode;
  sd<1> data_in;
  sd<size> data_out;
  node memory[arr_size];
  int stack_mem[arr_size];

  // Clear the tree
  opcode = 0;
  run(opcode, data_in, data_out, memory, stack_mem);
  printNode(memory[0], 0);
  std::cout << std:: endl;

  // Insert elements
  opcode = 1;
  for (int i=0; i<N; ++i){
    data_in.element[0].is_leaf = true;
    data_in.element[0].has_left = false;
    data_in.element[0].has_right = false;
    for (int j=0; j<k; ++j){
      data_in.element[0].coords[j] = std::rand() % limit;
    }
    std::cout << "Inserting node ";
    printNode(data_in.element[0], 0);
    std::cout << std::endl;
    run(opcode, data_in, data_out, memory, stack_mem);
  }
  // Printing Results
  printTree(memory);

  // Remove element
  opcode = 2;
  std::cout << "Removing node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  printTree(memory);

  // Search for element
  opcode = 3;
  data_in.element[0].coords[0]=4;
  data_in.element[0].coords[1]=21;
  std::cout << "Search for node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  std::cout << "Node position: " << data_out.num << std::endl;

  // Remove element
  opcode = 2;
  std::cout << "Removing node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  printTree(memory);

  // Dummy insert element
  opcode = 4;
  std::cout << "Inserting dummy node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  std::cout << "Dummy position: " << data_out.num << std::endl;

  // Search for element
  opcode = 3;
  std::cout << "Search for node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  std::cout << "Node position: " << data_out.num << std::endl;

  // Range search around {24,23} with range 14
  opcode = 5;
  data_in.num = 14;
  data_in.element[0].coords[0] = 24;
  data_in.element[0].coords[1] = 23;
  std::cout << "Range search of radius^2 " << data_in.num << " for node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  std::cout << data_out.num << " neighbors found:" << std::endl;
  for(int i=0; i<data_out.num; ++i){
    printNode(data_out.element[i], 0);
  }
  std::cout << std::endl;

  // Nearest Neighbor search
  opcode = 6;
  data_in.element[0].coords[0] = 24;
  data_in.element[0].coords[1] = 20;
  std::cout << "Nearest neighbor of node ";
  printNode(data_in.element[0], 0);
  std::cout << std::endl;
  run(opcode, data_in, data_out, memory, stack_mem);
  // Printing Results
  printNode(data_out.element[0], 0);
  std::cout << " Distance^2: " << data_out.num << std::endl;

  CCS_RETURN(0);
}
