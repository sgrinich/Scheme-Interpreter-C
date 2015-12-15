#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    Value *list = tokenize(stdin);
    //printf("Tokenized\n");
    Value *tree = parse(list);
    //printf("Parsed\n");
    interpret(tree);
    //printf("Interpreted\n");
    
    tfree();
    return 0;
}
