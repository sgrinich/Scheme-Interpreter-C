// Andrew Yang, Stephen Grinich, Anmol Raina

#include "linkedlist.h"
#include "talloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Create a new NULL_TYPE value node.
Value *makeNull(){
  Value *nulVal = talloc(sizeof(Value));
  nulVal->type = NULL_TYPE;

  return nulVal;
}



// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr){
  Value *value = talloc(sizeof(Value));
  value->type = CONS_TYPE;
  value->c.cdr = cdr;
  value->c.car = car;

  return value;
}


// Display the contents of the linked list to the screen in some kind of readable format
void display(Value *list){
  assert(list->type != '\0');
  while(list->type != NULL_TYPE){
    // find out what type car(list) is for display
    switch(car(list)->type){
      case INT_TYPE: printf("%d ", list->c.car->i);
                     break;
      case DOUBLE_TYPE: printf("%f ", list->c.car->d);
                        break;
      case STR_TYPE: printf("%s ", list->c.car->s);
                    break;
      case BOOL_TYPE: printf("%s ", list->c.car->s);
                    break;
      case OPEN_TYPE: printf("%s ", list->c.car->s);
                    break;
      case CLOSE_TYPE: printf("%s ", list->c.car->s);
                    break;
    case SYMBOL_TYPE: printf("%s ", list->c.car->s);
                    break;
     case CONS_TYPE: break;
     case NULL_TYPE: break;
     case PTR_TYPE: break;
    }
      // iterate
    list = cdr(list);
  }

  printf(" \n");
}


// Utility to make it less typing to get car value.
Value *car(Value *list){
    assert(list->type != '\0');
    // assume not null b/c should be cons cell
    assert(list->type != NULL_TYPE);
    // assume no nulls in car(list)
    assert(list->c.car->type != NULL_TYPE);

    return list->c.car;
}

// Utility to make it less typing to get cdr value.
Value *cdr(Value *list){
    assert(list->type != '\0');
    // assume not null b/c should be cons cell
    assert(list->type != NULL_TYPE);

    return list->c.cdr;
}


// Return a new list that is the reverse of the one that is passed in.
Value *reverse(Value *list){
    Value *head = makeNull();
    while(list->type != NULL_TYPE){
        Value *newVal = car(list);
        // cons for reversed list
        head = cons(newVal, head);
        // iterate
        list = cdr(list);
    }
    return head;
}

// Utility to check if pointing to a NULL_TYPE value.
bool isNull(Value *value){
  assert(value->type != '\0');
  if(value->type == NULL_TYPE){
    return true;
  }
  else{
    return false;
  }
}

// Measure length of list.
int length(Value *list){
  assert(list->type != '\0');
  int counter = 0;
  while(list->type != NULL_TYPE){
    counter++;
    list = cdr(list);
  }
  return counter;
}

