#ifndef _INTERPRETER
#define _INTERPRETER

// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Value it is bound to. Specifically how you implement the list of bindings is
// up to you.
struct Frame {
    Value *bindings;
    struct Frame *parent;
};

typedef struct Frame Frame;

Value *nullCheck(Value *args, Frame *frame);
Value *plus(Value *args, Frame *frame);
Value *carPrimitive(Value *args, Frame *frame);
Value *cdrPrimitive(Value *args, Frame *frame);
Value *consPrimitive(Value *args, Frame *frame);
Value *equal(Value *args, Frame *frame);
Value *multiply(Value *args, Frame *frame);
Value *minus(Value *args, Frame *frame);
Value *begin(Value *args, Frame *frame);
Value *evalCond(Value *args, Frame *frame);
Value *greaterThan(Value *args, Frame *frame);
Value *lessThan(Value *args, Frame *frame);
Value *divide(Value *args, Frame *frame);
Value *modulo(Value *args, Frame *frame);





void interpret(Value *tree);
Value *eval(Value *expr, Frame *frame);

#endif
