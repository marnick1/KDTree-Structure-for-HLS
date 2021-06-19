#ifndef STACK_STRUCTURE
#define STACK_STRUCTURE

#include "ac_int.h"

// Stack interface with external memory
template<typename T, int AddrBits>
class Stack {
  typedef ac_int<AddrBits+1,false> int_max;
private:
  T *memory;      // Stack content
  int_max rear;      // Current position in stack
  int_max capacity;  // Maximum Capacity
public:
  // Constructor
  Stack():rear(0),capacity(1<<AddrBits){};
  // Connect to memory
  #ifdef __SYNTHESIS__
  void setMemory(T* mem){
    memory = mem;
  };
  #else
  void setMemory(T* mem){
    delete[] memory;
    memory = new T[1<<AddrBits];
  };
  #endif
  // Helpers
  bool isEmpty(){return (rear==0) ? true : false;}
  bool isFull(){return (rear==capacity) ? true : false;}
  // Push
  void push(T d_in){
    #ifdef __SYNTHESIS__
    memory[rear] = d_in;
    rear += 1;
    #else
    if (isFull()){
      throw "Error: push to full stack";
    } else {
      memory[rear] = d_in;
      rear += 1;
    }
    #endif
  }
  // Pop
  T pop(){
    #ifdef __SYNTHESIS__
    rear -= 1;
    return memory[rear];
    #else
    if (isEmpty()){
      throw "Error: pop from empty stack";
    } else {
      rear -= 1;
      return memory[rear];
    }
    #endif
  }
};

#endif
