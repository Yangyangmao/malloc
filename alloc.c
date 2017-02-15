/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

//#define ALIGNMENT 8
//#define ALIGN(size) (((size)+(ALIGNMENT-1))&~(ALIGNMENT-1))
/*
typedef struct _listHead {
    void* prev;
    void* next;
} listHead;
#define LIST_SIZE sizeof(listHead)
*/
size_t ALIGN(size_t s) {
    if (s%8==0) return s;
    return s-s%8+8;
}

typedef struct _blockHead {
    size_t size;
    struct _blockHead* prev;
    struct _blockHead* next;
    struct _blockHead* list_next;
    struct _blockHead* list_prev;
    int free;
} blockHead;


blockHead* head = NULL;
blockHead* back = NULL;

#define BLOCK_HEAD_SIZE sizeof(blockHead)

blockHead *freeList = NULL;

void insert_mem (blockHead* meta);
void remove_mem (blockHead* meta);
void merge_block(blockHead*);
void split_block (blockHead* org_block, size_t new_size);


void* byte_move(void* p, int step) {
    return (void*)((char*)p + step);
}

blockHead* enlarge_heap (size_t size) {
    if (size == 0) return NULL;

    size_t s = size;
    size_t as = s + BLOCK_HEAD_SIZE; 
    blockHead* new_head = sbrk(0);
    void* temp = sbrk(as);
    if (temp == (void*)-1)
        return NULL;

    new_head->size = s;
    new_head->next = NULL;
    new_head->free = 0;
    new_head->list_prev = NULL;
    new_head->list_next = NULL;
    if (head == NULL) {
        head = new_head;
        new_head->prev = NULL;
        back = head;
    } else {
        back->next = new_head;
        new_head->prev = back;
        back = new_head;
    }

    return new_head;
}

void insert_mem(blockHead* meta) {
    if (!meta) {write(1,"A\n", 3); return;}
    //if (!meta->free) return;
    meta->free = 1;
    meta->list_prev = NULL;
    meta->list_next = NULL;
    if (meta == freeList) write(1, "AAA\n", 5);
    if (!freeList) {
        freeList = meta;
        meta->list_prev = NULL;
        meta->list_next = NULL;
    } else {
        meta->list_next = freeList;
        meta->list_prev = NULL;
        freeList->list_prev = meta;
        freeList = meta;
    }
    merge_block(meta);
}

void remove_mem (blockHead* temp) {
///*
    if (!temp) {write(1,"B\n", 3); return;}
    if (temp == freeList) {
        freeList = freeList->list_next;
        if (freeList)
            freeList->list_prev = NULL;
    } else {
        blockHead* tprev = temp->list_prev;    
        blockHead* tnext = temp->list_next;
        if (tprev) temp->list_prev->list_next = tnext;
        if (tnext) temp->list_next->list_prev = tprev;
    }
    temp->free = 0;
//*/
/*
    if (!temp) return;
    assert(temp->free);

    if (freeList == temp) freeList = freeList->list_next;
    if (temp->list_prev) temp->list_prev->list_next = temp->list_next;
    if (temp->list_next) temp->list_next->list_prev = temp->list_prev;

    temp->free = 0;
*/
}

blockHead* find_free (size_t size) {
    blockHead* temp = freeList;
    if (freeList == NULL) return NULL;

    while (temp) {
        //write(1, "AAA ", 5);
        if (temp->size >= size && temp->free) {
            remove_mem(temp);
            temp->free = 0;
            break;
        }
        temp = temp->list_next;
    }
//write(1, "AAA ", 5);

    split_block(temp, size);
    if (temp) temp->free = 0;
    return temp;
}

void split_block (blockHead* org_block, size_t new_size) {
    if (org_block == NULL) return;
    
    size_t org_size = org_block->size;
    if (new_size + BLOCK_HEAD_SIZE + 16 >= org_size) return;

    blockHead* new_block = (blockHead*)((char*)org_block+ BLOCK_HEAD_SIZE + new_size);
    size_t gap = org_size - new_size - BLOCK_HEAD_SIZE;
    new_block->size = gap;
    new_block->free = 1;
    new_block->prev = org_block;
    new_block->next = org_block->next;
    if (new_block->next) new_block->next->prev = new_block;
    org_block->next = new_block;
    org_block->size = new_size;

    insert_mem(new_block);

    
}

void merge_block(blockHead* org_block) {
    if (!org_block) return;
    
    blockHead* cur_block = org_block;
    blockHead* pre_block = org_block->prev;
    blockHead* tsu_block = org_block->next;
    
    
    if (pre_block != NULL && pre_block->free) { 
//write(1, "AAA ", 5);

        pre_block->size += cur_block->size + BLOCK_HEAD_SIZE;
        pre_block->next = cur_block->next;
        if (tsu_block)
            tsu_block->prev = pre_block;
        
        remove_mem(cur_block);
        cur_block = pre_block;
    }

    if (tsu_block != NULL  && tsu_block->free) {
//write(1, "AAA ", 5);

        cur_block->size += tsu_block->size + BLOCK_HEAD_SIZE;
        cur_block->next = tsu_block->next;
        if (tsu_block->next)
            tsu_block->next->prev = cur_block;
        remove_mem(tsu_block);

    }
}
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
  // implement calloc!
    size_t s = ALIGN(num*size);
    void* ptr = malloc(s);
    if (ptr == NULL) return NULL;
    
    memset(ptr, 0, s);
  return ptr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
  // implement malloc!
    if (size == 0) return NULL;
    size = ALIGN(size);

    blockHead* block = NULL;
    void* ptr = NULL;
    if (head == NULL) {
        block = enlarge_heap(size);
        if (!block) return NULL;
        ptr = (void*) (block+1);
        return ptr;
    }

    block = find_free(size);
    if (!block) {
        block = enlarge_heap(size);
        if (!block) return NULL;
        ptr = (void*) (block+1);
        return ptr;
    } else {
        block->free = 0;
        ptr = (void*) (block+1);
        return ptr;
    }

  return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
  // implement free!

    if (!ptr) return;
    blockHead* block = (blockHead*)((blockHead*)ptr-1);
    assert(block->free == 0);

    block->free = 1;
    insert_mem(block);
    //merge_block(block);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
  // implement realloc!
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return ptr;
    }
    
    blockHead* meta = (blockHead*)((blockHead*)ptr - 1);
    size = ALIGN(size);
    if (size <= meta->size) {
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (new_ptr == NULL) return NULL;
    memcpy(new_ptr, ptr, meta->size);
    free(ptr);
    return new_ptr;
}
