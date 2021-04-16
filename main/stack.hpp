#ifndef STACK_STRUCTURE
#define STACK_STRUCTURE

#include "ac_integers.hpp"

typedef class Stack {
  private:
    UInt *memory;     // Stores node positions
    UInt rear;        // Stores memory positions
    UInt capacity;    // Stores memory size
  public:
    Stack(UInt* mem, const UInt& memSize);
    bool isEmpty();
    bool isFull();
    void push(UInt d_in);
    UInt pop();
} Stack;

#ifdef __SYNTHESIS__
Stack::Stack(UInt *mem, const UInt& memSize):
memory(mem),rear(0),capacity(memSize){}
#else
Stack::Stack(UInt* mem, const UInt& memSize):
memory(new UInt[memSize]),rear(0),capacity(memSize){}
#endif

bool Stack::isEmpty(){
  return (rear==0) ? true : false;
}

bool Stack::isFull(){
  return (rear==capacity) ? true : false;
}

void Stack::push(UInt d_in){
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

UInt Stack::pop(){
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

#endif
