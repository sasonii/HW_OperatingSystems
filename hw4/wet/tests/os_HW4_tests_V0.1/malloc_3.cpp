//
// Created by student on 7/5/23.
//

#include "unistd.h"
#include <cstring>
#include <list>
#include <sys/mman.h>
#include <random>

#define MAX_SIZE_TO_ALLOCATE 100000000
#define MAX_ORDER 10
#define MAX_SIZE (128 * 1024)
#define START_SIZE (128)



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

std::random_device rd;
std::mt19937 generator(rd());

// Generate a random 32-bit cookie value
uint32_t generateRandomCookieValue() {
    std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max());
    return distribution(generator);
}



struct MallocMetadata {
    uint32_t cookie;
    size_t size;
    bool is_free;
    bool is_mmap;
    int order;
    MallocMetadata* next;
    MallocMetadata* prev;
};

class MemList {
private:
    MallocMetadata* head;
    MallocMetadata* MMAPhead;
    std::list<MallocMetadata*> freed[MAX_ORDER + 1];
    uint32_t EXPECTED_COOKIE_VALUE;

    MallocMetadata* GetFreeBlock(size_t size);
    void SplitIfPossible(MallocMetadata* addr,size_t size);
    void UnionIfPossible(MallocMetadata* addr);
    void AddToFreeOrder(MallocMetadata* addr, int order);

public:
    MemList() : head(NULL), MMAPhead(NULL), freed{}, EXPECTED_COOKIE_VALUE(generateRandomCookieValue()) {}
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

    curr_ptr = MMAPhead;

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


    curr_ptr = MMAPhead;

    while(curr_ptr != NULL){
        num_bytes += curr_ptr->size;
        curr_ptr = curr_ptr->next;
    }

    return num_bytes;
}

int pow2(int i) {
    return 1 << i;
}

MallocMetadata* MemList::GetFreeBlock(size_t size){
    for(int i = 0; i<= 10; i++){
        if(size <= pow2(i) * START_SIZE - sizeof(MallocMetadata) && !freed[i].empty()){
            MallocMetadata* addr = freed[i].front();
            freed[i].pop_front();
            return addr;
        }
    }
    return NULL;
}

void MemList::SplitIfPossible(MallocMetadata* addr ,size_t size){
    if(addr->cookie != EXPECTED_COOKIE_VALUE){
        exit(0xdeadbeef);
    }
    while(addr->order > 0){
        if((addr->size - sizeof(MallocMetadata))/2 >= size){

            MallocMetadata* buddy = (MallocMetadata*)((intptr_t)addr ^ pow2(addr->order - 1) * START_SIZE);
            buddy->cookie = EXPECTED_COOKIE_VALUE;
            buddy->prev = addr;
            buddy->next = addr->next;
            buddy->is_free = true;
            buddy->is_mmap = false;
            buddy->order = addr->order - 1;
            buddy->size = pow2(buddy->order) * START_SIZE - sizeof(MallocMetadata);
            AddToFreeOrder(buddy, buddy->order);

            MallocMetadata* next_to_buddy = addr->next;
            if(next_to_buddy != NULL){
                next_to_buddy->prev = buddy;
            }

            addr->order = addr->order - 1;
            addr->next = buddy;
            addr->size = buddy->size;
        }
        else{
            break;
        }
    }
    addr->is_free = false;
}

void MemList::UnionIfPossible(MallocMetadata* addr){
    if(addr->is_mmap){
        return;
    }
    MallocMetadata* buddy = (MallocMetadata*)((intptr_t)addr ^ pow2(addr->order) * START_SIZE);
    while(buddy->is_free && buddy->order < 10) {
        if((buddy->cookie != EXPECTED_COOKIE_VALUE) || (addr->cookie != EXPECTED_COOKIE_VALUE)){
            exit(0xdeadbeef);
        }
        if (buddy > addr) {
            freed[buddy->order].remove(buddy);
            freed[buddy->order].remove(addr);
            addr->next = buddy->next;
            addr->order++;
            addr->size = pow2(addr->order) * START_SIZE - sizeof(MallocMetadata);
            addr->is_free = true;

            MallocMetadata *next_to_buddy = buddy->next;
            if (next_to_buddy != NULL) {
                next_to_buddy->prev = addr;
            }
            buddy = (MallocMetadata*)((intptr_t)addr ^ pow2(addr->order) * START_SIZE);
        }
        else{
            freed[buddy->order].remove(buddy);
            freed[buddy->order].remove(addr);
            buddy->next = addr->next;
            buddy->order++;
            buddy->size = pow2(buddy->order) * START_SIZE - sizeof(MallocMetadata);
            buddy->is_free = true;

            MallocMetadata *next_to_buddy = addr->next;
            if (next_to_buddy != NULL) {
                next_to_buddy->prev = buddy;
            }
            addr = buddy;
            buddy = (MallocMetadata*)((intptr_t)addr ^ pow2(addr->order) * START_SIZE);
        }
    }
    addr->is_free = true;
    AddToFreeOrder(addr, addr->order);
}

void MemList::AddToFreeOrder(MallocMetadata* addr, int order){
    std::list<MallocMetadata*>::iterator insertPosition = std::lower_bound(freed[order].begin(), freed[order].end(), addr);
    freed[order].insert(insertPosition, addr);
}

void* MemList::Malloc(size_t size) {

    // initializing "buddy memory" allocator
    if(head == NULL){
        void* current = sbrk(0);

        intptr_t alignmentOffset = (intptr_t)current % (32 * 128 * 1024);

        size_t bytesToAlign = (alignmentOffset == 0) ? 0 : (32 * 128 * 1024) - alignmentOffset;

        if (bytesToAlign > 0) {
            sbrk(bytesToAlign + 32 * 128 * 1024);
        }
        head = (MallocMetadata*) ((char*)current+bytesToAlign);
        MallocMetadata* next_addr = (MallocMetadata *)((char*)head + 128 * 1024);
        MallocMetadata new_meta_data;
        new_meta_data.cookie = EXPECTED_COOKIE_VALUE;
        new_meta_data.next  = next_addr;
        new_meta_data.prev = NULL;
        new_meta_data.is_free = true;
        new_meta_data.size = 128*1024 - sizeof(MallocMetadata);
        new_meta_data.is_mmap = false;
        new_meta_data.order = MAX_ORDER;
        *head = new_meta_data;

        freed[MAX_ORDER].push_back(head);
        for(int i=1; i <= 31; i++){
            MallocMetadata* prev_addr = (MallocMetadata *)((char*)head + (i-1) * 128 * 1024);
            MallocMetadata* curr_addr = (MallocMetadata *)((char*)head + i * 128 * 1024);
            freed[MAX_ORDER].push_back(curr_addr);
            next_addr = (MallocMetadata *)((char*)head + (i+1) * 128 * 1024);
            new_meta_data.prev = prev_addr;
            new_meta_data.next = next_addr;
            new_meta_data.order = MAX_ORDER;
            *curr_addr = new_meta_data;
        }
        ((MallocMetadata *)((char*)head + 31 * 128 * 1024))->next = NULL;
    }

    if(size == 0 || size > MAX_SIZE_TO_ALLOCATE){
        return NULL;
    }

    MallocMetadata* free_addr;
    if(size >= MAX_SIZE){
        free_addr = (MallocMetadata*) mmap(NULL, sizeof(MallocMetadata) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (free_addr == MAP_FAILED) {
            return NULL;
        }
        if(MMAPhead == NULL){
            MMAPhead = free_addr;
            free_addr->order = -1;
            free_addr->next = NULL;
            free_addr->size = size;
            free_addr->prev = NULL;
            free_addr->cookie = EXPECTED_COOKIE_VALUE;
            free_addr->is_mmap = true;
        }
        else{
            MallocMetadata* curr_ptr = MMAPhead;
            MallocMetadata* last_ptr = MMAPhead;

            while(curr_ptr != NULL){
                last_ptr = curr_ptr;
                curr_ptr = curr_ptr->next;
            }
            last_ptr->next = free_addr;
            free_addr->cookie = EXPECTED_COOKIE_VALUE;
            free_addr->order = -1;
            free_addr->next = NULL;
            free_addr->size = size;
            free_addr->prev = last_ptr;
            free_addr->is_mmap = true;
        }
    }
    else{
        free_addr = GetFreeBlock(size);
        if(free_addr == NULL){
            return NULL;
        }
        SplitIfPossible(free_addr, size);
    }


    return reinterpret_cast<void*>(free_addr + 1);
}

void MemList::Free(void* p){
    if(p == NULL){
        return;
    }
    MallocMetadata* start_ptr = reinterpret_cast<MallocMetadata*>(p) - 1;
    if(start_ptr->is_mmap){
        if(MMAPhead == start_ptr){
            MMAPhead = MMAPhead->next;
            return;
        }
        MallocMetadata* curr_ptr = MMAPhead;
        MallocMetadata* last_ptr = MMAPhead;

        while(curr_ptr != start_ptr){
            last_ptr = curr_ptr;
            curr_ptr = curr_ptr->next;
        }

        last_ptr->next = curr_ptr->next;
        if(curr_ptr->next != NULL){
            curr_ptr->next->prev = last_ptr;
        }
        if(curr_ptr->cookie != EXPECTED_COOKIE_VALUE){
//            exit(0xdeadbeef);
        }
        munmap(start_ptr, start_ptr->size + sizeof(MallocMetadata));
    }
    else{
        UnionIfPossible(start_ptr);
    }
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