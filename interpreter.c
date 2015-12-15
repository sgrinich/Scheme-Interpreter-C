//Stephen Grinich, Anmol Raina, Andrew Yang

#include "linkedlist.h"
#include "talloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "value.h"
#include "parser.h"
#include "interpreter.h"

Frame *curFrame;
Frame *globalFrame;
int shouldPrint;
int hitCons;
int isFunction;
int isLet;
int ifCondShouldPrint;
int funcIsRunning;
int boundToClosure;
int letStar;
int inAnd;
int andCount;
int orCount;
int letLet;
int arithCount;

// Handles evaluation errors
void evalError(){
    printf("Evaluation error!\n");
    texit(0);
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    Value *newFuncBinding = makeNull();
    newFuncBinding = cons(value, newFuncBinding);

    Value *funcName = talloc(sizeof(Value));
    funcName->type = SYMBOL_TYPE;
    funcName->s = name;

    newFuncBinding = cons(funcName, newFuncBinding);

    frame->bindings = cons(newFuncBinding, frame->bindings);

}


//** remember to pop off "used up" frame bindings from let expression
void interpret(Value *tree){
    globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    globalFrame->parent = '\0';


    bind("null?",nullCheck,globalFrame);
    bind("+",plus,globalFrame);
    bind("car",carPrimitive,globalFrame);
    bind("cdr",cdrPrimitive,globalFrame);
    bind("cons",consPrimitive,globalFrame);
    bind("=",equal,globalFrame);
    bind("*",multiply,globalFrame);
    bind("-", minus, globalFrame);
    bind("begin",begin, globalFrame);
    bind("cond",evalCond,globalFrame);
    bind(">", greaterThan,globalFrame);
    bind("<", lessThan,globalFrame);
    bind("/", divide,globalFrame);
    bind("modulo", modulo,globalFrame);


    curFrame = globalFrame;

    shouldPrint = 1;
    funcIsRunning = 0;
    boundToClosure = 0;
    letStar = 0;
    letLet = 0;
    arithCount = 0;
    // inAnd = 0;
    // declare curFrame parent to not have uninitialized value later.
    Value *returnedOutput;
    // S-EXPRESSION LOOP
    while(tree->type != NULL_TYPE){
        arithCount = 0;
        andCount = 0;
        orCount = 0;
        hitCons = 0;
        returnedOutput = eval(car(tree),curFrame);
        curFrame = globalFrame;
        tree = cdr(tree);
        funcIsRunning = 0;
        letStar = 0;
        // boundToClosure= 0;
    }
}


// This function looks back at previous frames and sees if we can find a variable's value
Value *getValFromBinding(Value *symbol, Frame *frame){
    int count = 0;
    while(frame){
        // go through bindings
        Value *curBinding = frame->bindings;
        while(curBinding->type != NULL_TYPE){
            if((car(cdr(car(curBinding))))->type == CONS_TYPE){
                if(car(car(cdr(car(curBinding))))->type == SYMBOL_TYPE){
                    if(!strcmp(car(car(cdr(car(curBinding))))->s,"lambda")){

                        boundToClosure = 1;
                    }
                }
            }
            if(!strcmp((car(car(curBinding)))->s,symbol->s)){
                return car(cdr(car(curBinding)));
            }
            curBinding = cdr(curBinding);
        }
        count++;
        frame = frame->parent;
    }

    printf("Binding not found.\n");
    evalError();
}

Frame *getFrameForLetStar(Value *symbol, Frame *frame){
    int count = 0;
    while(frame){

        // go through bindings
        Value *curBinding = frame->bindings;
        while(curBinding->type != NULL_TYPE){
            if(!strcmp((car(car(curBinding)))->s,symbol->s)){
                return frame;
            }
            curBinding = cdr(curBinding);
        }
        count++;
        frame = frame->parent;
    }
    printf("Binding not found.\n");
    evalError();
}

Value *nullCheck(Value *args, Frame *frame) {
    // check if there are no arguments, i.e. it looks like (null? )
    if(args->type == NULL_TYPE){
        printf("Error, no arguments given to check if null \n");
        evalError();
    }
    else if(cdr(args)->type != NULL_TYPE){
        printf("Too many arguments for null?\n");
        evalError();
    }
    else{
        Value *trueVal = talloc(sizeof(Value));
        trueVal->type = BOOL_TYPE;
        Value *nullBinding = args;
        if (car(args)->type == SYMBOL_TYPE) {
            Value *binding = makeNull();
            binding = cons(getValFromBinding(car(args), frame),binding);
            nullBinding = binding;
        }

        if(car(nullBinding)->type == NULL_TYPE){
            printf("True\n");
            trueVal->i = 1;
        }

        else{
            printf("False\n");
            trueVal->i = 0;
        }
        return trueVal;
    }
}



Value *carPrimitive(Value *args, Frame *frame){
    Value *binding = makeNull();
    Value *toCheck = args;

    if(car(args)->type == SYMBOL_TYPE){
        shouldPrint = 0;
        toCheck = eval(getValFromBinding(car(args), frame), frame);
        shouldPrint = 1;
        toCheck = car(toCheck);
    }

    else if (car(args)->type != CONS_TYPE) {
        printf("Trying to car a non-cons cell\n");
        evalError();
    }

    else if (car(car(args))->type == SYMBOL_TYPE) {
        if(!strcmp(car(car(args))->s, "quote")){
            toCheck = car(cdr(car(args)));
            if(cdr(cdr(car(args)))->type != NULL_TYPE){
                evalError();
            }
        }
        else{
            if(car(car(args))->type == SYMBOL_TYPE){
                shouldPrint = 0;
                toCheck = eval(car(args), frame);
                shouldPrint = 1;
            }
            else{
                binding = cons(getValFromBinding(car(args), frame),binding);
                toCheck = binding;
            }

        }
    }
    else{
        printf("Missing quote.\n");
        evalError();
    }

    if(toCheck->type == NULL_TYPE || cdr(args)->type != NULL_TYPE ){
        printf("THIS EVAL ERROR\n");
        evalError();
    }

    return car(toCheck);

}


Value *cdrPrimitive(Value *args, Frame *frame){
    Value *binding = makeNull();
    Value *toCheck = args;

    if(car(args)->type == SYMBOL_TYPE){
        shouldPrint = 0;
        toCheck = eval(getValFromBinding(car(args), frame), frame);
        shouldPrint = 1;
        toCheck = cdr(car(toCheck));
    }

    else if (car(args)->type != CONS_TYPE) {
        printf("Trying to car a non-cons cell\n");
        evalError();
    }

    else if (car(car(args))->type == SYMBOL_TYPE) {
//        printf("SYMBOL TYPE\n");
        if(!strcmp(car(car(args))->s, "quote")){
//            printf("Quote seen\n");
            toCheck = cdr(car(cdr(car(args))));
            if(cdr(cdr(car(args)))->type != NULL_TYPE){
                evalError();
            }
        }
        else{
            if(car(car(args))->type == SYMBOL_TYPE){
                shouldPrint = 0;
                toCheck = cdr(eval(car(args), frame));
                shouldPrint = 1;
            }
            else{
                binding = cons(getValFromBinding(car(args), frame),binding);
                toCheck = binding;
            }
        }
    }
    else{
        printf("Missing quote.\n");
        evalError();
    }

    if(toCheck->type == NULL_TYPE){
        return toCheck;
    }
    if( cdr(args)->type != NULL_TYPE ){
        evalError();
    }

    return toCheck;
}

Value *consPrimitive(Value *args, Frame *frame) {
    if (args->type == NULL_TYPE){
        printf("EVAL ERROR 1\n");
        evalError();
    }
   if (cdr(args)->type == NULL_TYPE){
       printf("EVAL ERROR 2\n");
        evalError();
    }
   if (cdr(cdr(args))->type != NULL_TYPE ){
       printf("EVAL ERROR 3\n");
        evalError();
    }
    Value *secondArg = talloc(sizeof(Value));
    secondArg = cdr(args);

    if (car(cdr(args))->type == SYMBOL_TYPE) {
        secondArg = getValFromBinding(car(cdr(args)), frame);
        if(!strcmp(car(secondArg)->s,"quote")){
            shouldPrint = 0;
            secondArg = car(eval(secondArg,frame));
            shouldPrint = 1;
        }
        else {
            evalError();
        }
        printf("\n");
    }

    else if (cdr(args)->type == CONS_TYPE) {
        if(!strcmp(car(car(cdr(args)))->s, "quote")){
            secondArg = car(cdr(car(cdr(args))));
        }
    }

    else{
        printf("EVAL ERROR 4");
        evalError();
    }
    Value *firstArg = car(args);

    if(car(args)->type == CONS_TYPE){

         if(car(car(args))->type == SYMBOL_TYPE){
              if(!strcmp(car(car(args))->s, "quote")){
                 firstArg = car(cdr(car(args)));
             }
            else {
                shouldPrint = 0;
                firstArg = eval(car(args), frame);
                shouldPrint = 1;
            }
         }
        else{
            evalError();
        }
    }

    Value *returnVal = secondArg;
    returnVal = cons(firstArg, returnVal);
    return returnVal;
}

Value *equal(Value *args, Frame *frame){
    ifCondShouldPrint = 1;
    Value *returnVal = talloc(sizeof(Value));
    returnVal->type = BOOL_TYPE;

    if(args->type == NULL_TYPE){
        evalError();
    }

     int counter = 0;
     Value *argsCounter = args;
     while(argsCounter->type != NULL_TYPE){
         counter++;
         argsCounter = cdr(argsCounter);
     }

     // Validating number of elements after an if statement
     if(counter != 2){
         printf("Not enough number of parameters\n");
         evalError();
     }

     else{

        Value *binding = makeNull();

        Value *prevCompare = talloc(sizeof(Value));
        prevCompare->type = DOUBLE_TYPE;
        Value *toCompare = talloc(sizeof(Value));
        toCompare->type = DOUBLE_TYPE;
        Value *argsCheck = makeNull();

        int counter = 0;
        while(args->type != NULL_TYPE){

            argsCheck = args;

            if (car(args)->type == SYMBOL_TYPE) {
                argsCheck = cons(getValFromBinding(car(args), frame),argsCheck);
            }

             if(car(argsCheck)->type == VOID_TYPE || car(argsCheck)->type == NULL_TYPE){
                evalError();
              }

            if(car(argsCheck)->type == INT_TYPE){
                toCompare->d = car(argsCheck)->i;
            }

            else if (car(argsCheck)->type == DOUBLE_TYPE) {
                toCompare->d = car(argsCheck)->d;
            }

            else if(car(argsCheck)->type == CONS_TYPE){
                shouldPrint = 0;
                ifCondShouldPrint = 0;
                Value *returnedVal = eval(car(argsCheck), frame);
                shouldPrint = 1;
                ifCondShouldPrint = 1;

                 if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    toCompare->d = returnedVal->d;
                    }
            }
            else {

                printf("Trying to add non-numbers\n");
                evalError();
            }
            // check if equal
            if (counter == 1){
                if (toCompare->d == prevCompare->d){
                    returnVal->i = 1;

                    return returnVal;
                }
            }

            prevCompare->d = toCompare->d;
            args = cdr(args);
            counter ++;
        }

    }

    returnVal->i = 0;
    return returnVal;
}


Value *greaterThan(Value *args, Frame *frame){
    ifCondShouldPrint = 1;
    Value *returnVal = talloc(sizeof(Value));
    returnVal->type = BOOL_TYPE;

    if(args->type == NULL_TYPE){
        evalError();
    }

     int counter = 0;
     Value *argsCounter = args;
     while(argsCounter->type != NULL_TYPE){
         counter++;
         argsCounter = cdr(argsCounter);
     }

     // Validating number of elements after an if statement
     if(counter != 2){
         printf("Not enough number of parameters\n");
         evalError();
     }

     else{

        Value *binding = makeNull();

        Value *prevCompare = talloc(sizeof(Value));
        prevCompare->type = DOUBLE_TYPE;
        Value *toCompare = talloc(sizeof(Value));
        toCompare->type = DOUBLE_TYPE;
        Value *argsCheck = makeNull();
        int counter = 0;
        while(args->type != NULL_TYPE){

            argsCheck = args;

            if (car(args)->type == SYMBOL_TYPE) {
                argsCheck = cons(getValFromBinding(car(args), frame),argsCheck);
                // toCompare = binding;
            }

             if(car(argsCheck)->type == VOID_TYPE || car(argsCheck)->type == NULL_TYPE){
                evalError();
        }

            if(car(argsCheck)->type == INT_TYPE){
                toCompare->d = car(argsCheck)->i;
            }

            else if (car(argsCheck)->type == DOUBLE_TYPE) {
                toCompare->d = car(argsCheck)->d;
            }

            else if(car(argsCheck)->type == CONS_TYPE){
                shouldPrint = 0;
                ifCondShouldPrint = 0;
                Value *returnedVal = eval(car(argsCheck), frame);
                shouldPrint = 1;
                ifCondShouldPrint = 1;

                 if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    toCompare->d = returnedVal->d;
                    }
            }
            else {

                printf("Can't compare non-numbers\n");
                evalError();
            }
            // check if equal
            if (counter == 1){
                if (toCompare->d < prevCompare->d){
                    returnVal->i = 1;

                    return returnVal;
                }
            }

            prevCompare->d = toCompare->d;
            args = cdr(args);
            counter ++;
        }

    }

    returnVal->i = 0;
    return returnVal;
}


Value *lessThan(Value *args, Frame *frame){
    ifCondShouldPrint = 1;
    Value *returnVal = talloc(sizeof(Value));
    returnVal->type = BOOL_TYPE;

    if(args->type == NULL_TYPE){
        evalError();
    }

     int counter = 0;
     Value *argsCounter = args;
     while(argsCounter->type != NULL_TYPE){
         counter++;
         argsCounter = cdr(argsCounter);
     }

     // Validating number of elements after an if statement
     if(counter != 2){
         printf("Not enough number of parameters\n");
         evalError();
     }

     else{

        Value *binding = makeNull();

        Value *prevCompare = talloc(sizeof(Value));
        prevCompare->type = DOUBLE_TYPE;
        Value *toCompare = talloc(sizeof(Value));
        toCompare->type = DOUBLE_TYPE;
        Value *argsCheck = makeNull();
        // toCompare->type = NULL_TYPE;
        // toCompare->i = 0;
        int counter = 0;
        while(args->type != NULL_TYPE){

            argsCheck = args;

            if (car(args)->type == SYMBOL_TYPE) {
                argsCheck = cons(getValFromBinding(car(args), frame),argsCheck);
                // toCompare = binding;
            }

             if(car(argsCheck)->type == VOID_TYPE || car(argsCheck)->type == NULL_TYPE){
                evalError();
        }

            if(car(argsCheck)->type == INT_TYPE){
                toCompare->d = car(argsCheck)->i;
            }


            else if (car(argsCheck)->type == DOUBLE_TYPE) {
                toCompare->d = car(argsCheck)->d;
            }

            else if(car(argsCheck)->type == CONS_TYPE){
                shouldPrint = 0;
                ifCondShouldPrint = 0;
                Value *returnedVal = eval(car(argsCheck), frame);
                shouldPrint = 1;
                ifCondShouldPrint = 1;

                 if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    toCompare->d = returnedVal->d;
                    }
            }
            else {

                printf("Can't compare non-numbers\n");
                evalError();
            }
            // check if equal
            if (counter == 1){
                if (toCompare->d > prevCompare->d){
                    returnVal->i = 1;

                    return returnVal;
                }
            }

            prevCompare->d = toCompare->d;
            args = cdr(args);
            counter ++;
        }

    }

    returnVal->i = 0;
    return returnVal;
}









Value *multiply(Value *args, Frame *frame){
    arithCount++;
    Value *productVal = talloc(sizeof(Value));
    productVal->type = DOUBLE_TYPE;
    productVal->d = 1;

    if(args->type == NULL_TYPE){
      return productVal;
    }

    else{
      Value *binding = makeNull();
      while(args->type != NULL_TYPE){
        Value *toMult = args;
        if(car(args)->type == SYMBOL_TYPE){
          binding = cons(getValFromBinding(car(args), frame), binding);
          toMult = binding;
        }

         if(car(toMult)->type == VOID_TYPE || car(toMult)->type == NULL_TYPE){
                evalError();
        }

        if(car(toMult)->type == INT_TYPE){
          productVal->d *= car(toMult)->i;
        }

        else if(car(toMult)->type == DOUBLE_TYPE){
          productVal->d *= car(toMult)->d;
        }

        else if(car(toMult)->type == CONS_TYPE){
          // shouldPrint = 0;
          Value *returnedVal = eval(car(toMult), frame);
          // shouldPrint = 1;
           if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                evalError();
        }
          if(returnedVal->type == DOUBLE_TYPE){
            productVal->d *= returnedVal->d;
          }
        }

        else{
          printf("Trying to multiply non-numbers\n");
          evalError();
        }

        args = cdr(args);

      }
      arithCount--;
      return productVal;
    }

}

Value *minus(Value *args, Frame *frame){
    arithCount++;
    Value *differenceVal = talloc(sizeof(Value));
    differenceVal->type = DOUBLE_TYPE;
    if(args->type == NULL_TYPE){
        printf("Need at least one argument for -\n");
        evalError();
    }

    else{


      // need to get first value and then start subtracting from that
      differenceVal->d = 0;
      Value *toMinus = args;
      Value *binding = makeNull();

      if (car(args)->type == SYMBOL_TYPE) {
          binding = cons(getValFromBinding(car(args), frame),binding);
          toMinus = binding;
      }

       if(car(toMinus)->type == VOID_TYPE || car(toMinus)->type == NULL_TYPE){
                evalError();
        }

      if(car(toMinus)->type == INT_TYPE){
          differenceVal->d = car(toMinus)->i;
      }


      else if (car(toMinus)->type == DOUBLE_TYPE) {
          differenceVal->d = car(toMinus)->d;
      }

      else if(car(toMinus)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toMinus), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    differenceVal->d = returnedVal->d;
                }
            }

      else{
        printf("can't subtract nonnumbers\n");
        evalError();
      }

      args = cdr(args);

      //printf("tominus num: %d\n", car(toMinus)->i);



        while(args->type != NULL_TYPE){
            toMinus = args;
            if (car(args)->type == SYMBOL_TYPE) {
                binding = cons(getValFromBinding(car(args), frame),binding);
                toMinus = binding;
            }

            if(car(toMinus)->type == INT_TYPE){
                differenceVal->d -= car(toMinus)->i;
            }


            else if (car(toMinus)->type == DOUBLE_TYPE) {
                differenceVal->d -= car(toMinus)->d;
            }

            else if(car(toMinus)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toMinus), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    differenceVal->d -= returnedVal->d;
                }
            }
            else {
                printf("Trying to subtract non-numbers\n");
                evalError();
            }
            args = cdr(args);
        }
        arithCount--;
        return differenceVal;
    }
}



Value *divide(Value *args, Frame *frame){
    arithCount++;
    Value *quotientVal = talloc(sizeof(Value));
    quotientVal->type = DOUBLE_TYPE;
    if(args->type == NULL_TYPE){
        printf("Need at least one argument for -\n");
        evalError();
    }

    else{


      // need to get first value and then start subtracting from that
      quotientVal->d = 0;
      Value *toDivide = args;
      Value *binding = makeNull();

      if (car(args)->type == SYMBOL_TYPE) {
          binding = cons(getValFromBinding(car(args), frame),binding);
          toDivide = binding;
      }

       if(car(toDivide)->type == VOID_TYPE || car(toDivide)->type == NULL_TYPE){
                evalError();
        }

      if(car(toDivide)->type == INT_TYPE){
          quotientVal->d = car(toDivide)->i;
      }


      else if (car(toDivide)->type == DOUBLE_TYPE) {
          quotientVal->d = car(toDivide)->d;
      }

      else if(car(toDivide)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toDivide), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    quotientVal->d = returnedVal->d;
                }
            }

      else{
        printf("can't divide nonnumbers\n");
        evalError();
      }

      args = cdr(args);


        while(args->type != NULL_TYPE){
            toDivide = args;
            if (car(args)->type == SYMBOL_TYPE) {
                binding = cons(getValFromBinding(car(args), frame),binding);
                toDivide = binding;
            }

            if(car(toDivide)->type == INT_TYPE){
                quotientVal->d /= car(toDivide)->i;
            }


            else if (car(toDivide)->type == DOUBLE_TYPE) {
                quotientVal->d /= car(toDivide)->d;
            }

            else if(car(toDivide)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toDivide), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    quotientVal->d /= returnedVal->d;
                }
            }
            else {
                printf("Trying to divide non-numbers\n");
                evalError();
            }
            args = cdr(args);
        }
        arithCount--;
        return quotientVal;
    }
}


Value *modulo(Value *args, Frame *frame){
    arithCount++;
    Value *remainderVal = talloc(sizeof(Value));
    remainderVal->type = DOUBLE_TYPE;
    if(args->type == NULL_TYPE){
        evalError();
    }

     int counter = 0;
     Value *argsCounter = args;
     while(argsCounter->type != NULL_TYPE){
         counter++;
         argsCounter = cdr(argsCounter);
     }

     // Validating number of elements after an if statement
     if(counter != 2){
         printf("Not enough number of parameters\n");
         evalError();
     }

    else{


      // need to get first value and then start subtracting from that
      remainderVal->d = 0;
      Value *toModulo = args;
      Value *binding = makeNull();

      if (car(args)->type == SYMBOL_TYPE) {
          binding = cons(getValFromBinding(car(args), frame),binding);
          toModulo = binding;
      }

       if(car(toModulo)->type == VOID_TYPE || car(toModulo)->type == NULL_TYPE){
                evalError();
        }

      if(car(toModulo)->type == INT_TYPE){
          remainderVal->d = car(toModulo)->i;
      }


      else if (car(toModulo)->type == DOUBLE_TYPE) {
          remainderVal->d = car(toModulo)->d;
      }

      else if(car(toModulo)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toModulo), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    remainderVal->d = returnedVal->d;
                }
            }

      else{
        printf("can't Modulo nonnumbers\n");
        evalError();
      }

      args = cdr(args);


        while(args->type != NULL_TYPE){
            toModulo = args;
            if (car(args)->type == SYMBOL_TYPE) {
                binding = cons(getValFromBinding(car(args), frame),binding);
                toModulo = binding;
            }

            if(car(toModulo)->type == INT_TYPE){
                remainderVal->d = (int)remainderVal->d % (int)car(toModulo)->i;
            }


            else if (car(toModulo)->type == DOUBLE_TYPE) {
                remainderVal->d = (int)remainderVal->d % (int)car(toModulo)->d;
            }

            else if(car(toModulo)->type == CONS_TYPE){
                // shouldPrint = 0;
                Value *returnedVal = eval(car(toModulo), frame);
                // shouldPrint = 1;

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                    evalError();
                }

                if(returnedVal->type == DOUBLE_TYPE){
                    remainderVal->d = (int)remainderVal->d % (int)returnedVal->d;
                }
            }
            else {
                printf("Trying to Modulo non-numbers\n");
                evalError();
            }
            args = cdr(args);
        }
        arithCount--;
        return remainderVal;
    }
}


Value *begin(Value *args, Frame *frame){
    shouldPrint = 0;
    Value *valToReturn = talloc(sizeof(Value));
    valToReturn->type = VOID_TYPE;

    while(args->type != NULL_TYPE){
        valToReturn = eval(car(args), frame);
        args = cdr(args);
    }


    shouldPrint = 1;
    return valToReturn;
}


Value *plus(Value *args, Frame *frame){
    arithCount ++;
    Value *sumVal = talloc(sizeof(Value));
    sumVal->type = DOUBLE_TYPE;
    sumVal->d = 0;
    if(args->type == NULL_TYPE){
        return sumVal;
    }

    else{
        Value *binding = makeNull();
        while(args->type != NULL_TYPE){
            Value *toAdd = args;
            if (car(args)->type == SYMBOL_TYPE) {
                binding = cons(getValFromBinding(car(args), frame),binding);
                toAdd = binding;
            }

            if(car(toAdd)->type == VOID_TYPE || car(toAdd)->type == NULL_TYPE){
                evalError();
            }

            if(car(toAdd)->type == INT_TYPE){
                sumVal->d += car(toAdd)->i;
            }

            else if (car(toAdd)->type == DOUBLE_TYPE) {
                sumVal->d += car(toAdd)->d;
            }

            else if(car(toAdd)->type == CONS_TYPE){

                // shouldPrint = 0;
                // printTree(car(toAdd));
                Value *returnedVal = eval(car(toAdd), frame);

                if(returnedVal->type == VOID_TYPE || returnedVal->type == NULL_TYPE || returnedVal->type == CONS_TYPE){
                // printf("Returnedval is null type\n");
                    evalError();
                 }
                // shouldPrint = 1;
                if(returnedVal->type == DOUBLE_TYPE){
                    sumVal->d += returnedVal->d;
                    }

            }
            else {

                printf("Trying to add non-numbers\n");
                evalError();
            }
            args = cdr(args);
        }
        arithCount--;
        return sumVal;
    }
}


// Handles case where we run into an if
Value *evalIf(Value *conditionals, Frame *frame){

     int counter = 0;
     Value *conditionalCounter = conditionals;
     while(conditionalCounter->type != NULL_TYPE){
         counter++;
         conditionalCounter = cdr(conditionalCounter);
     }

     // Validating number of elements after an if statement
     if(counter != 3){
         printf("Not enough number of parameters\n");
         evalError();
     }


     //check if the element that is after if is a symbol that binds to a bool
     if (car(conditionals)->type == SYMBOL_TYPE || car(conditionals)->type == CONS_TYPE){
         // this variable handles the comment reminder above function
         //printf("FIRST TRIP TO 0\n");
         ifCondShouldPrint = 0;
         shouldPrint = 0;
         Value *symbolVal = eval(car(conditionals),frame);
         //printf("FIRST TRIP TO 1\n");

         ifCondShouldPrint = 1;
         shouldPrint = 1;
         if(symbolVal->type == BOOL_TYPE){
             if (symbolVal->i == 1 ) {
                return eval(car(cdr(conditionals)), frame);
             }
             else if (symbolVal->i == 0 ) {
                 return eval(car(cdr(cdr(conditionals))), frame);
             }
         }
     }

     //if the element that is after if is not a bool, throw evalError
     else if (car(conditionals)->type != BOOL_TYPE) {
         printf("Not eval error\n");
         evalError();
     }
     else {
        if (car(conditionals)->i == 1 ) {
            return eval(car(cdr(conditionals)), frame);
        }
        else if (car(conditionals)->i == 0 ) {
            return eval(car(cdr(cdr(conditionals))), frame);
        }
     }
 }


// Handles case where we run into an if
Value *evalCond(Value *args, Frame *frame){
    // int consCount = 0;
    // Value *argsPtr


    while (args->type != NULL_TYPE) {

        if (car(args)->type != CONS_TYPE) {
            printf("not cons type in cond.\n");
            evalError();
        }

        if (car(car(args))->type == SYMBOL_TYPE) {
            if (!strcmp(car(car(args))->s, "else")) {
                if(cdr(args)->type != NULL_TYPE){
                    printf("else is not the default case.\n");
                    evalError();
                }
                return eval(car(cdr(car(args))),frame);
            }
        }
        Value *condVal = eval(car(car(args)), frame);
        if (condVal->type == BOOL_TYPE) {
            if (condVal->i == 1) {
                return eval(car(cdr(car(args))), frame);
            }
        }


        else {
            printf("cond not a bool.\n");
            evalError();
        }
        args = cdr(args);
    }
    Value *voidBuddy = talloc(sizeof(Value));
    voidBuddy->type = VOID_TYPE;

    return voidBuddy;
}



// Create new frame, with variable bindings and pointer to parent frame
Frame *evalLet(Value *args, Frame *frame){

    Value *firstBinding = car(args);

    //loop to check that the number of elements in a binding are 2, else throw evalError
    while(firstBinding->type != NULL_TYPE){
        Value *bindingElt = car(firstBinding);
        int countElts = 0;
        while(bindingElt->type!=NULL_TYPE){
            countElts++;
            bindingElt = cdr(bindingElt);
        }
        if(countElts != 2){
            printf("Inadequate elements in bindings\n");
            evalError();
        }
        firstBinding = cdr(firstBinding);
    }

    Frame *newFrame;
    newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    Value *tempArgs = car(args);

    //loop to check that bindings are in the correct format, else throw evalError
    while(tempArgs->type != NULL_TYPE){\
        if(tempArgs->type != CONS_TYPE){

            evalError();
        }
        if(car(car(tempArgs))->type != SYMBOL_TYPE){

            evalError();
        }
        if((cdr(car(tempArgs)))->type == NULL_TYPE) {

            evalError();
        }

        tempArgs = cdr(tempArgs);
    }

    newFrame->bindings = car(args);

    return newFrame;
}





// Create new frame, with variable bindings and pointer to parent frame
Frame *evalLetStar(Value *args, Frame *frame){

    Value *firstBinding = car(args);

    //loop to check that the number of elements in a binding are 2, else throw evalError
    while(firstBinding->type != NULL_TYPE){
        Value *bindingElt = car(firstBinding);
        int countElts = 0;
        while(bindingElt->type!=NULL_TYPE){
            countElts++;
            bindingElt = cdr(bindingElt);
        }
        if(countElts != 2){
            printf("Inadequate elements in bindings\n");
            evalError();
        }
        firstBinding = cdr(firstBinding);
    }


    Value *tempArgs = car(args);

    //loop to check that bindings are in the correct format, else throw evalError
    while(tempArgs->type != NULL_TYPE){
        if(tempArgs->type != CONS_TYPE){

            evalError();
        }
        if(car(car(tempArgs))->type != SYMBOL_TYPE){

            evalError();
        }
        if((cdr(car(tempArgs)))->type == NULL_TYPE) {

            evalError();
        }

        Frame *newFrame;
        newFrame = talloc(sizeof(Frame));
        newFrame->parent = frame;
        newFrame->bindings = tempArgs;


        tempArgs = cdr(tempArgs);

        frame = newFrame;
    }




    return frame;
}

void *evalDefine(Value *args){
    Value *var = car(args);

    if(cdr(args)->type == NULL_TYPE){
        //printf("Error");
        evalError();
    }

    Value *expr = car(cdr(args));

    //loop to check that bindings are in the correct format, else throw evalError
    if(var->type != SYMBOL_TYPE){
        evalError();
    }

    else{
        Value *newDefBinding = makeNull();
        newDefBinding = cons(expr, newDefBinding);
        newDefBinding = cons(var, newDefBinding);
        globalFrame->bindings = cons(newDefBinding,globalFrame->bindings);
    }

}

Value *evalLambda(Value *args, Frame *frame){
    Value *params = car(args);
    Value *code = car(cdr(args));
    Value *closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl.paramNames = params;
    closure->cl.functionCode = code;

    closure->cl.frame = frame;

    if(letLet == 1){
        closure->cl.frame = frame->parent;
        letLet = 0;
    }

    return closure;
}

// function is our Lambda closure, args is our linked list of actual parameters
Value *apply(Value *function, Value *args){

    function->cl.paramNames = reverse(function->cl.paramNames);

    if(length(function->cl.paramNames) != length(args)){
        printf("Incorrect number of parameters\n");
        evalError();
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = function->cl.frame;
    Value *bindings = makeNull();

    while(args->type != NULL_TYPE){
        Value *singleBinding = makeNull();
        singleBinding = cons(car(args), singleBinding);
        singleBinding = cons(car(function->cl.paramNames), singleBinding);


        bindings = cons(singleBinding, bindings);

        args = cdr(args);
        function->cl.paramNames = cdr(function->cl.paramNames);

    }

    newFrame->bindings = bindings;

    return eval(function->cl.functionCode, newFrame);

}

Value *evalEach(Value *args, Frame *frame){
    Value *argsEvaled = makeNull();
    shouldPrint = 0;
    while(args->type != NULL_TYPE){
        argsEvaled = cons(eval(car(args), frame), argsEvaled);

        args = cdr(args);
    }
    shouldPrint = 1;


    return argsEvaled;
}

int *evalSet(Value *args, Frame *frame){
  Value* symbol = car(args);
  int count = 0;

  while(frame){

      // go through bindings
      Value *curBinding = frame->bindings;

      while(curBinding->type != NULL_TYPE){
          if(!strcmp((car(car(curBinding)))->s,symbol->s)){
            Value *consCellToChange = cdr(car(curBinding));

            if(car(cdr(args))->type == CONS_TYPE){
              if(car(car(cdr(args)))->type == SYMBOL_TYPE){
                if(!strcmp(car(car(cdr(args)))->s,"lambda")){
                  boundToClosure = 1;
                }
              }
            }
            consCellToChange->c.car= eval(car(cdr(args)),frame);
            return 1;
          }

          curBinding = cdr(curBinding);

      }
      count++;
      frame = frame->parent;

  }

  //if the symbol is not in the binding
  //printf("getValFromBinding error:%d\n", count);
  printf("Binding not found.\n");
  evalError();
  return 0;

}


Value *evalOr(Value *args, Frame *frame){
  orCount++;
  Value *currentArgEvaled;
  Value *currentArg;
    while(args->type != NULL_TYPE){
      currentArg = car(args);

      //check if the element that is after if is a symbol that binds to a bool
      if (car(args)->type == SYMBOL_TYPE || car(args)->type == CONS_TYPE){
          // this variable handles the comment reminder above function
          ifCondShouldPrint = 0;
          shouldPrint = 0;
          currentArgEvaled = eval(car(args),frame);

          ifCondShouldPrint = 1;
          shouldPrint = 1;
      }

      else{

        // if not bool type, break it.
        currentArgEvaled = currentArg;
      }

      if(currentArgEvaled->type == BOOL_TYPE){
        shouldPrint = 1;
        ifCondShouldPrint = 1;
        if(currentArgEvaled->i == 1){
           orCount--;
          return currentArgEvaled;
        }
      }

      else{
        printf("Trying to evalute 'and' with an incorrect type.\n");
        evalError();
      }

      args = cdr(args);
    }
    orCount--;
    return currentArgEvaled;
}


Value *evalAnd(Value *args, Frame *frame){
  andCount++;
  Value *currentArgEvaled;
  Value *currentArg;
    while(args->type != NULL_TYPE){
      currentArg = car(args);

      //check if the element that is after if is a symbol that binds to a bool
      if (car(args)->type == SYMBOL_TYPE || car(args)->type == CONS_TYPE){
        // printf("seeing symbol or cons type\n");
          // this variable handles the comment reminder above function
          ifCondShouldPrint = 0;
          // printf("evalAnd ifCondShouldPrint %i\n", ifCondShouldPrint);
          shouldPrint = 0;
          // printf("evalAnd shouldPrint %i\n", shouldPrint);
          currentArgEvaled = eval(car(args),frame);

          ifCondShouldPrint = 1;
          shouldPrint = 1;
      }

      else{

        // if not bool type, break it.

        currentArgEvaled = currentArg;
        //currentArgEvaled = eval(currentArg, frame);

      }

      if(currentArgEvaled->type == BOOL_TYPE){
        shouldPrint = 1;
        ifCondShouldPrint = 1;
        if(currentArgEvaled->i == 0){
           andCount--;
          return currentArgEvaled;
        }
      }

      else{
        printf("Trying to evalute 'and' with an incorrect type.\n");
        evalError();
      }

      args = cdr(args);
    }
    andCount--;
    return currentArgEvaled;
}


Value *eval(Value *tree, Frame *frame) {
   Value *first, *args, *applyReturn, *letted, *symbolReturn;
   switch (tree->type)  {
     case INT_TYPE:
       if(shouldPrint == 1 && arithCount == 0){
            printf("%i\n", tree->i);
   }
        return tree;
        break;
     case BOOL_TYPE:
       if(shouldPrint == 1 && ifCondShouldPrint == 1 && andCount == 0 && orCount == 0){
            if(tree->i == 0){
                printf("#f\n");
            }
            else{
                printf("#t\n");
            }
        }
        return tree;
        break;
     case STR_TYPE:
       if(shouldPrint == 1){
            printf("%s\n", tree->s);
       }
        return tree;
        break;
     case DOUBLE_TYPE:
       if(shouldPrint == 1 && arithCount == 0){
            printf("%f\n", tree->d);
       }
        return tree;
        break;

     case VOID_TYPE:
        return tree;
        break;

     case SYMBOL_TYPE:
      // printf("in symbol type\n");
     if (letStar == 1 || letLet == 1){
       Value *valFromBinding;
       Frame *frameReturn;
       valFromBinding = getValFromBinding(tree, frame);
       frameReturn = getFrameForLetStar(tree, frame);
       symbolReturn = eval(valFromBinding, frameReturn->parent);

     }
     else {
       symbolReturn = eval(getValFromBinding(tree, frame), frame);
     }


       if (isFunction == 0 && isLet == 0 && funcIsRunning==0 && boundToClosure == 1) {
           printf("#<procedure>\n");
       }
       boundToClosure = 0;

        // this will always be a closure type, despite the differences between (test) and test
        return symbolReturn;
        break;

     case CONS_TYPE:
       hitCons = 1;
        first = car(tree);
        args = cdr(tree);

        // Sanity and error checking on first...

        if (!strcmp(first->s,"if")) {
            return evalIf(args,frame);
        }

        else if(!strcmp(first->s, "let")) {
            curFrame = evalLet(args,frame);

            letLet = 1;
            isLet = 1;
            letted = eval(car(cdr(args)),curFrame);
            letLet = 0;
            isLet = 0;
            return letted;

        }

        else if(!strcmp(first->s, "let*")) {


            letStar = 1;
            curFrame = evalLetStar(args,frame);

            isLet = 1;

            letted = eval(car(cdr(args)),curFrame);

            isLet = 0;
            return letted;

        }

        else if(!strcmp(first->s, "letrec")) {
            curFrame = evalLet(args,frame);
            isLet = 1;
            letted = eval(car(cdr(args)),curFrame);
            isLet = 0;
            return letted;

        }


        else if(!strcmp(first->s, "begin")){
            Value *began = begin(args, frame);
            return eval(began, frame);
        }


       else if(!strcmp(first->s, "null?")) {
//            printf("seeing null\n");
            Value *isNull = nullCheck(args, frame);
            return eval(isNull, frame);
       }

       else if(!strcmp(first->s, "cond")){
        shouldPrint = 0;
            Value *conded = evalCond(args, frame);
            shouldPrint = 1;
            return eval(conded, frame);
        }

        else if (!strcmp(first->s, "modulo")){
         Value *remainder = modulo(args,frame);
         return eval(remainder, frame);
       }

       else if (!strcmp(first->s, "-")){
         Value *difference = minus(args,frame);
         return eval(difference, frame);
       }

       else if(!strcmp(first->s, "*")){
         Value *product = multiply(args, frame);
         return eval(product, frame);
       }

       else if(!strcmp(first->s, "/")){
         Value *quotient = divide(args, frame);
         return eval(quotient, frame);
       }

       else if(!strcmp(first->s, "+")) {
//           printf("seeing +\n");
           Value *sum = plus(args, frame);
           //printf(sum->d);
           return eval(sum, frame);
       }

       else if(!strcmp(first->s, ">")) {

            Value *greater = greaterThan(args, frame);

            return eval(greater, frame);

       }



       else if(!strcmp(first->s, "<")) {

            Value *lesser = lessThan(args, frame);

            return eval(lesser, frame);

       }



       else if(!strcmp(first->s, "=")) {

            Value *equalVal = equal(args, frame);

            return eval(equalVal, frame);

       }

       else if(!strcmp(first->s, "set!")){

         int res = evalSet(args, frame);


         Value *voidVal = talloc(sizeof(Value));
         voidVal->type = VOID_TYPE;
         return voidVal;
       }

       else if (!strcmp(first->s, "and")){

            Value *boolVal = eval(evalAnd(args, frame),frame);
            return boolVal;

       }

       else if (!strcmp(first->s, "or")){

            Value *boolOr = eval(evalOr(args, frame), frame);
            return boolOr;

       }

       else if(!strcmp(first->s, "car")) {
           Value *getCar = carPrimitive(args, frame);
           if(shouldPrint == 1){
               if(getCar->type == CONS_TYPE){
                printf("(");
                printTree(getCar);
                printf(")\n");
               }
               else {
                   Value *tempCar = makeNull();
                   tempCar = cons(getCar, tempCar);
                   printTree(tempCar);
                   printf("\n");

                }
           }

           return getCar;
       }

       else if(!strcmp(first->s, "cdr")) {
           Value *getCdr = cdrPrimitive(args, frame);
           if(shouldPrint == 1){
               if(getCdr->type == CONS_TYPE){
                printf("(");
                printTree(getCdr);
                printf(")\n");
               }
               else {
                   Value *tempCdr = makeNull();
                   tempCdr = cons(getCdr, tempCdr);
                   printTree(tempCdr);
                   printf("\n");

                }
           }

           return getCdr;
       }

       else if(!strcmp(first->s, "cons")) {
            Value *getCons = consPrimitive(args, frame);
           if(shouldPrint == 1){
            printf("(");
            printTree(getCons);
            printf(")\n");
           }
//           printf("CONS TYPE: %i\n", getCons->type);
            return getCons;

       }


       else if(!strcmp(first->s, "quote")) {

           if(shouldPrint == 1){
//               printf("hkjh");
                printTree(args);

                printf("\n");
           }
            return args;
        }

       else if(!strcmp(first->s, "define")) {
            evalDefine(args);
            Value *empty = talloc(sizeof(Value));
            empty->type = VOID_TYPE;
            return empty;
       }

        else if(!strcmp(first->s, "lambda")) {

            Value *closure = evalLambda(args, frame);
            return closure;
       }


       // for user defined functions
        else {


            funcIsRunning = 1;
            isFunction = 1;

            // evaledOperator should be the closure we get back
            Value *evaledOperator = eval(first, frame);

            Value *evaledArgs = evalEach(args, frame);


            if (evaledOperator->type != CLOSURE_TYPE) {
                printf("evaledOperator type is not closuretype\n");
                evalError();
            }
            applyReturn = apply(evaledOperator,evaledArgs);

           // printf("I'm not a function anymore!\n");
            isFunction = 0;
            return applyReturn;
        }
        break;
    }
}
