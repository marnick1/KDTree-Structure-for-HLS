#ifndef MY_CUSTOM_CHANNELS
#define MY_CUSTOM_CHANNELS

#include "ac_integers.hpp"
#include "ac_channel.h"

#include "node.hpp"

enum Action {
  op_reset,
  op_insert,
  op_remove,
  op_search,
  op_dummyIn,
  op_rangeSearch,
  op_NearestNeighbor,
  op_Traversal,
  op_print
};

struct Data{
  UInt num;
  Node node;
};

struct Eval{
  UInt num;
  bool flag;
};

struct In{
  Action opcode;
  UInt num;
  Node node;
};

struct Out{
  UInt num;
  Node node;
  bool flag;
};

typedef ac_channel<Data> DataChannel;
typedef ac_channel<Eval> EvalChannel;
typedef ac_channel<Node> NodeChannel;
typedef ac_channel<UInt> UIntChannel;

typedef ac_channel<In> InChannel;
typedef ac_channel<Out> OutChannel;


#endif
