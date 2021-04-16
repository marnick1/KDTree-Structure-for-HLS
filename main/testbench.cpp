#include <iostream>
#include <cstdlib>
#include <ctime>

#include "ac_integers.hpp"
#include "ac_channels.hpp"

#include "space_partitioning.hpp"

// Max value for node coordinates
static const int limit = 128;

// Test nodes
static const int N = 6;

CCS_MAIN(int argc, char* argv[]){
  // RNG Initialization
  std::srand(1234);

  // Declarations
  InChannel input;
  In command;
  OutChannel output;
  Out result;

  Node memory[arr_size];
  UInt stack_mem[arr_size];

  // Clear the tree
  command.opcode = op_reset;
  input.write(command);
  run(input, output, memory, stack_mem);

  // Print the tree
  command.opcode = op_print;
  input.write(command);
  run(input, output, memory, stack_mem);

  std::cout << std:: endl;

  // // Insert elements
  // opcode = 1;
  // for (int i=0; i<N; ++i){
  //   data_in.element[0].is_leaf = true;
  //   data_in.element[0].has_left = false;
  //   data_in.element[0].has_right = false;
  //   for (int j=0; j<k; ++j){
  //     data_in.element[0].coords[j] = std::rand() % limit;
  //   }
  //   std::cout << "Inserting node ";
  //   printNode(data_in.element[0], 0);
  //   std::cout << std::endl;
  //   run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // }
  // // Printing Results
  // printTree(memory);

  // // Remove element
  // opcode = 2;
  // std::cout << "Removing node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // printTree(memory);

  // // Search for element
  // opcode = 3;
  // data_in.element[0].coords[0]=4;
  // data_in.element[0].coords[1]=21;
  // std::cout << "Search for node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // std::cout << "Node position: " << data_out.num << std::endl;

  // // Remove element
  // opcode = 2;
  // std::cout << "Removing node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // printTree(memory);

  // // Dummy insert element
  // opcode = 4;
  // std::cout << "Inserting dummy node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // std::cout << "Dummy position: " << data_out.num << std::endl;

  // // Search for element
  // opcode = 3;
  // std::cout << "Search for node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // std::cout << "Node position: " << data_out.num << std::endl;

  // // Range search around {24,23} with range 14
  // opcode = 5;
  // data_in.num = 14;
  // data_in.element[0].coords[0] = 24;
  // data_in.element[0].coords[1] = 23;
  // std::cout << "Range search of radius^2 " << data_in.num << " for node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // std::cout << data_out.num << " neighbors found:" << std::endl;
  // for(int i=0; i<data_out.num; ++i){
  //   printNode(data_out.element[i], 0);
  // }
  // std::cout << std::endl;

  // // Nearest Neighbor search
  // opcode = 6;
  // data_in.element[0].coords[0] = 24;
  // data_in.element[0].coords[1] = 20;
  // std::cout << "Nearest neighbor of node ";
  // printNode(data_in.element[0], 0);
  // std::cout << std::endl;
  // run<arr_size>(opcode, data_in, data_out, memory, stack_mem, arr_size);
  // // Printing Results
  // printNode(data_out.element[0], 0);
  // std::cout << " Distance^2: " << data_out.num << std::endl;

  CCS_RETURN(0);
}
