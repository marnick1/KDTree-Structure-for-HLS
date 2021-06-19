#ifndef MY_CUSTOM_DATATYPES
#define MY_CUSTOM_DATATYPES

#include "ac_int.h"
#include "ac_channel.h"

#include "_2a_node.hpp"
#include "_2b_stack.hpp"

enum OP {
  op_reset = 0,
  op_insert = 1,
  op_remove = 2,
  op_search = 3,
  op_rangeSearch = 4,
  op_NearestNeighbor = 5
};
static const int operations = 6;

struct In{
  ac_int<3,false> opcode;
  Node node;
  int distance;
};

struct DataIn{
  Node node;
  int distance;
};

struct Out{
  Node node;
  int distance;
  bool eval;
};

template <typename T, int bufSize>
struct Token{
  T data;
  ac_int<ac::log2_ceil<bufSize>::val,false> token;
  template<typename N>
  static Token<T,bufsize> copyToken(const Token<N,bufSize> &tk_old){
    Token<T,bufsize> tk_new;
    tk_new.token = tk_old.token;
    return tk_new;
  }
};

// Memory Input Variables
template <typename T, int AddrBits>
struct MemoryIn
{
  ac_int<AddrBits,false> address;
  T data;
  bool rw;    // 0 -> Read  1 -> Write
  // Testing Timings
  int ticket;
};

// Memory Output Variables
template <typename T>
struct MemoryOut
{
  T data;
  // Testing Timings
  int ticket;
};

// (FL)ow-controlled dig(IT)
template <typename T, int OUT>
struct Flit {
  ac_int<ac::log2_ceil<OUT>::val,false> dst;
  T data;
};

template <typename T, int AddrBits, int banks>
struct Arrays{
  T array0[1<<AddrBits];
  T* get_array(const int &i){
    return array0;
  };
};

// Partial Specialization for Stack Memory
template <typename T, int AddrBits>
struct Arrays<T, AddrBits, 10>
{
  T array0[1<<AddrBits];
  T array1[1<<AddrBits];
  T array2[1<<AddrBits];
  T array3[1<<AddrBits];
  T array4[1<<AddrBits];
  T array5[1<<AddrBits];
  T array6[1<<AddrBits];
  T array7[1<<AddrBits];
  T array8[1<<AddrBits];
  T array9[1<<AddrBits];
  T* get_array(const int &i){
    static const T* arr[10] = {
      array0, array1, array2, array3, array4,
      array5, array6, array7, array8, array9
    };
    return arr[i];
  };
};

// Partial Specialization for Tree Memory
template <typename T, int AddrBits>
struct Arrays<T, AddrBits, 4>
{
  T array0[1<<AddrBits];
  T array1[1<<AddrBits];
  T array2[1<<AddrBits];
  T array3[1<<AddrBits];
  T* get_array(const int &i){
    static const T* arr[10] = {
      array0, array1, array2, array3
    };
    return arr[i];
  };
};

#endif
