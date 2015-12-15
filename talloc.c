// Andrew Yang, Anmol Raina, Stephen Grinich

#import "talloc.h"
#import "value.h"
#include <stdlib.h>
#import "linkedlist.h"


// global variable for global head
Value *globalHead;

// 0 means not malloced, 1 means malloced
int isMallocd = 0;
// 0 means not free, 1 means free
int isFreed = 0;


// make null value
Value *makeNullM(){
  Value *nulVal = malloc(sizeof(Value));
  nulVal->type = NULL_TYPE;

  return nulVal;
}

// Create a new CONS_TYPE value node.
Value *consM(Value *car, Value *cdr){
  Value *value = malloc(sizeof(Value));
  value->type = CONS_TYPE;
  value->c.cdr = cdr;
  value->c.car = car;

  return value;
}


// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    // modify global variable to indicate no longer freed
    isFreed = 0;
    
    // malloc for return
    Value *malVal = malloc(size);

    // if we have not malloced yet
    if(isMallocd == 0){
        globalHead = makeNullM();
        // modify global variable to indicate malloc'd
        isMallocd = 1;
    }
  
    //Pointer to an address we're storing in our list to free later. 
    Value *ptr = malloc(sizeof(Value));
    ptr->type = PTR_TYPE; 
    ptr->p = malVal;
    
    // add newly malloc'd ptr to globalHead
    globalHead = consM(ptr,globalHead);

  return malVal;


}

// Helper function for tfree()
void tfreeHelp(Value *head){
    
    // if it's the null value at end of list, free it
    if(head->type == NULL_TYPE){
        free(head);
    }
    
    // if cons cell
    else if (head->type == CONS_TYPE){
        // free value at car
        free(car(head)->p);
        free(car(head));
        // recurse on cdr of list
        tfreeHelp(cdr(head));
        // free current value
        free(head);
    }
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    // if not freed, call tfree helper function and set freed to true
    if (isFreed == 0) {
        isFreed = 1;
        tfreeHelp(globalHead);
    }
    // malloc'd now false
    isMallocd = 0;
}

// Replacement for the C function "exit", that consists of two lines: it calls tfree before calling exit.
void texit(int status){
    // check if already free to avoid freeing excessively
    if (isFreed == 0){
        tfree();
    }
    exit(status);
}



