//
// Created by student on 7/5/23.
//

#include "unistd.h"
#include <cstring>

#define MAX_SIZE_TO_ALLOCATE 100000000
#define MAX_ORDER 10



void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _num_meta_data_bytes();
size_t _size_meta_data();

struct MallocMetadata {
    size_t size;
    bool is_free;
    bool is_mmap;
    MallocMetadata* next;
    MallocMetadata* prev;
};

class MemList {
private:
    MallocMetadata* head;
    MallocMetadata* freed[MAX_ORDER + 1];
    MallocMetadata* allocated[MAX_ORDER + 1];
public:
    MemList() : head(NULL) {
        for(int i = 0; i <= MAX_ORDER; i++){
            freed[i] = NULL;
        }
    }
    size_t GetNumberOfFreeBlocks();
    size_t GetNumberOfFreeBytes();
    size_t GetNumberOfAllocatedBlocks();
    size_t GetNumberOfAllocatedBytes();
    void* Malloc(size_t size);
    void Free(void* p);
    void* Srealloc(void* oldp, size_t size);
};

size_t MemList::GetNumberOfFreeBlocks() {
    MallocMetadata* curr_ptr = head;
    size_t counter = 0;

    while(curr_ptr != NULL){
        if(curr_ptr->is_free){
            counter++;
        }
        curr_ptr = curr_ptr->next;
    }

    return counter;
}

size_t MemList::GetNumberOfFreeBytes() {
    MallocMetadata* curr_ptr = head;
    size_t bytes_count = 0;

    while(curr_ptr != NULL){
        if(curr_ptr->is_free){
            bytes_count += curr_ptr->size;
        }
        curr_ptr = curr_ptr->next;
    }

    return bytes_count;
}

size_t MemList::GetNumberOfAllocatedBlocks() {
    MallocMetadata* curr_ptr = head;
    size_t counter = 0;

    while(curr_ptr != NULL){
        counter++;
        curr_ptr = curr_ptr->next;
    }

    return counter;
}

size_t MemList::GetNumberOfAllocatedBytes() {
    if(head == NULL){
        return 0;
    }
    MallocMetadata* curr_ptr = head;
    MallocMetadata* first_ptr = head;
    MallocMetadata* last_ptr = head;

    int counter = 0;
    while(curr_ptr != NULL){
        counter++;
        last_ptr = curr_ptr;
        curr_ptr = curr_ptr->next;
    }
    size_t num_bytes = static_cast<size_t>(reinterpret_cast<char*>(last_ptr) - reinterpret_cast<char*>(first_ptr)) + \
                    + last_ptr->size - sizeof(MallocMetadata) * (counter-1);

    return num_bytes;
}

void* MemList::Malloc(size_t size) {
    if(head == NULL){
        void* current = sbrk(0);

        intptr_t alignmentOffset = (intptr_t)current % (32 * 128 * 1024);

        size_t bytesToAlign = (alignmentOffset == 0) ? 0 : (32 * 128 * 1024) - alignmentOffset;

        if (bytesToAlign > 0) {
            void* alignedAddress = sbrk(bytesToAlign + 32 * 128 * 1024);
        }
        head = (MallocMetadata*) ((char*)current+bytesToAlign);

    }

    MallocMetadata* curr_ptr = head;
    MallocMetadata* last_ptr = head;

    while(curr_ptr != NULL){
        if(curr_ptr->is_free && size <= curr_ptr->size){
            curr_ptr->is_free = false;
            return reinterpret_cast<void*>(curr_ptr + 1);
        }
        last_ptr = curr_ptr;
        curr_ptr = curr_ptr->next;
    }
    intptr_t s = static_cast<intptr_t>(size + sizeof(MallocMetadata));
    void* previous_program_break = sbrk(s);

    if(previous_program_break == (void*)(-1)){
        return NULL;
    }



    curr_ptr = reinterpret_cast<MallocMetadata*> (previous_program_break);
    curr_ptr->is_free = false;
    curr_ptr->size = size;
    curr_ptr->next = NULL;
    curr_ptr->prev = last_ptr;



    if(head == NULL){
        head = curr_ptr;
    }
    else {
        last_ptr->next = curr_ptr;
    }

    return reinterpret_cast<void*>(curr_ptr + 1);
}

void MemList::Free(void* p){
    MallocMetadata* start_ptr = reinterpret_cast<MallocMetadata*>(p) - 1;
    start_ptr->is_free = true;
}

void* MemList::Srealloc(void* oldp, size_t size){
    MallocMetadata* start_ptr = reinterpret_cast<MallocMetadata*>(oldp) - 1;
    if(size <= start_ptr->size){
        return oldp;
    }
    void* addr = Malloc(size);

    if(addr == NULL){
        return NULL;
    }
    std::memmove(addr, oldp, size);
    Free(oldp);
    return addr;
}

MemList GlobalList = MemList();

size_t _num_free_blocks(){
    return GlobalList.GetNumberOfFreeBlocks();
}

size_t _num_free_bytes()
{
    return GlobalList.GetNumberOfFreeBytes();
}

size_t _num_allocated_blocks()
{
    return GlobalList.GetNumberOfAllocatedBlocks();
}

size_t _num_allocated_bytes()
{
    return GlobalList.GetNumberOfAllocatedBytes();
}

size_t _num_meta_data_bytes()
{
    return GlobalList.GetNumberOfAllocatedBlocks() * sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof(MallocMetadata);
}

void* smalloc(size_t size){
    if(size == 0 || size > MAX_SIZE_TO_ALLOCATE){
        return NULL;
    }

    return GlobalList.Malloc(size);
}

void* scalloc(size_t num, size_t size){
    size_t total_size = num * size;
    if(size == 0 || num == 0 || total_size > MAX_SIZE_TO_ALLOCATE){
        return NULL;
    }

    void* addr = GlobalList.Malloc(total_size);

    if(addr == NULL){
        return NULL;
    }
    std::memset(addr, '\0', total_size);
    return addr;
}

void sfree(void* p){
    GlobalList.Free(p);
}

void* srealloc(void* oldp, size_t size){
    if(size == 0 || size > MAX_SIZE_TO_ALLOCATE){
        return NULL;
    }
    if(oldp == NULL){
        return smalloc(size);
    }
    return GlobalList.Srealloc(oldp, size);
}