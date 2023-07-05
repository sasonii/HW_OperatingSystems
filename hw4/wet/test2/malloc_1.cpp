//
// Created by student on 7/5/23.
//

#include "unistd.h"

void* smalloc(size_t size);

#define MAX_SIZE_TO_ALLOCATE 100000000
void* smalloc(size_t size){
    if(size == 0 || size > MAX_SIZE_TO_ALLOCATE){
        return NULL;
    }

    intptr_t s = static_cast<intptr_t>(size);
    void* previous_program_break = sbrk(s);

    if(previous_program_break == (void*)(-1)){
        return NULL;
    }

    return previous_program_break;
}