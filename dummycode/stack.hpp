#ifndef STACK_STRUCTURE
#define STACK_STRUCTURE

class stack{
  private:
    int *arr;
    int rear;
public:
    stack(int *mem);
    void push(int d_in);
    void pop(int& d_out);
};

stack::stack(int *mem):
arr(mem),rear(0){
  arr[rear] = -1;
}

void stack::push(int d_in){
  rear = (rear + 1);
  arr[rear] = d_in;
}

void stack::pop(int& d_out){
  d_out = arr[rear];
  rear = (rear - 1);
}

#endif
