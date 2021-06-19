#ifndef KD_TREE_STRUCTURE
#define KD_TREE_STRUCTURE

#include "_2c_data_structures.hpp"

// Helper Function: Distance between 2 nodes
int dist(Node& a, Node& b){
  int distance = 0;
  #pragma unroll yes
  for(int i=0; i<Dims; ++i){
    int tmp = a.coords[i] - b.coords[i];
    distance += tmp * tmp;
  }
  return distance;
}

// Helper Function: Depth based on position
template<int Depth>
int get_depth(ac_int<Depth,false> position){
  int depth = 0;
  bool flag = true;
  #pragma unroll yes
  for (int i=1; i<=Depth; ++i){
    ac_int<Depth+1,false> tmp = (1<<i);
    ac_int<Depth,false> maxPos = tmp - 1;
    if ( maxPos > position){
      depth = (flag) ? (i-1) : depth;
      flag = false;
    }
  }
  return depth;
}

template<int Depth>
ac_int<Depth,false> left_child(ac_int<Depth,false> parent){
  return (parent<<1) + 1;
}

template<int Depth>
ac_int<Depth,false> right_child(ac_int<Depth,false> parent){
  return (parent<<1) + 2;
}

template<int Depth>
ac_int<Depth,false> get_parent(ac_int<Depth,false> child){
  return (child>>1) - (~child[0]);
}

template<int Depth, int Banks>
Flit<MemoryIn <Node,Depth>,Banks> set_input(
  const ac_int<Depth,false> &address,
  const Node &data,
  const bool &rw
){
  Flit<MemoryIn <Node,Depth>,Banks> tmp;
  tmp.dst = address.template slc<2>(0);
  tmp.data.address = address;
  tmp.data.data = data;
  tmp.data.rw = rw;
  return tmp;
}

// 1. Reset KDTree ------------------------ STACKLESS
template<int Depth, int Banks>
class ResetTree_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // FSM
  enum FSM {newOp, writeMem, waitMem, result, finish};
private:
  // Command input
  TokenDataIn input;
  // Internal state & variables
  FSM fsm;
public:
  // Constructor
  ResetTree_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn> &mem_req,
    ac_channel<MemOut> &mem_res,
    ac_channel<bool> &finish
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::writeMem : FSM::newOp;
        break;
      }
      case (FSM::writeMem):{
        // Write empty node @ 0
        MemIn write = set_input<Depth,Banks>(0,Node::empty(),true);
        bool flag = mem_req.nb_write(write);
        fsm = (flag) ? FSM::waitMem : FSM::writeMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        MemOut mem_out;
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? FSM::result : FSM::waitMem;
        break;
      }
      case (FSM::result):{
        // Write output
        TokenOut output = TokenOut::copyToken(input);
        bool flag = out.nb_write(output);
        fsm = (flag) ? FSM::finish : FSM::result;
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

// 2. Insert node in KDTree --------------- STACKLESS
template<int Depth, int Banks>
class InsertNode_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // FSM
  enum FSM {newOp, accessMem, waitMem, logic, insertNext, result, finish};
private:
  // Command Input
  TokenDataIn input;
  // Memory operations
  MemOut mem_out;
  Addr_t mem_position;
  Node mem_node;
  bool mem_rw;
  // Internal state
  FSM fsm;
  FSM fsm_next;
  FSM fsm_next2;
  // Internal Variables
  Addr_t mem_position_next;
  Node mem_node_next;
  bool overwrite_flag;
public:
  // Constructor
  InsertNode_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn> &mem_req,
    ac_channel<MemOut> &mem_res,
    ac_channel<bool> &finish
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get new input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::accessMem : FSM::newOp;
        mem_position = 0;
        // mem_node = Node::empty;
        mem_rw = false;
        fsm_next = FSM::logic;
        // Initialize variables
        overwrite_flag = false;
        break;
      }
      case (FSM::accessMem):{
        // Access memory @ {mem_position} for {mem_rw} operation on data {mem_node}
        MemIn tmp = set_input<Depth,Banks>(mem_position,mem_node,mem_rw);
        bool flag = mem_req.nb_write(tmp);
        fsm = (flag) ? FSM::waitMem : FSM::accessMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? fsm_next : FSM::waitMem;
        break;
      }
      case (FSM::logic):{
        // Insert operation logic
        Node input_node = input.data.node;
        Node current_node = mem_out.data.data;
        // Mem_position overwrite in case of leaf on leaf;
        mem_position = (overwrite_flag) ? mem_position_next : mem_position;
        overwrite_flag = false;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool equal = input_node.coords[axis] == current_node.coords[axis];
        bool right = input_node.coords[axis] >  current_node.coords[axis];
        // Children positions
        Addr_t pos_left = left_child(mem_position);
        Addr_t pos_right = right_child(mem_position);
        Addr_t child = (right) ? pos_right : pos_left;
        // Prepare plane
        Node plane = Node::plane(axis,input_node.coords[axis],current_node.coords[axis]);
        // Decide next action
        if (!current_node.is_leaf){                     // Plane -------------------
          if (!current_node.has_left && !current_node.has_right){ // Empty: --------
            // Insert and finish
            fsm = FSM::accessMem;
            // mem_position = mem_position;
            mem_node = input_node;
            mem_rw = true;
            fsm_next = FSM::result;
          } else if (!right && !current_node.has_left){           // Free left: ----
            // Update parent, insert child and finish
            fsm = FSM::accessMem;
            // mem_position = mem_position;
            mem_node = current_node;
            mem_node.has_left = true;
            mem_rw = true;
            fsm_next = FSM::insertNext;
            mem_position_next = child;
            mem_node_next = input_node;
            fsm_next2 = FSM::result;
          } else if (right && !current_node.has_right){           // Free right: ---
            // Update parent, insert child and finish
            fsm = FSM::accessMem;
            // mem_position = mem_position;
            mem_node = current_node;
            mem_node.has_right = true;
            mem_rw = true;
            fsm_next = FSM::insertNext;
            mem_position_next = child;
            mem_node_next = input_node;
            fsm_next2 = FSM::result;
          } else {                                                // Not free: -----
            // Move down the tree and read the next node
            fsm = FSM::accessMem;
            mem_position = child;
            // mem_node = Node::empty;
            mem_rw = false;
            fsm_next = FSM::logic;
          }
        } else {                                        // Leaf --------------------
          if (current_node == input_node){                        // Duplicate: ----
            // Finish
            fsm = FSM::result;
          } else {                                                // Different: ----
            // Insert plane, move down old node and read again
            bool tmp = (right || equal);
            plane.has_left  = (tmp)  ? true : false;
            plane.has_right = (!tmp) ? true : false;
            fsm = FSM::accessMem;
            // mem_position = mem_position;
            mem_node = plane;
            mem_rw = true;
            fsm_next = FSM::insertNext;
            mem_position_next = (tmp) = pos_left : pos_right;
            mem_node_next = current_node;
            fsm_next2 = FSM::logic;
            mem_out.data.data = plane;
          }
        }                                               // End ---------------------
        break;
      }
      case (FSM::insertNext):{
        Addr_t tmp = mem_position;
        fsm = FSM::accessMem;
        mem_position = mem_position_next;
        mem_node = mem_node_next;
        // mem_rw = true;
        fsm_next = fsm_next2;
        mem_position_next = tmp;
        overwrite_flag = true;
        break;
      }
      case (FSM::result):{
        // Write output
        TokenOut output = TokenOut::copyToken(input);
        bool flag = out.nb_write(output);
        fsm = (flag) ? FSM::finish : FSM::result;
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

// 3. Remove node from KDTree ------------- STACKLESS
template<int Depth, int Banks>
class RemoveNode_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // FSM
  enum FSM {newOp, accessMem, waitMem, logic, result, finish};
private:
  // Command Input
  TokenDataIn input;
  // Memory operations
  MemOut mem_out;
  Addr_t mem_position;
  Node mem_node;
  bool mem_rw;
  // Internal state
  FSM fsm;
  FSM fsm_next;
  // Internal variables
  Addr_t par_position;
  Node par_node;
public:
  // Constructor
  RemoveNode_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn> &mem_req,
    ac_channel<MemOut> &mem_res,
    ac_channel<bool> &finish
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get new input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::accessMem : FSM::newOp;
        mem_position = 0;
        // mem_node = Node::empty;
        mem_rw = false;
        fsm_next = FSM::logic;
        // Initialize Variables
        par_position = 0;
        par_node = Node::empty();
        break;
      }
      case (FSM::accessMem):{
        // Access memory @ {mem_position} for {mem_rw} operation on data {mem_node}
        MemIn tmp = set_input<Depth,Banks>(mem_position,mem_node,mem_rw);
        bool flag = mem_req.nb_write(tmp);
        fsm = (flag) ? FSM::waitMem : FSM::accessMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? fsm_next : FSM::waitMem;
        break;
      }
      case (FSM::logic):{
        // Remove operation logic
        Node input_node = input.data.node;
        Node current_node = mem_out.data.data;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool right = input_node.coords[axis] > current_node.coords[axis];
        // Decide next action
        if (!current_node.is_leaf){             // Plane ---------------------
          // Update parent and child positions
          par_position = mem_position;
          par_node = current_node;
          mem_position = (right) ? right_child(position) : left_child(position);
          if (!right && current_node.has_left){       // ------------- To left
            // Move down the tree and read the next node
            fsm = FSM::accessMem;
            // mem_position = mem_position
            // mem_node = Node::empty()
            // mem_rw = false;
            fsm_next = FSM::logic;
          }
          else if (right && current_node.has_right){  // ------------ To Right
            // Move down the tree and read the next node
            fsm = FSM::accessMem;
            // mem_position = mem_position
            // mem_node = Node::empty()
            // mem_rw = false;
            fsm_next = FSM::logic;
          }
          else {                                      // -- Not found or empty
            fsm = FSM::result;
          }
        }
        else if (current_node == input_node){   // Leaf & found --------------
          fsm = FSM::accessMem;
          mem_position = par_position;
          mem_node = par_node;
          mem_rw = true;
          fsm_next = FSM::result;
        }
        else {                                  // Leaf & not found ----------
          fsm = FSM::result
        }                                       // End -----------------------
        break;
      }
      case (FSM::result):{
        // Write output
        TokenOut output = TokenOut::copyToken(input);
        bool flag = out.nb_write(output);
        fsm = (flag) ? FSM::finish : FSM::result;
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

// 4. Search node in KDTree --------------- STACKLESS
template<int Depth, int Banks>
class SearchNode_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // FSM
  enum FSM {newOp, accessMem, waitMem, logic, result, finish};
private:
  // Command Input
  TokenDataIn input;
  // Command Output
  TokenOut output;
  // Memory operations
  MemOut mem_out;
  Addr_t mem_position;
  Node mem_node;
  bool mem_rw;
  // Internal state
  FSM fsm;
  FSM fsm_next;
public:
  // Constructor
  SearchNode_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run(
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn> &mem_req,
    ac_channel<MemOut> &mem_res,
    ac_channel<bool> &finish
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get new input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::accessMem : FSM::newOp;
        mem_position = 0;
        // mem_node = Node::empty;
        mem_rw = false;
        fsm_next = FSM::logic;
        // Initialize Variables
        output = TokenOut::copyToken(input);
        break;
      }
      case (FSM::accessMem):{
        // Access memory @ {mem_position} for {mem_rw} operation on data {mem_node}
        MemIn tmp = set_input<Depth,Banks>(mem_position,mem_node,mem_rw);
        bool flag = mem_req.nb_write(tmp);
        fsm = (flag) ? FSM::waitMem : FSM::accessMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? fsm_next : FSM::waitMem;
        break;
      }
      case (FSM::logic):{
        // Search operation logic
        Node input_node = input.data.node;
        Node current_node = mem_out.data.data;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool right = input_node.coords[axis] > current_node.coords[axis];
        // Decide next action
        if (!current_node.is_leaf){             // Plane ---------------------
          mem_position = (right) ? right_child(position) : left_child(position);
          if (!right && current_node.has_left){       // ------------- To left
            // Move down the tree and read the next node
            fsm = FSM::accessMem;
            // mem_position = mem_position
            // mem_node = Node::empty()
            // mem_rw = false;
            fsm_next = FSM::logic;
          }
          else if (right && current_node.has_right){  // ------------ To Right
            // Move down the tree and read the next node
            fsm = FSM::accessMem;
            // mem_position = mem_position
            // mem_node = Node::empty()
            // mem_rw = false;
            fsm_next = FSM::logic;
          }
          else {                                      // -- Not found or empty
            fsm = FSM::result;
            output.data.eval = false;
          }
        }
        else if (current_node == input_node){   // Leaf & found --------------
          fsm = FSM::result;
          output.data.eval = true;
        }
        else {                                  // Leaf & not found ----------
          fsm = FSM::result;
          output.data.eval = false;
        }                                       // End -----------------------
        break;
      }
      case (FSM::result):{
        // Write output
        bool flag = out.nb_write(output);
        fsm = (flag) ? FSM::finish : FSM::result;
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

// Simulate node insert in KDTree ------ STACKLESS
// template<int Depth, int Banks>
// class dummy_insert{
//   typedef ac_int<Depth,false> Addr_t;   // Address type
//   enum FSM {start, check, finish};
// private:
//   // Command Input
//   Node in_node;
//   bool active;
//   // Command Output
//   Eval result;
//   // Memory results
//   Flit<MemoryOut<Node      >,Banks> mem_out;
//   bool mem_op;
//   // Internal variables
//   FSM fsm;
//   // Current node data
//   Addr_t position;
// public:
//   // Constructor
//   dummy_insert():active(false){};
//   // Interface
//   #pragma hls_design interface
//   void run (
//     NodeChannel& in,
//     EvalChannel& out,
//     ac_channel<Flit<MemoryIn <Node,Depth>,Banks> > &mem_req,
//     ac_channel<Flit<MemoryOut<Node      >,Banks> > &mem_res
//   ){
//     if (!active){     // Get Input (Non - Blocking)
//       active = in.nb_read(in_node);     // Retrieve the input node
//       position = 0;                     // Starting position (root)
//       result.flag = false;              // Preset to PARENT-NOT-FREE
//       result.num = 0;                   // Preset to root
//       mem_op = false;
//       fsm = FSM::start;
//     }
//     else {            // Block active
//       if (!mem_op){   // Insert logic
//         switch (fsm){
//         case (FSM::start):
//           // Retrieve current node data
//           Node tmp;
//           mem_req.write(set_input<Depth,Banks>(position,tmp,false));
//           mem_op = true;
//           fsm = (FSM::check);
//           break;
//         case (FSM::check):
//           // Get current node
//           Node node = mem_out.data.data;
//           // Calculate flags
//           int axis = get_depth(position) % Dims;
//           bool right = in_node.coords[axis] > node.coords[axis];
//           // Children positions
//           Addr_t pos_left = left_child(position);
//           Addr_t pos_right = right_child(position);
//           Addr_t child = (right) ? pos_right : pos_left;
//           // Decide next action
//           if (!node.is_leaf){                     // Plane ---------------------
//             if (!node.has_left && !node.has_right){   // ---- Empty --> Put here
//               result.flag = true;
//               result.num = position;
//               fsm = FSM::finish;
//             } else if (!right && !node.has_left){     // ------ Free to put left
//               result.flag = true;
//               result.num = pos_left;
//               fsm = FSM::finish;
//             } else if (right && !node.has_right){     // ----- Free to put right
//               result.flag = true;
//               result.num = pos_right;
//               fsm = FSM::finish;
//             } else {                                  // ----- Not free --> Next
//               position = child;
//               fsm = FSM::start;
//             }
//           } else {                                // Leaf ----------------------
//             result.num = position;
//             fsm = FSM::finish;
//           }                                       // End -----------------------
//           break;
//         case (FSM::finish):
//           out.write(result);
//           active = false;
//           fsm = FSM::start;
//           break;
//         } // End case
//       } // End insert logic
//       else {          // Waiting for memory
//         mem_op = !(mem_res.nb_read(mem_out));
//       }
//     } // End active
//   }
// };

// 5. Range search in KDTree -------------- NOT STACKLESS (yet)
template<int Depth, int Banks>
class RangeSearch_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // FSM
  enum FSM {newOp, readMem, waitMem, logic, result, finish};
private:
  // Command Input
  TokenDataIn input;
  // Command Output
  TokenOut output;
  // Memory results
  MemOut mem_out;
  Addr_t mem_position;
  // Internal state
  FSM fsm;
  // Internal data
  Stack<Addr_t,Depth> mystack;
public:
  // Constructor
  RangeSearch_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run (
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn>& mem_req,
    ac_channel<MemOut>& mem_res,
    ac_channel<bool>& finish,
    Addr_t StackMem[1<<Depth]
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get new input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::readMem : FSM::newOp;
        mem_position = 0;
        // Initialize Variables
        mystack.setMemory(StackMem);
        output = TokenOut::copyToken(input);
        break;
      }
      case (FSM::readMem):{
        // Read memory @ {mem_position}
        MemIn tmp = set_input<Depth,Banks>(mem_position,Node::empty(),false);
        bool flag = mem_req.nb_write(tmp);
        fsm = (flag) ? FSM::waitMem : FSM::accessMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? FSM::logic : FSM::waitMem;
        break;
      }
      case (FSM::logic):{
        // Range search operation logic
        Node input_node = input.data.node;
        int input_distance = input.data.distance;
        Node current_node = mem_out.data.data;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool right = input_node.coords[axis] > current_node.coords[axis];
        // Children positions
        Addr_t pos_left = left_child(mem_position);
        Addr_t pos_right = right_child(mem_position);
        // Distances
        int current_distance_square = dist(current_node, input_node);
        int input_distance_square = input_distance * input_distance;
        int range_min = input_node.coords[axis] - input_distance;
        int range_max = input_node.coords[axis] + input_distance;
        // Decide next action
        if (!node.is_leaf){                     // Plane or Empty --------------
          bool immediate_flag = false;
          if (
            current_node.has_left &&
            (range_min <= current_node.coords[axis])
          ){                                              //  Left within bounds
            mem_position = pos_left;
            immediate_flag = true;
          }
          if (
            current_node.has_right &&
            (range_max > node.coords[axis])
          ){                                              // Right within bounds
            if (immediate_flag){
              mystack.push(pos_right);
            } else {
              mem_position = pos_right;
              immediate_flag = true;
            }
          }
          // Next state
          if (immediate_flag){
            fsm = FSM::readMem;
          } else {
            bool flag = !mystack.isEmpty();
            fsm = (flag) ? FSM::readMem : FSM::finish;
            mem_position = (flag) ? mystack.pop() : mem_position;
          }
        } else {                                // Leaf ------------------------
          if (current_distance_square <= input_distance_square){
            output.data.node = current_node;
            output.data.distance = current_distance_square;
            fsm = FSM::result;
          } else{
            bool flag = !mystack.isEmpty();
            fsm = (flag) ? FSM::readMem : FSM::finish;
            mem_position = (flag) ? mystack.pop() : mem_position;
          }
        }                                       // End -------------------------
        break;
      }
      case (FSM::result):{
        // Write output
        bool flag = out.nb_write(output);
        if (flag){
          // Prepare next memory address
          bool stack_flag = !mystack.isEmpty();
          fsm = (stack_flag) ? FSM::readMem : FSM::result_end;
          mem_position = (stack_flag) ? mystack.pop() : mem_position;
        } else {
          fsm = FSM::result;
        }
        break;
      }
      case (FSM::result_end):{
        // End of range search flag
        output.data.node = Node::empty();
        output.data.distance = -1;
        output.data.eval = true;
        // Write output
        bool flag = out.nb_write(output);
        if (flag){
          // Prepare next memory address
          bool stack_flag = !mystack.isEmpty();
          fsm = (stack_flag) ? FSM::readMem : FSM::result_end;
          mem_position = (stack_flag) ? mystack.pop() : mem_position;
        } else {
          fsm = FSM::result;
        }
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

// 6. Nearest NEighbor search in KDTree --- NOT STACKLESS (yet)
template<int Depth, int Banks>
class NNSearch_Unit{
  // TokenTypes
  typedef Token<DataIn,cbuf_size> TokenDataIn;
  typedef Token<Out,cbuf_size> TokenOut;
  // Memory Types
  typedef Flit<MemoryIn <Node,Depth>,Banks> MemIn;
  typedef Flit<MemoryOut<Node      >,Banks> MemOut;
  // Address type
  typedef ac_int<Depth,false> Addr_t;
  // FSM
  enum FSM {
    newOp, readMem, waitMem,
    logic_first_neighbor, logic_search, result, finish};
private:
  // Command Input
  TokenDataIn input;
  // Command Output
  TokenOut output;
  // Memory results
  MemOut mem_out;
  Addr_t mem_position;
  // Internal state
  FSM fsm;
  FSM fsm_next;
  // Internal data
  Stack<Addr_t,Depth> mystack;
public:
  // Constructor
  NNSearch_Unit():fsm(FSM::newOp){};
  // Interface
  #pragma hls_design interface
  void run  (
    ac_channel<TokenDataIn>& in,
    ac_channel<TokenOut>& out,
    ac_channel<MemIn>& mem_req,
    ac_channel<MemOut>& mem_res,
    ac_channel<bool>& finish,
    Addr_t StackMem[1<<Depth]
  ){
    switch (fsm){
      case (FSM::newOp):{
        // Get new input
        bool flag = in.nb_read(input);
        fsm = (flag) ? FSM::readMem : FSM::newOp;
        mem_position = 0;
        fsm_next = FSM::logic_first_neighbor;
        // Initialize Variables
        mystack.setMemory(StackMem);
        output = TokenOut::copyToken(input);
        break;
      }
      case (FSM::readMem):{
        // Read memory @ {mem_position}
        MemIn tmp = set_input<Depth,Banks>(mem_position,Node::empty(),false);
        bool flag = mem_req.nb_write(tmp);
        fsm = (flag) ? FSM::waitMem : FSM::accessMem;
        break;
      }
      case (FSM::waitMem):{
        // Wait for memory output
        bool flag = mem_res.nb_read(mem_out);
        fsm = (flag) ? fsm_next : FSM::waitMem;
        break;
      }
      case (FSM::logic_first_neighbor):{
        // First neighbor logic
        Node input_node = input.data.node;
        Node current_node = mem_out.data.data;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool right = input_node.coords[axis] > current_node.coords[axis];
        // Children positions
        Addr_t pos_left = left_child(position);
        Addr_t pos_right = right_child(position);
        Addr_t child = (right) ? pos_right : pos_left;
        // Distances
        int current_distance_square = dist(current_node, input_node);
        // Decide next action
        if (!node.is_leaf){                     // Plane or Empty ------------
          Addr_t tmp_addr;
          if (!node.has_left && !node.has_right){       // ------------- Empty
            // Error, fix later
          }
          else if (node.has_left && !node.has_right){   // --------- Left only
            tmp_addr = pos_left;
          }
          else if (!node.has_left && node.has_right){   // -------- Right only
            tmp_addr = pos_right;
          }
          else {
            tmp_addr = child;
          }
          fsm = FSM::readMem;
          mem_position = tmp_addr;
          fsm_next = FSM::logic_first_neighbor;
        } else {                                // Leaf ----------------------
          output.data.node = current_node;
          output.data.distance = current_distance_square;
          fsm = FSM::readMem;
          mem_position = 0;
          fsm_next = FSM::logic_search;
        }                                       // End -----------------------
        break;
      }
      case (FSM::logic_search):{
        // Nearest Neighbor search operation logic
        Node input_node = input.data.node;
        Node current_node = mem_out.data.data;
        int best_distance_square = output.data.distance;
        // Calculate flags
        int axis = get_depth(mem_position) % Dims;
        bool equal = input_node.coords[axis] == current_node.coords[axis];
        bool right = input_node.coords[axis] > current_node.coords[axis];
        // Children positions
        Addr_t pos_left = left_child(mem_position);
        Addr_t pos_right = right_child(mem_position);
        // Distances
        // For leafs
        int current_distance_square = dist(current_node, input_node);
        // For planes
        int current_range_square = input_node.coords[axis] - current_node.coords[axis];
        current_range_square = current_range_square * current_range_square;
        // Decide next action
        if (!node.is_leaf){                   // Plane or Empty --------------------
          bool immediate_flag;
          if (right){                           // Plane right of center -----------
            if (current_node.has_left){           // Plane has left -> push --------
              mem_position = pos_left;
              immediate_flag = true;
            }
            if (current_node.has_right &&         // Plane has right -> evaluate ---
              (current_range_square < best_distance_square)
            ){
              if (immediate_flag){
                mystack.push(pos_right);
              } else {
                mem_position = pos_right;
                immediate_flag = true;
              }
            }
          } else if (equal){                    // Plane on top of center ----------
            if (current_node.has_left){           // Plane has left -> push --------
              mem_position = pos_left;
              immediate_flag = true;
            }
            if (current_node.has_right){          // Plane has right -> push -------
              if (immediate_flag){
                mystack.push(pos_right);
              } else {
                mem_position = pos_right;
                immediate_flag = true;
              }
            }
            mystack.push(pos_right);
          } else {                              // Plane left of center ------------
            if (current_node.has_right){          // Plane has right -> push -------
              mem_position = pos_right;
              immediate_flag = true;
            }
            if (current_node.has_right &&         // Plane has left -> evaluate ----
              (current_range_square < best_distance_square)
            ){
              if (immediate_flag){
                mystack.push(pos_right);
              } else {
                mem_position = pos_right;
                immediate_flag = true;
              }
            }
          }
          // Next state
          if (immediate_flag){
            fsm = FSM::readMem;
            // mem_position = mem_position
            fsm_next = FSM::logic_search;
          } else {
            bool flag = !mystack.isEmpty();
            fsm = (flag) ? FSM::readMem : FSM::result;
            mem_position = (flag) ? mystack.pop() : mem_position;
            fsm_next = FSM::logic_search;
          }
        } else {                                // Leaf ----------------------
          if (current_distance_square < best_distance_square){
            output.data.node = current_node;
            output.data.distance = current_distance_square;
          }
          bool flag = !mystack.isEmpty();
          fsm = (flag) ? FSM::readMem : FSM::result;
          mem_position = (flag) ? mystack.pop() : mem_position;
          fsm_next = FSM::logic_search;
        }                                       // End -----------------------
        break;
      }
      case (FSM::result):{
        // Write output
        bool flag = out.nb_write(output);
        fsm = (flag) ? FSM::finish : FSM::result;
        break;
      }
      case (FSM::finish):{
        // Write finish signal
        bool flag = finish.nb_write(true);
        fsm = (flag) ? FSM::newOp : FSM::finish;
        break;
      }
    }
  };
};

#ifndef __SYNTHESIS__
template<int Depth, int Banks>
class printNode{
  enum FSM {start, print, finish};
private:
  // Memory results
  Flit<MemoryOut<Node      >,Banks> mem_out;
  bool mem_op;
  // Internal variables
  FSM fsm;
  ac_int<Depth,false> in_position;
public:
  // Constructor;
  printNode():mem_op(false),fsm(FSM::start){};
  // Interface
  run(
    ac_int<Depth,false> position,
    bool finish_flag,
    ac_channel<Flit<MemoryIn <Node,Depth>,Banks> > &mem_req,
    ac_channel<Flit<MemoryOut<Node      >,Banks> > &mem_res
  ){
    finish_flag = false;
    if (!mem_op){
      switch (fsm){
      case FSM::start:
        // Get position
        in_position = position;
        // Retrieve current node data
        Node tmp;
        mem_req.write(set_input<Depth,Banks>(in_position,tmp,false));   // Read
        mem_op = true;
        fsm = FSM::print;
        break;
      case FSM::print:
        int axis = get_depth(in_position) % Dims;
        Node n = mem_out.data.data;
        // Print node
        if (n.is_leaf){   // Leaf
          std::cout << "{" << n.coords[0];
          for (int i=1; i<Dims; ++i){
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
        fsm = FSM::finish;
        break;
      case FSM::finish:
        fsm = (in_position==position) ? FSM::finish : FSM::start;
        finish_flag = true;
        break;
      }
    }
    else{
      mem_op = !(mem_res.nb_read(mem_out));
    }

  };
};

template<int Depth, int Banks>
class printTree{
  enum FSM {start, check, finish};
private:
  // Command Input
  Eval in_eval;
  bool active;
  // Print node block
  printNode<Depth,Banks> prtNd;
  // Internal variables;
  FSM fsm;
  int i;
  int j;
  bool print_flag;
public:
  // Constructor
  printTree():active(false){};
  // Interface
  run(
    EvalChannel& in,
    ac_channel<Flit<MemoryIn <Node,Depth>,Banks> > &mem_req,
    ac_channel<Flit<MemoryOut<Node      >,Banks> > &mem_res
  ){
    if (!active){     // Get Input (Non - Blocking)
      active = in.nb_read(in_eval);     // Retrieve the input node
      fsm = FSM::start;
      i = 0;
      j = 0;
    }
    else {
      switch (fsm){
      case FSM::start:        // Outer loop -- i
        std::cout << i << ": ";
        print_flag = false;
        fsm = FSM::check;
        break;
      case FSM::check:        // Inner loop -- j
        std::cout << j << ": ";
        if (!print_flag){
          prtNd.run(j, print_flag, mem_req, mem_res);
        } else {
          j += 1;
          print_flag = false;
        }
        fsm = (j < (2<<i)-1) ? FSM::check : FSM::finish ;
        break;
      case FSM::finish:
      std::cout << std::endl;
        i += 1;
        active = (i<Depth);
        fsm = FSM::start;
        break;
      }
    }

  };
};

#endif

#endif
