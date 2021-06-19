#ifndef COMPLETION_BUFFER_STRUCTURE
#define COMPLETION_BUFFER_STRUCTURE

#include "_2c_data_structures.hpp"

template<typename IN, typename OUT, int size>
class CompletionBuffer{
  typedef Token<In,size> TokenIn;
  typedef Token<Out,size> TokenOut;
private:
  // Buffer variables
  OUT buffer[size];
  bool buf_vld[size];
  bool buf_block[size];
  int i;
  int o;
  int cnt;
  // GetToken internal state
  TokenIn tk;
  bool getToken_actv;
  bool token_valid;
  // Put interna state
  TokenOut out;
  bool put_actv;
public:
  // Constructor
  CompletionBuffer():
  i(0),o(0),cnt(0),
  getToken_actv(false){};
  // Methods
  void getToken(
    ac_channel<IN> &new_input,
    ac_channel<TokenIn> &token_input
  ){
    // Get input
    if(!getToken_actv){
      getToken_actv = new_input.nb_read(tk.data);
      token_valid = false;
    }
    // Generate new token
    if (getToken_actv && !token_valid){
      if (cnt < size) {
        // Bind one buffer position
        buf_vld[i] = false;
        buf_block[i] = (tk.data.opcode == OP::op_rangeSearch) ? true : false;
        // Get token
        tk.token = i;
        token_valid = true;
        // Next state
        cnt += 1;
        i = (i==size) ? 0 : i+1;
      }
    }
    // Write to output
    if (getToken_actv && token_valid){
      getToken_actv = !token_input.nb_write(tk);
    }
  };
  void put(
    ac_channel<TokenOut> &token_output
  ){
    // Get input
    if (!put_actv){
      put_actv = token_output.nb_read(out);
    }
    // Write to buffer
    if (put_actv && !buf_vld[out.token]){
      buf_vld[out.token] = true;
      if (
        (out.data.node == Node::empty()) &&
        (out.data.distance == -1) &&
        (out.data.eval == true)
      ){
        buf_block[out.token] = false;
      }
      buffer[out.token] = out.data;
      put_actv = false;
    }
  };
  void getResult(
    ac_channel<OUT> &reordered_output
  ){
    // Oldest operation completed
    if ((cnt > 0) && buf_vld[o]){
      // Write to output
      bool flag = reordered_output.nb_write(buffer[o]);
      // Next state
      if (flag){
        // Range search override
        if (buf_block[o]){
          buf_vld[o] = false;
        }
        // Other operations or end of range search
        else {
          o = (o==size) ? 0 : o+1;
          cnt -= 1;
        }
      }
    }
  };
  // Interface
  #pragma hls_design interface
  void run(
    // getToken interface
    ac_channel<IN> &new_input,
    ac_channel<TokenIn> &token_input,
    // put interface
    ac_channel<TokenOut> &token_output,
    // getResult interface
    ac_channel<OUT> &reordered_output
  ){
    // Order for maximum throughput:
    // 1. Fill buffer
    put(token_output);
    // 2. Release token
    getResult(reordered_output);
    // 3. Get input
    getToken(new_input, token_input);
  };
};

#endif
